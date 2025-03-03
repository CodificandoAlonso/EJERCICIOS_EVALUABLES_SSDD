//
// Created by hector on 3/03/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>


/*
typedef struct value_2 {
    int num_elem;
    double *array_doubles;
} value_2;
*/

struct Coord
{
    int x;
    int y;
};

typedef struct request
{
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

void init_request(request* req)
{
    req->type = 0;
    req->key = 0;
    req->N_value_2 = 0;
    for (int i = 0; i < 32; i++)
    {
        req->value_2[i] = 0;
    }
    req->value_3.x = 0;
    req->value_3.y = 0;
}


int main(int argc, char** argv)
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 50; // Máximo 50 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;

    mqd_t server_queue;
    char name[20] = "/servidor_queue_9453";

    // Abrimos la cola en modo lectura/escritura
    server_queue = mq_open(name, O_CREAT | O_WRONLY, 0700, &attr);
    if (server_queue == (mqd_t)-1)
    {
        perror("Error abriendo la cola del servidor");
        return -1;
    }
    printf("Todo bien abriendo la cola del servidor con fd: %d\n", server_queue);


    /*
    // Crear la petición
    request nueva_peticion;
    nueva_peticion.type = 1;
    nueva_peticion.key = 2;
    nueva_peticion.value_1 = "HOLIII";

    // Inicializar value_2 correctamente
    value_2 newval;
    newval.num_elem = 4;
    newval.array_doubles = malloc(newval.num_elem * sizeof(double)); // Reservamos memoria

    if (newval.array_doubles == NULL) {
        perror("Error al asignar memoria para array_doubles");
        return -1;
    }

    // Asignar valores al array
    double valores[] = {5.44, 4.33, 2.22, 1.34};
    memcpy(newval.array_doubles, valores, newval.num_elem * sizeof(double));

    nueva_peticion.value_2 = newval;

    // Inicializar la estructura Coord
    struct Coord tete;
    tete.x = 1;
    tete.y = 3;
    nueva_peticion.value_3 = tete;

    // Enviar la petición a la cola de mensajes
    if (mq_send(server_queue, (char *)&nueva_peticion, sizeof(nueva_peticion), 1) == -1) {
        perror("Error enviando mensaje a la cola");
        return -1;
    }

    printf("Mensaje enviado con éxito\n");

    // Liberar memoria reservada
    free(newval.array_doubles);
    */

    request nueva_peticion;
    init_request(&nueva_peticion);
    char buffer[256];
    while (1)
    {
        printf("WELCOME CLIENT, NOW TYPE 'EXIT' TO LOG OUT OR "
            "	'INSERT', 'DELETE', 'DELETE_KEY', 'MODIFY_VALUE','GET_VALUE'"
            " O ME CHUPAS EL PIRULI\n");
        fgets(buffer, sizeof(buffer),stdin);
        if (strcmp(buffer, "EXIT\n") == 0)
        {
            break;
        }
        else if (strcmp(buffer, "INSERT\n") == 0)
        {
            //INSERT == 1
            nueva_peticion.type = 1;
            printf("DIME TU KEY\n");
            fgets(buffer, sizeof(buffer),stdin);
            int key = 0;
            if ((key = atoi(buffer)) == 0)
            {
                printf("Era un entero tonto. Mi programa se muere\n");
                return -1;
            }
            nueva_peticion.key = key;
            printf("DIME TU VALUE_1\n");
            fgets(buffer, sizeof(buffer),stdin);
            //nueva_peticion.value_1 = malloc(sizeof(buffer) * sizeof(char));
            memcpy(nueva_peticion.value_1, &buffer, sizeof(buffer));
            printf("%s\n", nueva_peticion.value_1);
            printf("DIME LA LONGITUD DE TU VALUE_2\n");
            fgets(buffer, sizeof(buffer),stdin);
            if ((key = atoi(buffer)) == 0)
            {
                printf("Era un entero tonto. Mi programa se muere\n");
                return -1;
            }
            if (key > 32)
            {
                printf("Te pasaste de verga\n");
                return -1;
            }
            printf("%d\n", key);
            /*
            value_2 temp_value_2;
            temp_value_2.num_elem = key;
            temp_value_2.array_doubles = malloc(key * sizeof(double));
            */
            nueva_peticion.N_value_2 = key;
            double* pruebaaa;
            pruebaaa = malloc(key * sizeof(double));
            double insert_data = 0;
            for (int i = 0; i < key; i++)
            {
                printf("DIME tu elemento %d del value2\n", i);
                fgets(buffer, sizeof(buffer),stdin);
                if ((sscanf(buffer, "%lf", &insert_data)) == 0)
                {
                    printf("Era un double tonto. Mi programa se muere\n");
                    free(pruebaaa);
                    mq_close(server_queue);
                    return -1;
                }
                pruebaaa[i] = insert_data;
            }
            memcpy(nueva_peticion.value_2, pruebaaa, key * sizeof(double));

            struct Coord temp_coord;
            printf("DIME tu value x\n");
            fgets(buffer, sizeof(buffer),stdin);
            if ((key = atoi(buffer)) == 0)
            {
                printf("Era un int tonto. Mi programa se muere\n");
                free(pruebaaa);
                return -1;
            }
            temp_coord.x = key;
            printf("DIME tu value y\n");
            fgets(buffer, sizeof(buffer),stdin);
            if ((key = atoi(buffer)) == 0)
            {
                printf("Era un entero tonto. Mi programa se muere\n");
                free(pruebaaa);
                return -1;
            }
            temp_coord.y = key;
            nueva_peticion.value_3 = temp_coord;
            if (mq_send(server_queue, (char*)&nueva_peticion, sizeof(nueva_peticion), 1) == -1)
            {
                perror("Error enviando mensaje a la cola");
                free(pruebaaa);
                return -1;
            }
            printf("Enviado mensaje uuuu\n");
            //free(nueva_peticion.value_1);
            //free(temp_value_2.array_doubles);
        }
        else if (strcmp(buffer, "DELETE\n") == 0)
        {
            //DELETE == 2
        }
        else if (strcmp(buffer, "DELETE_KEY\n") == 0)
        {
            //DELETE_KEY == 3
        }
        else if (strcmp(buffer, "MODIFY_VALUE\n") == 0)
        {
            //MODIFY == 4
        }
        else if (strcmp(buffer, "GET_VALUE\n") == 0)
        {
            //GET_VALUE == 5
        }
        else if (strcmp(buffer, "EXIST\n") == 0) //EXISTS == 6
        {
        }
        else
        {
            printf("ME CHUPAS EL PIRULI\n");
        }
    }


    // Cerrar la cola
    mq_close(server_queue);


    return 0;
}
