cmake_minimum_required(VERSION 3.10)
project(Ejercicio1_SSDD C)


set(CMAKE_C_STANDARD 99)

#Se necesita que la libreria de hilos esté instalada
find_package(Threads REQUIRED)

# lo mismo con sql, para la gestion de la base de datos
find_package(SQLite3 REQUIRED)

# Si quieres incluir cabeceras de src/ directamente (o subdirectorios)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/server-side)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/client-side)

#Servidor
add_executable(server
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server-side/servidor-mq.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server-side/claves.c
)

#enlazar librerrias con el servidor
target_link_libraries(server
        Threads::Threads
        ${SQLite3_LIBRARIES}
)

#Cliente
add_executable(cliente
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client-side/app-cliente.c
        src/proxy-mq.c  # ajusta si proxy-mq.c está en otro lado
)

#enlazar librerias con el cliente
target_link_libraries(cliente
        Threads::Threads
        ${SQLite3_LIBRARIES}
)
