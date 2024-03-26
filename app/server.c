#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "net.h"
#include "request.h"
#include "response.h"

#ifdef LOCAL_TEST
	#define PORT 30505
#else
	#define PORT 4221
#endif

#define IN_BUF_SIZE 1024
static char IN_BUF[IN_BUF_SIZE];


HttpResponse handle_request(const char* buf, const int size)
{
	HttpRequest req;
	HttpResponse res;
	if (!request_parse(buf, size, &req)) {
		response_set_status(&res, 400);
		goto free_and_return;
	}

	response_set_status(&res, 200);
	set_header_str(&res.Headers, "Content-Type", "text/plain");

	const size_t path_size = strlen(req.Path);
	// Path always starts with '/' and is null terminated so we can always + 1 safely
	const int word_start = first_index_of(req.Path + 1, path_size - 1, '/');
	if (word_start == 0) {
		// Path was just "/"
		set_header_str(&res.Headers, "Content-Length", "1");
		// Dark magic to deal with this dumb test case
		res.Content = calloc(2, 1);
		goto free_and_return;
	}

	// -2 for each '/'
	// Example strlen(/echo/abc) = 9, word_start = 4
	// 9 - 4 - 2 = 3 which is the length of 'abc'
	const int reply_str_size = path_size - word_start - 2;
	if (reply_str_size <= 0) {
		// Path only had one '/' so I guess we also send empty
		set_header_str(&res.Headers, "Content-Length", "1");
		// Dark magic to deal with this dumb test case
		res.Content = calloc(2, 1);
		goto free_and_return;
	}

	char* reply_str = malloc(reply_str_size + 1);
	// +2 same logic as above
	memcpy(reply_str, req.Path + word_start + 2, reply_str_size);
	reply_str[reply_str_size] = '\0';


	set_header_i32(&res.Headers, "Content-Length", reply_str_size);
	// This str is freed with the response
	res.Content = reply_str;

free_and_return:
	request_destroy(&req);
	return res;
}

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
	printf("Starting socket...\n");

	const int socket_fd = create_socket(PORT);
	if (socket_fd < 0) {
		return 1;
	}

	const int connection_backlog = 5;
	if (listen(socket_fd, connection_backlog) != 0) {
		fprintf(stderr, "Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");

	for (;;) {
		const int client_fd = accept_client(socket_fd);
		if (client_fd < 0) {
			continue;
		}

		int nbytes = recv(client_fd, IN_BUF, IN_BUF_SIZE, 0);

		if (nbytes < 0) {
			fprintf(stderr, "Failed to read bytes: %s", strerror(errno));
			close(client_fd);
			break;
		}

		printf("Received data:\n%.*s", nbytes, IN_BUF);

		HttpResponse res = handle_request(IN_BUF, nbytes);
		size_t res_size;
		const char* res_str = response_to_str(&res, &res_size);
		printf("Sending response:\n%.*s", res_size, res_str);

		if (send(client_fd, res_str, res_size, 0) < 0) {
			fprintf(stderr, "Failed to send response: %s\n", strerror(errno));
		}

		response_destroy(&res);
		printf("Response sent successfully\n");
		// For the time being we only handle one connection
		close(client_fd);
		break;
	}

	close(socket_fd);
	return 0;
}
