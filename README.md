# Ejercicio1_SSDD
Ejercicio evaluable 1 realizado por Ángela Elena Serrano Casas y Héctor Álvarez Marcos. Esta aplicación es un sistema
distribuido a nivel local basado en la implementación sencilla de un Cliente-Servidor. Al cliente se le otorga una API,
que le permite realizar operaciones de insercion, busqueda y eliminación sobre una base de datos.
Para ejecutar la aplicación se realiza un make sobre el directorio principal. Esto genera un ejecutable para el servidor
y 4 ejecutables para 4 distintos clientes.
Los 3 primeros tienen estructura similar, y el 4 es una implementación de un cliente "infinito", es decir, un bucle
infinito en el que puedes estar solicitando peticiones directamente al servidor todo el rato hasta que quieras.
Esta práctica se ha dotado de la implementacion de envio por sistema sockets TCP. Para su uso, deberá definirse en
variables de entorno la IP a utilizar para el intercambio de mensajes mediante la variable env IP_TUPLAS. En la terminal
donde se ejecute el cliente deberá ademas indicarse en la dirección PORT_TUPLAS el puerto a utilizar por el servidor, y
en la terminal del servidor ese mismo puerto deberá ir especificado como parámetro del ejecutable servidor.
Utiliza también un sistema de almacenamiento de guardado por base de datos sql, implementado gracias a la libreria
sqlite3, incluida en C.
