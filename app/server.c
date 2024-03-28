#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "net.h"
#include "request.h"
#include "response.h"

#ifdef LOCAL_TEST
	#define PORT 30505
#else
	#define PORT 4221
#endif

#define BUF_SIZE 1024
static char IN_BUF[BUF_SIZE];
pthread_mutex_t IN_BUF_MUTEX = PTHREAD_MUTEX_INITIALIZER;

#define MAX_CLIENTS 5
typedef struct
{
	int Id;
	ClientInfo* ClientInfo;
	pthread_t Thread;
	Buffer In;
	Buffer Out;
} Client;

static Client Clients[MAX_CLIENTS];

static const char* files_directory = NULL;
static size_t files_directory_len = 0;

Client* init_new_client(ClientInfo* info)
{
	Client* c = NULL;
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (!Clients[i].ClientInfo) {
			c = &Clients[i];
			c->Id = i;
			break;
		}
	}

	if (!c) {
		printf("Cannot init new client %s:%u, server full!\n", info->RemoteAddress, info->RemotePort);
		disconnect_client(info);
		return NULL;
	}

	c->ClientInfo = info;
	ARRAY_INIT(&c->In, BUF_SIZE);
	// ARRAY_INIT(&c->Out, BUF_SIZE); // Unused for now
	printf("Client %s:%u (%d) connected successfully!\n", info->RemoteAddress, info->RemotePort, c->Id);
	return c;
}

void free_client(Client* c)
{
	ARRAY_FREE(&c->In);
	ARRAY_FREE(&c->Out);
	disconnect_client(c->ClientInfo);
	c->ClientInfo = NULL;
}

void serve_file(const HttpRequest* req, HttpResponse res)
{
	const size_t path_size = strlen(req->Path);
	const int word_start = first_index_of(req->Path + 1, path_size - 1, '/');
	if (word_start == 0) {
		response_set_status(res, 404);
		return;
	}

	const char* file_name = req->Path + word_start + 1;
	const int file_name_size = path_size - word_start - 1;
	if (file_name_size > 64) {
		fprintf(stderr, "File name was bigger than 64 bytes!!\n");
		response_set_status(res, 400);
		return;
	}

	char* full_file_path = malloc(files_directory_len - 1 + file_name_size);
	memcpy(full_file_path, files_directory, files_directory_len - 1);
	memcpy(full_file_path + files_directory_len - 1, file_name, file_name_size);

	Buffer file = read_file_to_end(full_file_path);
	free(full_file_path);

	if (file.Count <= 0) {
		response_set_status(res, 404);
		ARRAY_FREE(&file);
		return;
	}

	response_set_status(res, 200);
	response_set_header(res, create_header_str("Content-Type", "application/octet-stream"));
	response_set_header(res, create_header_i32("Content-Length", file.Count));
	// Buffer will be freed when the response is destroyed
	response_set_content(res, file.Data, file.Count);
}

HttpResponse handle_request(const HttpRequest* req)
{
	HttpResponse res;
	response_create(&res);
	
	if (req->Method == METHOD_HEAD) {
		// We don't really handle files for now, so no headers in response
		response_set_status(res, 200);
		return res;
	}

	const size_t path_size = strlen(req->Path);

	if (path_size > 1 && strncmp(req->Path, "/user-agent", path_size) == 0) {
		printf("Matched Test 5\n");
		// Test 5: Parse headers
		const char* value = get_header_value(&req->Headers, "User-Agent");
		if (!value) {
			response_set_status(res, 400);
			return res;
		}
		const size_t value_len = strlen(value);
		// This str is freed with the response
		response_set_status(res, 200);
		response_set_header(res, create_header_str("Content-Type", "text/plain"));
		response_set_header(res, create_header_i32("Content-Length", value_len));

		char* tmp = malloc(value_len);
		strcpy(tmp, value);
		response_set_content(res, tmp, value_len);
		return res;
	}

	if (path_size >= 5 && strncmp(req->Path, "/files", 5) == 0) {
		// Test 7: Get a file
		serve_file(req, res);
		return res;
	}

	if (path_size >= 5 && strncmp(req->Path, "/echo", 5) != 0) {
		// Path didn't start with "/echo" - Test 3: Respond with 404
		printf("Matched Test 3\n");
		response_set_status(res, 404);
		return res;
	}

	// Path always starts with '/' and is null terminated so we can always + 1 safely
	const int word_start = first_index_of(req->Path + 1, path_size - 1, '/');
	if (word_start == 0) {
		// No second '/' was found
		if (path_size == 1) {
			// Path was just "/" - Test 2: Respond with 200
			printf("Matched Test 2\n");
			response_set_status(res, 200);
		} else {
			// Path was some other route
			printf("Path was some other route\n");
			response_set_status(res, 404);
		}
		return res;
	}

	// -2 for each '/'
	// Example strlen(/echo/abc) = 9, word_start = 4
	// 9 - 4 - 2 = 3 which is the length of 'abc'
	const int reply_str_size = path_size - word_start - 2;
	if (reply_str_size <= 0) {
		// Path only had one '/' - Idk what we do here honestly
		response_set_status(res, 400);
		return res;
	}

	// Test 4: Respond with content
	char* reply_str = malloc(reply_str_size);
	// +2 same logic as above
	memcpy(reply_str, req->Path + word_start + 2, reply_str_size);

	response_set_status(res, 200);
	response_set_header(res, create_header_str("Content-Type", "text/plain"));
	response_set_header(res, create_header_i32("Content-Length", reply_str_size));
	// This str is freed with the response
	response_set_content(res, reply_str, reply_str_size);
	return res;
}

bool is_valid_http_request(const Buffer* in)
{
	return in->Count > 4 && strncmp(in->Data + (in->Count - 4), "\r\n\r\n", 4) == 0;
}

void try_read_data(Client* c)
{
	fd_set read_fd_set;
	FD_ZERO(&read_fd_set);
	FD_SET(c->ClientInfo->Socket, &read_fd_set);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;

	do {
		select(c->ClientInfo->Socket + 1, &read_fd_set, NULL, NULL, &timeout);

		if (!(FD_ISSET(c->ClientInfo->Socket, &read_fd_set))) {
			break;
		}

		pthread_mutex_lock(&IN_BUF_MUTEX);
		const int nbytes = recv(c->ClientInfo->Socket, IN_BUF, BUF_SIZE, 0);
		if (nbytes > 0) {
			ARRAY_APPEND_MANY(&c->In, IN_BUF, nbytes);
		}
		pthread_mutex_unlock(&IN_BUF_MUTEX);

		if (nbytes < 0) {
			fprintf(stderr, "Failed to read bytes: %s", strerror(errno));
			return;
		}

		if (nbytes == 0) {
			break;
		}
	} while (true);

	if (is_valid_http_request(&c->In)) {
		printf("Received data from client %s:%u (%d):\n\n%.*s",
			c->ClientInfo->RemoteAddress, c->ClientInfo->RemotePort, c->Id,
			(int)c->In.Count, c->In.Data);

		HttpRequest req = {0};
		if (!request_parse(&c->In, &req)) {
			printf("Failed to parse request!\n");
			free_client(c);
			return;
		}

		HttpResponse res = handle_request(&req);
		c->In.Count = 0;
		size_t res_size;
		const char* res_str = response_to_str(res, &res_size);

		printf("Sending response to client %s:%u (%d):\n\n%.*s",
			c->ClientInfo->RemoteAddress, c->ClientInfo->RemotePort, c->Id,
			(int)res_size, res_str);

		if (send(c->ClientInfo->Socket, res_str, res_size, 0) < 0) {
			fprintf(stderr, "Failed to send response to client %s:%u (%d): %s\n\n",
				c->ClientInfo->RemoteAddress, c->ClientInfo->RemotePort, c->Id, strerror(errno));
		}

		printf("Response sent successfully.\n");

		const char* connection = get_header_value(&req.Headers, "Connection");
		if (!connection || strcmp(connection, "keep-alive") != 0) {
			free_client(c);
		}
		response_destroy(res);
		request_destroy(&req);
		free((void*)res_str);
	}
}

void* network_function(void* arg)
{
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 10 * 1000000;
	Client* c = arg;

	while (c->ClientInfo) {
		try_read_data(c);
		nanosleep(&ts, NULL);
	}
	return NULL;
}

int main(const int argc, const char** argv) {
	if (argc > 2) {
		if (strcmp(argv[1], "--directory") == 0) {
			files_directory = argv[2];
			files_directory_len = strlen(files_directory);
		}
	}

	// Disable output buffering
	setbuf(stdout, NULL);
	printf("Starting socket...\n");

	const int socket_fd = create_socket(PORT);
	if (socket_fd < 0) {
		return 1;
	}

	if (listen(socket_fd, MAX_CLIENTS) != 0) {
		fprintf(stderr, "Listen failed: %s \n", strerror(errno));
		close(socket_fd);
		return 1;
	}

	for (;;) {
		ClientInfo* info = accept_client(socket_fd);
		if (!info) {
			break; // Server is full
		}
		Client* c = init_new_client(info);
		if (c) {
			pthread_attr_t attr;
			// Detached means we don't have to join it when it's done
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			if (pthread_create(&c->Thread, &attr, network_function, c) != 0) {
				fprintf(stderr, "Failed to start network thread\n");
				close(socket_fd);
				return 1;
			}
		}
	}

	for (int i = 0 ; i < MAX_CLIENTS; i++) {
		free_client(&Clients[i]);
	}

	close(socket_fd);
	return 0;
}
