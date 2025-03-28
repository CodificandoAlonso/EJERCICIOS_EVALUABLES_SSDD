//
// Created by hectorpc on 28/03/25.
//
#include "socket_message.h"
#include "struct.h"
#include <stdio.h>
#include <unistd.h>

int receive_message(int socket, request *message) {
    int r = 0;
    int left = sizeof(request);
    char *buffer = (char *) message;

    while (left > 0) {
        r = read(socket, buffer, left);
        if (r <= 0) {
            perror("Error reading from socket");
            return -1;
        }
        left -= r;
        buffer += r;
    }
    return 0;
}

int send_message(int socket, request *answer) {
    int written = 0;
    int left = sizeof(request);
    char *buffer = (char *) answer;

    while (left > 0) {
        written = write(socket, buffer, left);
        if (written <= 0) {
            perror("Error reading from socket");
            return -1;
        }
        left -= written;
        buffer += written;
    }
    return 0;
}
