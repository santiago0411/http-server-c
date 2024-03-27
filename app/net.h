#ifndef NET_H
#define NET_H

#include <stdint.h>

typedef struct
{
    int Socket;
    const char RemoteAddress[15];
    uint16_t RemotePort;
} ClientInfo;

int create_socket(uint16_t port);
ClientInfo* accept_client(int server_socket);
void disconnect_client(ClientInfo* client);

#endif //NET_H
