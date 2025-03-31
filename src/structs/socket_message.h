//
// Created by hectorpc on 28/03/25.
//

#ifndef SOCKET_MESSAGE_H
#define SOCKET_MESSAGE_H

#include "claves.h"
#include "struct.h"
int receive_message(int socket, request *message);
int send_message(int socket, request *answer);
int send_package(int socket, void *message, int size);
int receive_package(int socket,void *message, int size);
double net_to_host_double(double value);
double host_to_net_double(double value);
double swap_endian(double value);
int isBigEndian(void);

#endif //SOCKET_MESSAGE_H
