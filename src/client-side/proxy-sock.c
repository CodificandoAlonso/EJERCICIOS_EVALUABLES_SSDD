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
 * Utiliza como ip y puerto las generadas por las variables de entorno de la terminal en la que se ejecutan
 */
int connect_socket_to_server()
{
    int sock = 0;
    struct sockaddr_in server_addr = {0};

    // Crear el socket TCP.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        return -2;
    }

    // Inicializar la estructura del servidor.
    bzero((char *)&server_addr, sizeof(server_addr));

    // Obtener IP y puerto desde las variables de entorno.
    char *ip_str = getenv("IP_TUPLAS");
    char *port_str = getenv("PORT_TUPLAS");
    if (!ip_str || !port_str){
        fprintf(stderr, "ENV Variables IP_TUPLAS o PORT_TUPLAS not defined\n");
        return -2;
    }
    // Convertir el puerto a número entero y validar.
    int port_num = atoi(port_str);
    if (port_num <= 0 || port_num > 65535){
        fprintf(stderr, "Invalid port\n");
        return -2;
    }

    // Configurar la dirección del servidor.
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    if(inet_aton(ip_str, &server_addr.sin_addr) == 0){
        fprintf(stderr, "Invalid IP Adress\n");
        exit(-2);
    }
    // Conectar con el servidor.
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting\n");
        close(sock);
        return -2;
    }
    return sock;
}

/**
 * @brief
 * Esta función se encarga de enviar las peticiones al servidor. Es invocada por todos los servicios
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

/**
 * @brief Envía una petición de destrucción al servidor.
 *
 * Esta función prepara una petición con tipo 2 y la envía al servidor. Tras enviar la petición, espera la respuesta
 * del servidor y retorna el valor recibido.
 *
 * @return int Código de respuesta del servidor o -2 en caso de error.
 */
int destroy()
{
    request msg = {0};
    msg.type = 2;

    // Conectar al servidor.
    int sock = connect_socket_to_server();
    if (sock < 0)
    {
        return -2;
    }
    // Enviar la petición.
    if(send_request(sock,&msg)< 0)
    {
        return -2;
    }
    request answer = {0};
    // Recibir la respuesta del servidor.
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}


/**
 * @brief Envía una petición para establecer un valor asociado a una clave.
 *
 * Primero valida el formato de los campos: El string 'value1' no excede los 255 caracteres y  N_value2 está en el rango
 * permitido (1 a 32). Posteriormente, se prepara la estructura de petición con tipo 1, se copian los datos
 * correspondientes y se envía al servidor. Se espera una respuesta y se devuelve el valor resultante.
 *
 * @param key Clave identificadora.
 * @param value1 Cadena de caracteres asociada.
 * @param N_value2 Número de elementos del array de double.
 * @param V_value2 Puntero al array de doubles.
 * @param value3 Estructura Coord que contiene coordenadas.
 * @return int Código de respuesta del servidor o -1/-2 en caso de error.
 */
int set_value(int key, char* value1, int N_value2, double* V_value2, struct Coord value3)
{
    // Validación de parámetros.
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request msg = {0};
    msg.type = 1;
    msg.key = key;
    msg.N_value_2 = N_value2;
    msg.value_3 = value3;

    // Copiar los datos en la estructura del mensaje.
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));

    // Conectar al servidor.
    int sock = connect_socket_to_server();
    if (sock < 0)
    {
        return -2;
    }

    // Enviar la petición.
    if(send_request(sock,&msg)< 0)
    {
        return -2;
    }

    // Recibir y procesar la respuesta.
    request answer = {0};
    if (get_response(sock,&answer) < 0)
    {
        perror("Error receiving from the server\n");
        return -2;
    }
    return answer.answer;
}


/**
 * @brief Solicita y obtiene un valor asociado a una clave desde el servidor.
 *
 * Esta función prepara una petición para solicitar el valor asociado a 'key'.
 * Tras enviar la petición y recibir la respuesta, se pueden dar dos casos:
 * La clave no existe y devolverá -1
 * La clave existe y se devuelven los valores asociados a ellas
 *
 * @param key Clave identificadora.
 * @param value1 Buffer donde se almacenará la cadena de caracteres recibida.
 * @param N_value2 Puntero donde se almacenará el número de elementos del array recibido.
 * @param V_value2 Array donde se almacenarán los doubles recibidos.
 * @param value3 Puntero a la estructura Coord donde se almacenarán las coordenadas recibidas.
 * @return int Código de respuesta del servidor o -1/-2 en caso de error.
 */
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
    // Comprobar si el servidor indica que la clave no existe.
    if (answer.answer == -1)
    {
        return -1;
    }

    // Copiar los datos recibidos en las variables proporcionadas.
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

/**
 * @brief Modifica el valor asociado a una clave.
 *
 * Similar a set_value, esta función valida los parámetros de entrada y prepara una petición
 * para modificar el valor existente asociado a la clave 'key'. Se copian en el mensaje
 * la cadena, el número y contenido del array de doubles y las coordenadas. Tras conectar y enviar
 * la petición, se espera la respuesta del servidor y se devuelve el código de respuesta.
 *
 * @param key Clave identificadora.
 * @param value1 Cadena de caracteres asociada.
 * @param N_value2 Número de elementos del array.
 * @param V_value2 Puntero al array de doubles.
 * @param value3 Estructura Coord con las nuevas coordenadas.
 * @return int Código de respuesta del servidor o -1/-2 en caso de error.
 */
int modify_value(int key,char* value1, int N_value2, double* V_value2,
                 struct Coord value3)
{
    // Validación de parámetros.
    if (strlen(value1) > 255 || N_value2 < 1 || N_value2 > 32) return -1;

    request msg = {0};
    msg.type = 4; //MODIFY_VALUE
    msg.key = key;
    msg.N_value_2 = N_value2;
    msg.value_3 = value3;
    strncpy(msg.value_1, value1, 255);
    memcpy(msg.value_2, V_value2, N_value2 * sizeof(double));

    //Conexión y envio de la petición
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
/**
 * @brief Elimina la clave y su valor asociado en el servidor.
 *
 * Esta función prepara una petición de tipo 3, asigna la clave a eliminar
 * y envía la petición al servidor. Tras recibir la respuesta, devuelve el código resultante.
 *
 * @param key Clave identificadora a eliminar.
 * @return int Código de respuesta del servidor o -2 en caso de error.
 */
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

/**
 * @brief Comprueba la existencia de una clave en el servidor.
 *
 * Esta función prepara una petición de tipo 6 (EXIST) asignándole la clave a comprobar.
 * Se conecta al servidor, envía la petición y espera la respuesta, devolviendo el código
 * que indica si la clave existe o no.
 *
 * @param key Clave identificadora a comprobar.
 * @return int Código de respuesta del servidor o -2 en caso de error.
 */
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
