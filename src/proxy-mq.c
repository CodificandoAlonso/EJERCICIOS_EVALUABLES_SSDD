#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include "claves.h"

#define MQ_NAME "/servidor_queue_9453"
#define MAX_MSG_SIZE 1024

typedef struct {
    int type;
    int key;
    char value_1[256];
    int N_value_2;
    double value_2[32];
    struct Coord value_3;
} request_t;

int send_request(request_t *msg) {

    mqd_t mq_server;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(request_t);
    attr.mq_curmsgs = 0;

    mq_server = mq_open("/servidor_queue_9453", O_WRONLY);
    if (mq_server == -1) {
        return -2;
    }


    if (mq_send(mq_server, (char *)msg, sizeof(request_t), 0) == -1) {
        mq_close(mq_server);
        return -2;
    }

    mq_close(mq_server);
    return 0;
}


int destroy() {
    request_t msg = {2, 0};
    return send_request(&msg);
}

int set_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request_t msg = {1, key, "", N_value2, {0}, value3};
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));
    return send_request(&msg);
}

int get_value(int key, char *value1, int *N_value2, double *V_value2, struct Coord *value3) {
    request_t msg = {5, key};
    return send_request(&msg);
}

int modify_value(int key, char *value1, int N_value2, double *V_value2, struct Coord value3) {
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request_t msg = {4, key, "", N_value2, {0}, value3};
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));
    return send_request(&msg);
}

int delete_key(int key) {
    request_t msg = {3, key};
    return send_request(&msg);
}

int exist(int key) {
    request_t msg = {6, key};
    return send_request(&msg);
}
