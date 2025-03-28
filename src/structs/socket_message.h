//
// Created by hectorpc on 28/03/25.
//

#ifndef SOCKET_MESSAGE_H
#define SOCKET_MESSAGE_H

#include "claves.h"
#include "struct.h"
int receive_message(int socket, request *message);
int send_message(int socket, request *answer);


#endif //SOCKET_MESSAGE_H
