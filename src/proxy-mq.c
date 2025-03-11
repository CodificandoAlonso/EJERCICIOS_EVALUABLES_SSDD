#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>

#include "claves.h"
#include "../headers.h"
#define MQ_NAME "/servidor_queue_9453"
#define MAX_MSG_SIZE 1024








int send_request(request *msg) {

    mqd_t mq_server;
    mqd_t client_queue;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(request);
    attr.mq_curmsgs = 0;

    char client[32];
    sprintf(client, "/client_queue_%d", getpid());
    client_queue = mq_open(client, O_CREAT | O_RDONLY, 0644, &attr);
    if (client_queue == -1) {
        return -2;
    }
    mq_server = mq_open("/servidor_queue_9453", O_WRONLY);
    if (mq_server == -1) {
        return -2;
    }
    msg->answer = 0;
    strncpy(msg->client_queue, client, 32);
    if (mq_send(mq_server, (char *)msg, sizeof(request), 0) == -1) {
        perror("Error al enviar\n");
        mq_close(mq_server);
        return -2;
    }
    request answer;
    ssize_t message = mq_receive(client_queue, (char*)&answer, sizeof(request), 0);
    if (message <0) {
        perror("Error al recibir\n");
    }
    //Extraer metodo de recibir mensaje a una func de tipo request que devuelva la estructura recibida.
    //Dropear las colas de mensajes. debuggeando me he creado sin querer 45



    mq_close(mq_server);
    return 0;
}


int destroy() {
    request msg = {2, 0};
    return send_request(&msg);
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request msg = {1, key, "", N_value2, {0}, value3};
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));
    return send_request(&msg);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    request msg = {0};
    msg.type = 5;
    msg.key = key;
    return send_request(&msg);
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request msg = {4, key, "", N_value2, {0}, value3};
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));
    return send_request(&msg);
}

int delete_key(int key) {
    request msg = {3, key};
    return send_request(&msg);
}

int exist(int key) {
    request msg = {6, key};
    return send_request(&msg);
}
