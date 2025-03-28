#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <socket_message.h>
#include <unistd.h>
#include "claves.h"
#include "struct.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX_MSG_SIZE 1024

/**
 * @brief
 * Esta función se encarga de recoger la respuesta del servidor y devolverla en forma de estructura
 * a la función correspondiente, de cara a devolver los valores adecuados al cliente.
 */
int get_response(int socket, request* answer)
{
    return receive_message(socket, answer);
}


/**
 * @brief
 * Esta función se encarga de inicializar la conexión al socket que usará el cliente para comunicarse con el servidor.
 * Utiliza como ip y puerto las generadas por las variables de entorno de la terminal en la que se ejecutanç
 */
int connect_socket_to_server()
{
    int sock = 0;
    struct sockaddr_in server_addr = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        return -2;
    }

    //Puerto e ip del servidor
    bzero((char *)&server_addr, sizeof(server_addr));
    char *ip_str = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");
    if (!ip_str || !port_str){
        fprintf(stderr, "ENV Variables IP_TUPLAS o PORT_TUPLAS not defined\n");
        return -2;
    }
    int port_num = atoi(port_str);
    if (port_num <= 0 || port_num > 65535){
        fprintf(stderr, "Invalid port\n");
        return -2;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    if(inet_aton(ip_str, &server_addr.sin_addr) == 0){
        fprintf(stderr, "Invalid IP Adress\n");
        exit(-2);
    }
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting\n");
        close(sock);
        return -2;
    }
    return sock;
}

/**
 * @brief
 * Esta función se encarga de enviar las peticiones al srevidor. Es invocada por todos los servicios
 * ofrecidos por claves dentro de proxy-mq.
 */
int send_request(int sock, request* msg)
{
    msg->answer = 0;
    if(send_message(sock,msg)< 0)
    {
        return -2;
    }
    return 0;
}


int destroy()
{
    request msg = {0};
    msg.type = 2;
    int sock = connect_socket_to_server();
    if (sock < 0)
    {
        return -2;
    }
    if(send_request(sock,&msg)< 0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}

int set_value(int key, char* value1, int N_value2, double* V_value2, struct Coord value3)
{
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request msg = {0};
    msg.type = 1;
    msg.key = key;
    msg.N_value_2 = N_value2;
    msg.value_3 = value3;
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));
    int sock = connect_socket_to_server();
    if (sock < 0)
    {
        return -2;
    }
    if(send_request(sock,&msg)< 0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}



int get_value(int key, char* value1, int* N_value2, double* V_value2, struct Coord* value3)
{
    request msg = {0};
    msg.type = 5;
    msg.key = key;
    int sock = connect_socket_to_server();
    if (sock < 0)
    {
        return -2;
    }
    if(send_request(sock,&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    if (answer.answer == -1)
    {
        return -1;
    }
    memcpy(value1, answer.value_1, sizeof(answer.value_1) * sizeof(char));
    *N_value2 = answer.N_value_2;
    for (int i = 0; i < answer.N_value_2; i++)
    {
        V_value2[i] = answer.value_2[i];
    }
    value3->x = answer.value_3.x;
    value3->y = answer.value_3.y;
    return answer.answer;
}

int modify_value(int key,char* value1, int N_value2, double* V_value2,
                 struct Coord value3)
{
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request msg = {0};
    msg.type = 4; //MODIFY_VALUE
    msg.key = key;
    msg.N_value_2 = N_value2;
    msg.value_3 = value3;
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));
    int sock = connect_socket_to_server();
    if (sock < 0)
    {
        return -2;
    }
    if(send_request(sock,&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}

int delete_key(int key)
{
    request msg = {0};
    msg.type = 3; //DELETE_KEY
    msg.key = key;
    int sock = connect_socket_to_server();
    if(send_request(sock,&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}

int exist(int key)
{
    request msg = {0};
    msg.type = 6; //EXIST
    msg.key = key;
    int sock = connect_socket_to_server();
    if(send_request(sock,&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}
