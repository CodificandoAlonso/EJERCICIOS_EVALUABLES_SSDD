//
// Created by hector on 3/03/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>


typedef struct value_2 {
    int num_elem;
    double *array_doubles;
} value_2;

struct Coord {
    int x;
    int y;
};

typedef struct request {
    int type;
    int key;
    char *value_1;
    value_2 value_2;
    struct Coord value_3;
} request;

int main(int argc, char **argv) {
    mqd_t server_queue;
    char name[20] = "/servidor_queue_9453";

    // Abrimos la cola en modo lectura/escritura
    server_queue = mq_open(name, O_CREAT| O_WRONLY, 0700, NULL);
    if (server_queue == (mqd_t) -1) {
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
	char buffer[256];
    while(1){
      printf("WELCOME CLIENT, NOW TYPE 'EXIT' TO LOG OUT OR "
		"	'INSERT', 'DELETE', 'DELETE_KEY', 'MODIFY_VALUE','GET_VALUE'"
			" O ME CHUPAS EL PIRULI\n");
        fgets(buffer,sizeof(buffer),stdin);
        if (strcmp(buffer, "EXIT\n") == 0)
        {
            break;
        }
        else if (strcmp(buffer, "INSERT\n")== 0){  //PETICION == 1
			nueva_peticion.type = 1;
            printf("DIME TU KEY\n");
			fgets(buffer,sizeof(buffer),stdin);
            int key = 0;
            if ((key = atoi(buffer)) == 0){
              printf("Era un entero tonto. Mi programa se muere\n");
              return -1;
            }
            nueva_peticion.key = key;
            printf("DIME TU VALUE_1\n");
			fgets(buffer,sizeof(buffer),stdin);
            memcpy(&nueva_peticion.value_1,buffer, sizeof(buffer));
            printf("DIME LA LONGITUD DE TU VALUE_2\n");
			fgets(buffer,sizeof(buffer),stdin);
            if ((key = atoi(buffer)) == 0){
              printf("Era un entero tonto. Mi programa se muere\n");
              return -1;
            }
            else if(key > 32){
              printf("Te pasaste de verga\n");
              return -1;
            }
            nueva_peticion.value_2.num_elem = key;
            nueva_peticion.value_2.array_doubles = malloc(key * sizeof(double));
            for(int i = 0;i < key; i++)
            {
            printf("DIME tu elemento %d del value2\n", i);
			fgets(buffer,sizeof(buffer),stdin);
            if ((key = atof(buffer)) == 0){
              printf("Era un double tonto. Mi programa se muere\n");
              return -1;
            }
            nueva_peticion.value_2.array_doubles[i] = key;
            }
            printf("DIME tu value x\n");
            fgets(buffer,sizeof(buffer),stdin);
            if ((key = atof(buffer)) == 0){
                printf("Era un double tonto. Mi programa se muere\n");
                return -1;
            }
            nueva_peticion.value_3.x = key;
            printf("DIME tu value y\n");
            fgets(buffer,sizeof(buffer),stdin);
            if ((key = atof(buffer)) == 0){
                printf("Era un double tonto. Mi programa se muere\n");
                return -1;
            }
            nueva_peticion.value_3.y = key;

        }
        else if (strcmp(buffer, "DELETE\n")== 0){  //PETICION == 2

        }
        else if (strcmp(buffer, "DELETE_KEY\n")== 0){  //PETICION == 3

        }
        else if (strcmp(buffer, "MODIFY_VALUE\n")== 0){  //PETICION == 4

        }
        else if (strcmp(buffer, "GET_VALUE\n")== 0){  //PETICION == 5

        }
        else{
          printf("ME CHUPAS EL PIRULI\n");
        }
    }




    // Cerrar la cola
    mq_close(server_queue);

    return 0;
}