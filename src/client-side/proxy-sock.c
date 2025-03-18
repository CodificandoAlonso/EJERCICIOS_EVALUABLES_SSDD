#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>

#include "claves.h"
#include "struct.h"
#define MQ_SERVER_NAME "/servidor_queue_9453"
#define MAX_MSG_SIZE 1024

/**
 * @brief
 * Esta función se encarga de recoger la respuesta del servidor y devolverla en forma de estructura
 * a la función correspondiente, de cara a devolver los valores adecuados al cliente.
 */
int get_response(request* answer)
{
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(request);
    attr.mq_curmsgs = 0;

    char client[32];
    sprintf(client, "/client_queue_%d", getpid());
    const mqd_t client_queue = mq_open(client, O_CREAT | O_RDONLY, 0644, &attr);
    if (client_queue == -1)
    {
        return -2;
    }


    const ssize_t message = mq_receive(client_queue, (char*)answer, sizeof(request), 0);
    if (message < 0)
    {
        printf("Error receiving from the server\n");
        return -2;
    }
    mq_close(client_queue);
    mq_unlink(client);
    return 0;
}

/**
 * @brief
 * Esta función se encarga de enviar las peticiones al srevidor. Es invocada por todos los servicios
 * ofrecidos por claves dentro de proxy-mq.
 */
int send_request(request* msg)
{
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(request);
    attr.mq_curmsgs = 0;

    char client[32];
    sprintf(client, "/client_queue_%d", getpid());
    mqd_t mq_server = mq_open("/servidor_queue_9453", O_WRONLY, 0644, &attr);
    if (mq_server == -1)
    {
        return -2;
    }
    msg->answer = 0;
    strncpy(msg->client_queue, client, 32);
    if (mq_send(mq_server, (char*)msg, sizeof(request), 0) == -1)
    {
        perror("Error comunicating central server\n");
        mq_close(mq_server);
        return -2;
    }
    mq_close(mq_server);
    return 0;
}


int destroy()
{
    request msg = {0};
    msg.type = 2;
    if(send_request(&msg)< 0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    if (answer.answer == -1)
    {
        return -1;
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
    if(send_request(&msg)< 0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    if (answer.answer == -1)
    {
        return -1;
    }
    return answer.answer;
}



int get_value(int key, char* value1, int* N_value2, double* V_value2, struct Coord* value3)
{
    request msg = {0};
    msg.type = 5;
    msg.key = key;
    if(send_request(&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(&answer) < 0)
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
    if(send_request(&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    if (answer.answer == -1)
    {
        return -1;
    }
    return answer.answer;
}

int delete_key(int key)
{
    request msg = {0};
    msg.type = 3; //DELETE_KEY
    msg.key = key;
    if(send_request(&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    if (answer.answer == -1)
    {
        return -1;
    }
    return 0;
}

int exist(int key)
{
    request msg = {0};
    msg.type = 6; //EXIST
    msg.key = key;
    if(send_request(&msg)<0)
    {
        return -2;
    }
    request answer = {0};
    if (get_response(&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    if (answer.answer == -1)
    {
        return -1;
    }
    return 0;
}
