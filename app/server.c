#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef LOCAL_TEST
#define PORT 30505
#else
#define PORT 4221
#endif

#define IN_BUF_SIZE 1024
static uint8_t IN_BUF[IN_BUF_SIZE];

int accept_client(const int socket_fd)
{
	struct sockaddr_in client_addr;
	const int client_addr_len = sizeof(client_addr);

	const int client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if (client_fd < 0) {
		fprintf(stderr, "Error accepting connection: %s.\n", strerror(errno));
		return -1;
	}

	const uint8_t octets[] = {
		(client_addr.sin_addr.s_addr >> 0) & 0xFF,
		(client_addr.sin_addr.s_addr >> 8) & 0xFF,
		(client_addr.sin_addr.s_addr >> 16) & 0xFF,
		(client_addr.sin_addr.s_addr >> 24) & 0xFF,
	};

	printf("Client %d.%d.%d.%d:%d connected!\n",
		octets[0], octets[1],
		octets[2], octets[3],
		client_addr.sin_port
	);

	return client_fd;
}

int create_socket(const uint16_t port)
{
	const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		fprintf(stderr, "Socket creation failed: %s...\n", strerror(errno));
		return -1;
	}

	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		fprintf(stderr, "SO_REUSEPORT failed: %s \n", strerror(errno));
		return -1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET ,
		.sin_port = htons(port),
		.sin_addr = { htonl(INADDR_ANY) },
	};

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		fprintf(stderr, "Bind failed: %s \n", strerror(errno));
		return -1;
	}

	printf("Bound on port %d\n", port);
	return server_fd;
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

		printf("%.*s", nbytes, IN_BUF);

		const char* response = "HTTP/1.1 200 OK\r\n\r\n";
		printf("Sending response:\n%s", response);
		if (send(client_fd, response, strlen(response), 0) < 0) {
			fprintf(stderr, "Failed to send response: %s\n", strerror(errno));
		}

		printf("Response sent successfully\n");
		// For the time being we only handle one connection
		close(client_fd);
		break;
	}

	close(socket_fd);
	return 0;
}
