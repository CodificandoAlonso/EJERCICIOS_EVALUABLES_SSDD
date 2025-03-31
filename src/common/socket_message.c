//
// Created by hectorpc on 28/03/25.
//
#include "socket_message.h"
#include "struct.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <inttypes.h>

int isBigEndian(void) {
    unsigned int num = 1;
    // Si el primer byte (dirección más baja) es 1, es little endian
    if (*(char *)&num == 1)
        return 0;  // little endian
    else
        return 1;  // big endian
}

double swap_endian(double value) {
    uint8_t *temp = (uint8_t *)&value;
    char new_double[8];
    new_double[0] = *(temp + 7);
    new_double[1] = *(temp + 6);
    new_double[2] = *(temp + 5);
    new_double[3] = *(temp + 4);
    new_double[4] = *(temp + 3);
    new_double[5] = *(temp + 2);
    new_double[6] = *(temp + 1);
    new_double[7] = *(temp + 0);
    double ret_val = *(double *)&new_double;
    return ret_val;
}


double host_to_net_double(double value)
{
    if (isBigEndian() == 0) //Little Endian
    {
       return swap_endian(value);
    }
    return value;
}

void print_double_hex(double value) {
    uint8_t bytes[8];
    memcpy(bytes, &value, sizeof(bytes));
    for (int i = 0; i < 8; i++) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

double net_to_host_double(double value)
{
    if (isBigEndian() == 0) //Little Endian
    {
        return swap_endian(value);
    }
    return value;
}



int receive_package(int socket,void *message, int size)
{
    int r = 0;
    int left = size;
    void *buffer = message;

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



int receive_message(int socket, request *message)
{
    int type = 0;
    receive_package(socket, &type, sizeof(int));
    message->type = ntohl(type);
    int key = 0;
    receive_package(socket,&key, sizeof(int));
    message->key = ntohl(key);
    ssize_t len_v1 = 0;
    receive_package(socket, &len_v1, sizeof(long));
    receive_package(socket, &message->value_1, len_v1);
    int n2 = 0;
    receive_package(socket, &n2, sizeof(int));
    message->N_value_2 = ntohl(n2);
    for(int i = 0; i< message->N_value_2; i++)
    {
        double value = 0;
        receive_package(socket,&value, sizeof(double));
        message->value_2[i] = net_to_host_double(value);
        print_double_hex(message->value_2[i]);
    }
    int x,y = 0;
    receive_package(socket, &x, sizeof(int));
    receive_package(socket, &y, sizeof(int));
    message->value_3.x = ntohl(x);
    message->value_3.y = ntohl(y);
    int answer = 0;
    receive_package(socket, &answer, sizeof(int));
    message->answer = ntohl(answer);

    return 0;
}

int send_package(int socket, void *message, int size)
{
    int written = 0;
    int left = size;

    while (left > 0) {
        written = write(socket, message, left);
        if (written <= 0) {
            perror("Error reading from socket");
            return -1;
        }
        left -= written;
        message += written;
    }
    return 0;
}



int send_message(int socket, request *answer) {


    __uint32_t type = htonl(answer->type);
    send_package(socket, &type, sizeof(int));
    __uint32_t key = htonl(answer->key);
    send_package(socket, &key, sizeof(int));
    ssize_t len_v1 = strlen(answer->value_1);
    send_package(socket, &len_v1, sizeof(long));
    send_package(socket, &answer->value_1, len_v1);
    __uint32_t N_value_2 = htonl(answer->N_value_2);
    send_package(socket, &N_value_2, sizeof(int));
    for(int i = 0; i< answer->N_value_2; i++)
    {
        double conv_double = host_to_net_double(answer->value_2[i]);
        send_package(socket, &conv_double, sizeof(double));
    }
    __uint32_t x = htonl(answer->value_3.x);
    send_package(socket, &x, sizeof(int));
    __uint32_t y = htonl(answer->value_3.y);
    send_package(socket, &y, sizeof(int));
    __uint32_t response = htonl(answer->answer);
    send_package(socket, &response, sizeof(int));
    return 0;
}
