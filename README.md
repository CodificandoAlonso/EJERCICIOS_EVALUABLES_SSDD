# Ejercicios_evaluables_SSDD
Ejercicios evaluables realizado por Ángela Elena Serrano Casas y Héctor Álvarez Marcos. Esta aplicación es un sistema
distribuido, emulando una estructura Cliente-Servidor, en la que se otorga una "api" al cliente para que pueda realizar
operaciones sobre un sistema de guardado de datos. En nuestro caso el sistema ha sido una base de datos SQL, gestionada
por un servidor concurrente en el lenguaje de programación C. Este proyecto tiene 3 ramas, 2 de ellas implementando cada
una un método de comunicación diferente, una mediante colas de mensajes POSIX y otra mediante sockets. La tercera rama,
implementa el mismo sistema de "Api" pero usando "RPC",que "TOCARA COMPLETAR PARA PARTE 3"

Las distintas ramas están nombradas de esta forma:
- RAMA-COLAS-POSIX: Implementa colas de mensajes POSIX.
- RAMA-SOCKETS: Implementa sockets.
- RAMA-RPC: Implementa RPC.

Para ejecutar el programa, se debe clonar el repositorio y situarse en la rama deseada. Una vez situado en la rama, se 
tiene un readme con las instrucciones para ejecutar el programa.
