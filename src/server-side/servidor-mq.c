#define _GNU_SOURCE
#include<stdio.h>
#include<pthread.h>
#include "../claves.h"
#include<mqueue.h>
#include<stdlib.h>
#include "../claves.h"
#include<sqlite3.h>
#include<errno.h>
#include <string.h>
#include<unistd.h>


#define MAX_THREADS 10





//Creación de hilos, arrays de hilos y contador de hilos ocupados
pthread_t thread_pool[MAX_THREADS];
int free_threads_array[MAX_THREADS];
int workload = 0;
int free_mutex_copy_params_cond = 0;
pthread_mutex_t mutex_copy_params;
pthread_cond_t cond_wait_cpy;
pthread_mutex_t mutex_threads;
pthread_cond_t cond_wait_threads;



/**@brief Esta función se usa para rellenar el array auxiliar que indica qué hilo está trabajando(1)
 *y cuál está libre(0)
 */
void pad_array()

{
    for (int i = 0; i < MAX_THREADS; i++)
    {
        free_threads_array[i] = 0;
    }
}


//LA COLA ACABA CON 9453

typedef struct value_2
{
    int num_elem;
    double* array_doubles;
} value_2;


typedef struct request
{
    int type;
    int key;
    char* value_1;
    value_2 value_2;
    struct Coord value_3;
} request;

typedef struct parameters_to_pass
{
    request* this_request;
    int identifier;
} parameters_to_pass;


/**@brief Esta es la función que ejecutan los distintos hilos dentro de nuestra pool de hilos
 *Requiere de la dir de memoria de uns estructura de tipo parameters_to_pass que se compone de
 *una estructura petición y una variable id, que identifica al thread que está ejecutando, para
 *indicar en el array de threads que ese hilo no está disponible en el momento. Una vez realiza la copia
 *local de los datos se encargará de gestionar las distintas llamadas de claves.c para realizar las gestiones
 *en la base de datos correspondiente
 */
void process_request(parameters_to_pass* parameters)
{
    pthread_mutex_lock(&mutex_copy_params);
    request local_request = *parameters->this_request;
    int local_id = parameters->identifier;
    free_mutex_copy_params_cond = 1;
    free_threads_array[local_id] = 1;
    printf("Hilo %d ocupado\n", local_id);
    pthread_cond_signal(&cond_wait_cpy);
    pthread_mutex_unlock(&mutex_copy_params);
    sleep(10);

    printf("Acabado el trabajo el hilo %d\n", local_id);
    pthread_exit(0);
}




/**@brief Función implementada al inicializarse el servidor que creará las tablas de SQL que se encargarán
 *de mantener nuestros datos ordenados. Hemos creado 2 tablas, una "data" que guardará tanto el id(Primary key)
 *como value1 y value3. Value2 como es un array de longitud variable, nos hemos creado una tabla "value2_all"
 *que hereda de data la PK a modo de Foreign Key con UPDATE y DELETE CASCADE(Si se borra la pk de data, se borrarán
 *todas las referencias a ella en "value2-all". La pk de esta tabla será para cada elemento del array, la conversión
 *a entero de la concatenación de la PK con el índice del elemento en el array. Por ejemplo, si para id 3 tengo que
 *insertar el vector {3.44, 2.15, 14.33} tendré 3 filas en esta nueva tabla
 *  PK   FK   VALUE
 *  30   3    3.44
 *  31   3    2.15
 *  32   3    14.33
 */
void create_table(sqlite3* db)
{
    char* message_error = NULL;
    char* new_table =
        "CREATE TABLE IF NOT EXISTS data("
        " data_key INTEGER PRIMARY KEY,"
        " value1 TEXT,"
        " x INTEGER,"
        " y INTEGER"
        ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR CREANDO TABLA 1\n");
        exit(-4);
    }
    message_error = NULL;
    new_table =
        "CREATE TABLE IF NOT EXISTS value2_all("
        " id TEXT PRIMARY KEY,"
        " data_key INTEGER,"
        " value REAL,"
        "CONSTRAINT fk_origin FOREIGN KEY(data_key) REFERENCES data(pk) ON DELETE CASCADE ON UPDATE CASCADE"
        ");";
    if (sqlite3_exec(db, new_table, NULL, NULL, &message_error) != SQLITE_OK)
    {
        fprintf(stderr, "ERROR CREANDO TABLA 2\n");
        exit(-4);
    }
}



/**@brief PRUEBA. HABRA QUE MIGRARLA A CLAVES.C
 */
void insert_data_TEST(sqlite3* db)
{
    char* error_message = NULL;
    char insert[256];

    //Insertar los primeros parametros en data
    sprintf(insert,
            "INSERT into data(data_key, value1,x,y) "
            " VALUES(%d, '%s', %d ,%d)", 3, "SEXO", 5, 5);
    printf("Esto vale insert: %s\n", insert);
    int test;
    if ((test = sqlite3_exec(db, insert, NULL, NULL, &error_message)) != SQLITE_OK)
    {
        if (test != SQLITE_CONSTRAINT)
        {
            fprintf(stderr, "ERROR insertando en TABLA\n");
            exit(-4);
        }
        printf("DUPLICADA LA PK BOBO\n");
    }
    double var2[3] = {3.44, 4.3, 5.5};
    char primary_key[20];
    for (int i = 0; i < 3; i++)
    {
        sprintf(primary_key, "%d%d", 3, i);
        sprintf(insert,
                "INSERT into value2_all(id,data_key,value) "
                " VALUES(%s, %d, %f)", primary_key, 3, var2[i]);
        printf("Esto vale insert: %s\n", insert);
        if ((test = sqlite3_exec(db, insert, NULL, NULL, &error_message)) != SQLITE_OK)
        {
            if (test != SQLITE_CONSTRAINT)
            {
                fprintf(stderr, "ERROR insertando en TABLA\n");
                exit(-4);
            }
            printf("DUPLICADA LA PK BOBO\n");
        }
    }
}




int main(int argc, char* argv[])
{
    //Creando e inicializando la base de datos
    sqlite3* database;
    int create_database = sqlite3_open("database.db", &database);
    if (create_database != SQLITE_OK)
    {
        fprintf(stderr, "Error opening the database\n");
        exit(-1);
    }
    printf("Exito abriendo la base de datos\n");

    //Creo la tabla principal "data"
    create_table(database);
    insert_data_TEST(database);


    pad_array();


    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Máximo 10 mensajes en la cola
    attr.mq_msgsize = sizeof(request); // Tamaño del mensaje debe ser igual al struct
    attr.mq_curmsgs = 0;


    //inicializacion mutex para la copia local de parametros
    pthread_mutex_init(&mutex_copy_params, NULL);
    pthread_cond_init(&cond_wait_cpy, NULL);
    //inicializacion mutex para la variacion del numero de threads usados
    pthread_mutex_init(&mutex_threads, NULL);
    pthread_cond_init(&cond_wait_threads, NULL);


    //Inicializo y abro la cola del servidor
    mqd_t server_queue;
    char nombre[20] = "/servidor_queue_9453";
    server_queue = mq_open(nombre, O_CREAT | O_RDONLY, 0700, &attr);
    if (server_queue == -1)
    {
        mq_close(server_queue);
        fprintf(stderr, "Error abriendo la cola del servidor: %s %s\n", strerror(errno), nombre);
        printf("Error abriendo la cola del servidor\n");
        return -1;
    }
    printf("Todo bien abriendo la cola del servidor con fd: %d\n", server_queue);



    request new_request;
    char buffer[10];
    //Gestion de la concurrencia con las peticiones
    while (1)
    {
        printf("PARA SALIR PON 'exit' mamawebo\n");
        fgets(buffer,sizeof(buffer),stdin);
        if (strcmp(buffer, "exit"))
        {
            break;
        }

        //Esto intenta realizar un join no bloqueante
        pthread_mutex_lock(&mutex_threads);
        for (int i = 0; i < MAX_THREADS; i++) {
            if (free_threads_array[i] == 1) { //Si el hilo ha estado trabajando miro a ver si puedo hacerle join
                if (pthread_tryjoin_np(thread_pool[i], NULL)== 0) {
                    free_threads_array[i] = 0; //Al ruedo de nuevo, maquina
                    workload--; // Avisar de que se puede currar
                    printf("Hilo %d liberado y listo para reutilizarse\n", i);
                }
            }
        }
        pthread_mutex_unlock(&mutex_threads);


        ssize_t message = mq_receive(server_queue, (char*)&new_request, sizeof(request), 0);
        if (message >= 0)
        {
            printf("Se ha recibido un mensaje con id %d\n", new_request.key);


            //Miro el primer hilo disponible y le mando currar. JAAAAPIUU, a currar esclavo
            //Necesita seccion critica por el acceso al array
            pthread_mutex_lock(&mutex_threads);
            for(int i =0; i< MAX_THREADS; i++)
            {
                if(free_threads_array[i] == 0){
                    parameters_to_pass params;
                    params.identifier = i;
                    params.this_request = &new_request;
                    pthread_create(&thread_pool[i],NULL,(void *)process_request,&params);
                    break;
                }
            }
            pthread_mutex_unlock(&mutex_threads);


            //Solo hago la espera si no hay ningun hilo currando. Si son todos unos putos vagos de mierda no entro
            pthread_mutex_lock(&mutex_threads);
            if (workload >0)
            {
                pthread_mutex_lock(&mutex_copy_params);
                while(free_mutex_copy_params_cond == 0)
                    pthread_cond_wait(&cond_wait_cpy, &mutex_copy_params);
                free_mutex_copy_params_cond = 0;
                pthread_mutex_unlock(&mutex_copy_params);
            }
            else
            {
                pthread_mutex_unlock(&mutex_threads);
            }
        }
        else {
            //si se me lia el mensaje
            printf("error al recibir, tonto\n");
            //DE MOMENTO SI ES ERROR EN COMUNICACION ES ERROR -2
            exit(-2);
        }
    }
    return 0;
}
