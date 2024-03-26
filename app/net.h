#ifndef NET_H
#define NET_H

#include <stdint.h>

int create_socket(uint16_t port);
int accept_client(int socket_fd);

#endif //NET_H
