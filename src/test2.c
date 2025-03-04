//
// Created by hector on 4/03/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>



struct Coord {
    int x;
    int y;
};

typedef struct request {
    int type;
    int key;
    char value_1[256];
    int N_value_2;
    double value_2[32];
    struct Coord value_3;
} request;


void print_request(request req)
{
    printf("\n===== REQUEST INFO =====\n");
    printf("Type: %d\n", req.type);
    printf("Key: %d\n", req.key);
    printf("Value 1: %s\n", req.value_1);

    printf("Value 2 (Array - %d elementos): ", req.N_value_2);
    for (int i = 0; i < req.N_value_2; i++)
    {
        printf("%.2f ", req.value_2[i]);
    }
    printf("\n");

    printf("Value 3 (Coord): x = %d, y = %d\n", req.value_3.x, req.value_3.y);
    printf("=======================\n\n");
}




int main(int argc, char **argv) {

    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Máximo 50 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;



    mqd_t server_queue;
    char name[20] = "/servidor_queue_9453";

    // Abrimos la cola en modo lectura/escritura
    server_queue = mq_open(name, O_CREAT| O_WRONLY | O_NONBLOCK, 0660, NULL);
    if (server_queue == (mqd_t) -1) {
        perror("Error abriendo la cola del servidor");
        return -1;
    }
    printf("Todo bien abriendo la cola del servidor con fd: %d\n", server_queue);

    // Crear la petición
    request nueva_peticion;
    nueva_peticion.type = 1;
    nueva_peticion.key = 2;
    char puta[6] = "HOLIII";
    memcpy(nueva_peticion.value_1, puta,sizeof(puta));


    nueva_peticion.N_value_2 = 4;

    // Asignar valores al array
    double valores[] = {5.44, 4.33, 2.22, 1.34};
    memcpy(nueva_peticion.value_2, valores, nueva_peticion.N_value_2 * sizeof(double));


    // Inicializar la estructura Coord
    struct Coord tete;
    tete.x = 1;
    tete.y = 3;
    nueva_peticion.value_3 = tete;
    print_request(nueva_peticion);
    // Enviar la petición a la cola de mensajes
    if (mq_send(server_queue, (char *)&nueva_peticion, sizeof(nueva_peticion), 1) == -1) {
        perror("Error enviando mensaje a la cola");
        return -1;
    }

    printf("Mensaje enviado con éxito\n");


    // Cerrar la cola
    mq_close(server_queue);

    return 0;
}