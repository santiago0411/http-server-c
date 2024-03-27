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

#define BUF_SIZE 128
static char IN_BUF[BUF_SIZE];

#define MAX_CLIENTS 5
typedef struct
{
	int Id;
	ClientInfo* ClientInfo;
	Buffer In;
	Buffer Out;
} Client;

static Client Clients[MAX_CLIENTS];
static pthread_t network_thread;
static volatile bool network_running = false;

void init_new_client(ClientInfo* info)
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
		printf("Cannot init new client, server full!\n");
		return;
	}

	c->ClientInfo = info;
	ARRAY_INIT(&c->In, BUF_SIZE);
	ARRAY_INIT(&c->Out, BUF_SIZE);
}

void free_client(Client* c)
{
	ARRAY_FREE(&c->In);
	ARRAY_FREE(&c->Out);
	disconnect_client(c->ClientInfo);
}

HttpResponse handle_request(const Buffer* in)
{
	HttpRequest req = {0};
	HttpResponse res = {0};
	if (!request_parse(in, &req)) {
		printf("Failed to parse request!\n");
		response_set_status(&res, 400);
		goto free_and_return;
	}

	const size_t path_size = strlen(req.Path);

	if (strncmp(req.Path, "/user-agent", path_size) == 0) {
		printf("Matched Test 5\n");
		// Test 5: Parse headers
		const char* value = get_header_value(&req.Headers, "User-Agent");
		if (!value) {
			response_set_status(&res, 400);
			goto free_and_return;
		}
		const size_t value_len = strlen(value);
		// This str is freed with the response
		response_set_status(&res, 200);
		set_header_str(&res.Headers, "Content-Type", "text/plain");
		set_header_i32(&res.Headers, "Content-Length", value_len);

		res.Content = malloc(value_len + 1);
		strcpy((char*)res.Content, value);
		goto free_and_return;
	}

	if (path_size >= 5 && strncmp(req.Path, "/echo", 5) != 0) {
		// Path didn't start with "/echo" - Test 3: Respond with 404
		printf("Matched Test 3\n");
		response_set_status(&res, 404);
		goto free_and_return;
	}

	// Path always starts with '/' and is null terminated so we can always + 1 safely
	const int word_start = first_index_of(req.Path + 1, path_size - 1, '/');
	if (word_start == 0) {
		// No second '/' was found
		if (path_size == 1) {
			// Path was just "/" - Test 2: Respond with 200
			printf("Matched Test 2\n");
			response_set_status(&res, 200);
		} else {
			// Path was some other route
			printf("Path was some other route\n");
			response_set_status(&res, 404);
		}
		goto free_and_return;
	}

	// -2 for each '/'
	// Example strlen(/echo/abc) = 9, word_start = 4
	// 9 - 4 - 2 = 3 which is the length of 'abc'
	const int reply_str_size = path_size - word_start - 2;
	if (reply_str_size <= 0) {
		// Path only had one '/' - Idk what we do here honestly
		response_set_status(&res, 400);
		goto free_and_return;
	}

	// Test 4: Respond with content
	char* reply_str = malloc(reply_str_size + 1);
	// +2 same logic as above
	memcpy(reply_str, req.Path + word_start + 2, reply_str_size);
	reply_str[reply_str_size] = '\0';

	response_set_status(&res, 200);
	set_header_str(&res.Headers, "Content-Type", "text/plain");
	set_header_i32(&res.Headers, "Content-Length", reply_str_size);
	// This str is freed with the response
	res.Content = reply_str;

free_and_return:
	request_destroy(&req);
	return res;
}

void* network_function(void* arg)
{
	network_running = true;
	while (network_running) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			Client* c = &Clients[i];
			if (c->ClientInfo) {

			}
		}
	}

	return NULL;
}

int main() {
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

#ifdef MT
	printf("Waiting for a client to connect...\n");

	for (;;) {
		const ClientInfo* client = accept_client(socket_fd);
		if (!client) {
			continue;
		}

		int nbytes = recv(client->Socket, IN_BUF, BUF_SIZE, 0);

		if (nbytes < 0) {
			fprintf(stderr, "Failed to read bytes: %s", strerror(errno));
			close(client->Socket);
			break;
		}

		printf("Received data:\n%.*s", nbytes, IN_BUF);

		HttpResponse res = handle_request(IN_BUF, nbytes);
		size_t res_size;
		const char* res_str = response_to_str(&res, &res_size);
		printf("Sending response:\n%.*s", res_size, res_str);

		if (send(client->Socket, res_str, res_size, 0) < 0) {
			fprintf(stderr, "Failed to send response: %s\n", strerror(errno));
		}

		response_destroy(&res);
		printf("Response sent successfully\n");
		// For the time being we only handle one connection
		close(client->Socket);
		break;
	}
	close(socket_fd);
#elseif 0
	if (pthread_create(&network_thread, NULL, network_function, NULL) != 0) {
		fprintf(stderr, "Failed to start network thread\n");
		close(socket_fd);
		return 1;
	}

	for (;;) {
		ClientInfo* info = accept_client(socket_fd);
		if (info) {
			init_new_client(info);
		}
		// TODO thread sleep
	}
#else
	printf("Waiting for a client to connect...\n");

	ClientInfo* client_info = accept_client(socket_fd);
	if (!client_info) {
		close(socket_fd);
		return 0;
	}

	init_new_client(client_info);
	// Only handling one connection so it's always gonna be 0
	Client* client = &Clients[0];

	fd_set read_fd_set;
	FD_ZERO(&read_fd_set);
	FD_SET(client_info->Socket, &read_fd_set);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;

	do
	{
		select(client_info->Socket + 1, &read_fd_set, NULL, NULL, &timeout);

		if (!(FD_ISSET(client_info->Socket, &read_fd_set))) {
			break;
		}

		int nbytes = recv(client_info->Socket, IN_BUF, BUF_SIZE, 0);

		if (nbytes < 0) {
			fprintf(stderr, "Failed to read bytes: %s", strerror(errno));
			goto exit;
		}

		if (nbytes == 0) {
			break;
		}

		ARRAY_APPEND_MANY(&client->In, IN_BUF, nbytes);
	} while (true);

	if (client->In.Count > 0) {
		printf("Received data:\n%.*s", (int)client->In.Count, client->In.Data);

		HttpResponse res = handle_request(&client->In);
		size_t res_size;
		const char* res_str = response_to_str(&res, &res_size);
		printf("Sending response:\n%.*s", (int)res_size, res_str);

		if (send(client_info->Socket, res_str, res_size, 0) < 0) {
			fprintf(stderr, "Failed to send response: %s\n", strerror(errno));
		}

		response_destroy(&res);
		printf("Response sent successfully\n");
	}

exit:
	// For the time being we only handle one connection
	free_client(client);
	close(socket_fd);
#endif

	return 0;
}
