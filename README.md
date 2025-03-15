# Ejercicio1_SSDD
Ejercicio evaluable 1 realizado por Ángela Elena Serrano Casas y Héctor Álvarez Marcos. Esta aplicación es un sistema distribuido a nivel local basado en la implementación sencilla de un Cliente-Servidor. Al cliente se le otorga una API, que le permite realizar operaciones de insercion, busqueda y eliminación sobre una base de datos.
Para ejecutar la aplicación se realiza un make sobre el directorio principal. Esto genera un ejecutable para el servidor y 4 ejecutables para 4 distintos clientes.
Los 3 primeros tienen estructura similar, y el 4es una implementaciónd de un cliente "infinito", es decir, un bucle infinito en el que puedes estar solicitando peticiones directamente al servidor todo el rato hasta que quieras.
Esta práctica se ha dotado de la implementacion de envio por sistema de colas de mensajes POSIX, y utiliza un sistema de almacenamiento de guardado por base de datos sql, implementado gracias a la libreria sqlite3, incluida en C.
