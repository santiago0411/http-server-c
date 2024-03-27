#include "net.h"

#include <assert.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

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

ClientInfo* accept_client(const int server_socket)
{
    struct sockaddr_in client_addr;
    const int client_addr_len = sizeof(client_addr);

    const int client_fd = accept(server_socket, (struct sockaddr *) &client_addr, (socklen_t*)&client_addr_len);
    if (client_fd < 0) {
        fprintf(stderr, "Error accepting connection: %s.\n", strerror(errno));
        return NULL;
    }

    const uint8_t octets[] = {
        (client_addr.sin_addr.s_addr >> 0) & 0xFF,
        (client_addr.sin_addr.s_addr >> 8) & 0xFF,
        (client_addr.sin_addr.s_addr >> 16) & 0xFF,
        (client_addr.sin_addr.s_addr >> 24) & 0xFF,
    };

    ClientInfo* c = malloc(sizeof(ClientInfo));
    assert(c && "Out of ram lol");

    c->Socket = client_fd;
    c->RemotePort = client_addr.sin_port;
    memset((void*)c->RemoteAddress, sizeof(c->RemoteAddress), 0);
    snprintf((char*)c->RemoteAddress, sizeof(c->RemoteAddress), "%d.%d.%d.%d",
        octets[0], octets[1],
        octets[2], octets[3]);

    printf("Client %s:%u connected!\n", c->RemoteAddress, c->RemotePort);
    return c;
}

void disconnect_client(ClientInfo* client)
{
    if (client) {
        close(client->Socket);
        free(client);
    }
}
