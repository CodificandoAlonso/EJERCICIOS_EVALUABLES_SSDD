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


/**
 * @brief Determina si la máquina es big endian o little endian.
 *
 * Esta función verifica el orden de bytes (endianess) de la máquina.
 * Si el primer byte de un número entero de 1 es 1, la máquina es little endian,
 * de lo contrario, es big endian.
 *
 * @return 1 si es big endian, 0 si es little endian.
 */
int isBigEndian(void) {
    unsigned int num = 1;
    // Si el primer byte (dirección más baja) es 1, es little endian
    if (*(char *)&num == 1)
        return 0;  // little endian
    else
        return 1;  // big endian
}

/**
 * @brief Intercambia el orden de bytes de un número double.
 *
 * Esta función cambia el orden de bytes de un valor tipo double en función
 * del endianness de la máquina. Útil cuando el orden de los bytes es
 * diferente al que utiliza la máquina local.
 *
 * @param value Valor double que se desea modificar.
 * @return double Valor con los bytes intercambiados.
 */
double swap_endian(double value) {
    uint8_t *temp = (uint8_t *)&value;  // Tratamos el valor como un array de bytes
    char new_double[8];  // Creamos un nuevo array para almacenar los bytes en orden invertido

    // Revertimos el orden de los bytes
    new_double[0] = *(temp + 7);
    new_double[1] = *(temp + 6);
    new_double[2] = *(temp + 5);
    new_double[3] = *(temp + 4);
    new_double[4] = *(temp + 3);
    new_double[5] = *(temp + 2);
    new_double[6] = *(temp + 1);
    new_double[7] = *(temp + 0);

    // Convertimos el array de bytes de nuevo a un número double
    double ret_val = *(double *)&new_double;
    return ret_val;
}

/**
 * @brief Convierte un valor double de host a formato de red (big endian).
 *
 * Si la máquina es little endian, la función intercambia el orden de los bytes
 * del número double. En una máquina big endian, no realiza ninguna modificación.
 *
 * @param value Valor double que se desea convertir.
 * @return double Valor convertido al formato de red (big endian).
 */
double host_to_net_double(double value)
{
    if (isBigEndian() == 0) //Little Endian
    {
       return swap_endian(value);
    }
    return value;
}

/**
 * @brief Imprime un número double en formato hexadecimal.
 *
 * Esta función toma un valor double y lo imprime byte a byte en formato hexadecimal,
 * Esta nos ha sido de gran utilidad para depurar o verificar el contenido binario de los valores.
 *
 * @param value Valor double que se desea imprimir.
 */
void print_double_hex(double value) {
    uint8_t bytes[8];
    memcpy(bytes, &value, sizeof(bytes));
    for (int i = 0; i < 8; i++) {
        printf("%02X ", bytes[i]);
    }
    printf("\n");
}

/**
 * @brief Convierte un valor double desde formato de red (big endian) a host.
 *
 * Si la máquina es little endian, la función intercambia el orden de los bytes
 * del número double. En una máquina big endian, no realiza ninguna modificación.
 *
 * @param value Valor double en formato de red (big endian) que se desea convertir.
 * @return double Valor convertido a formato de host.
 */
double net_to_host_double(double value)
{
    if (isBigEndian() == 0) //Little Endian
    {
        return swap_endian(value);
    }
    return value;
}


/**
 * @brief Recibe un paquete de datos a través de un socket.
 *
 * Esta función lee los datos del socket en bloques de tamaño 'size' hasta que todo
 * el paquete se ha recibido correctamente. Si ocurre un error en la lectura, se informa.
 *
 * @param socket Descriptor del socket desde el que se va a leer.
 * @param message Puntero al búfer donde se almacenarán los datos recibidos.
 * @param size Tamaño total del paquete a recibir.
 * @return int 0 si la recepción fue exitosa, -1 si hubo un error.
 */
int receive_package(int socket, void *message, int size)
{
    int r = 0;
    int left = size;
    void *buffer = message;  // Buffer para almacenar los datos recibidos

    // Recibimos los datos en partes hasta completar el tamaño total
    while (left > 0) {
        // Leemos del socket y almacenamos en el buffer
        r = read(socket, buffer, left);
        if (r <= 0) {
            perror("Error reading from socket");
            return -1;
        }
        left -= r;  // Restamos la cantidad de bytes leídos
        buffer += r;  // Movemos el puntero del buffer
    }
    return 0;  // Retorno exitoso
}


/**
 * @brief Recibe un mensaje completo desde un socket y lo deserializa en una estructura 'request'.
 *
 * Esta función maneja la recepción de un mensaje completo, que incluye varios campos de datos como
 * el tipo de mensaje, la clave, los valores asociados, etc. Tras la recepcion individual de los parametros,
 * se deserializan en el formato esperado en la estrucutra. Los nuevos datos se almacenan en la estructura 'message'.
 *
 * @param socket Descriptor del socket desde el que se va a leer el mensaje.
 * @param message Puntero a la estructura 'request' donde se almacenarán los datos recibidos.
 * @return int 0 si la recepción fue exitosa, -1 si hubo un error.
 */
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
    //Bucle para almacenar todos los valores del vector de longitud variable
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


/**
 * @brief Envía un paquete de datos a través de un socket.
 *
 * Esta función envía los datos del paquete al socket en bloques hasta que el paquete completo
 * haya sido enviado correctamente. Si ocurre un error en la escritura, se representa con el codigo de error -1.
 *
 * @param socket Descriptor del socket al que se enviarán los datos.
 * @param message Puntero al paquete de datos a enviar.
 * @param size Tamaño total del paquete a enviar.
 * @return int 0 si el envío fue exitoso, -1 si hubo un error.
 */
int send_package(int socket, void *message, int size)
{
    int written = 0;
    int left = size;

    // Enviamos los datos en partes hasta completar el tamaño total
    while (left > 0) {
        written = write(socket, message, left);  // Escribimos en el socket
        if (written <= 0) {  // Si ocurre un error o no se escribió nada
            perror("Error reading from socket");
            return -1;
        }
        left -= written;  // Restamos la cantidad de bytes escritos
        message += written;  // Movemos el puntero del mensaje
    }
    return 0;  // Retorno exitoso
}


/**
 * @brief Envía un mensaje a través de un socket.
 *
 * Esta función serializa los datos de la estructura 'request' y los envía a través del socket,
 * por medio de la funcion definida previamente "send_package()"
 * Convierte los valores al formato de red (big endian) antes de enviarlos.
 * Para la serializacion de los doubles se hace uso de la funcion definida en la practica "host_to_net_double"
 *
 * @param socket Descriptor del socket al que se enviará el mensaje.
 * @param answer Puntero a la estructura 'request' que contiene el mensaje a enviar.
 * @return int 0 si el envío fue exitoso, -1 si hubo un error.
 */
int send_message(int socket, request *answer) {
    //Convierto a formato de red
    __uint32_t type = htonl(answer->type);
    //Envio
    send_package(socket, &type, sizeof(int));

    // Convierto y envio cada uno de los argumentos del mensaje a enviar

    __uint32_t key = htonl(answer->key);
    send_package(socket, &key, sizeof(int));
    ssize_t len_v1 = strlen(answer->value_1);
    send_package(socket, &len_v1, sizeof(long));
    send_package(socket, &answer->value_1, len_v1);
    __uint32_t N_value_2 = htonl(answer->N_value_2);
    send_package(socket, &N_value_2, sizeof(int));
    //Bucle para la serialización del vector de longitud variable
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
