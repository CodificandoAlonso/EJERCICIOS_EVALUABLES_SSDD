cmake_minimum_required(VERSION 3.10)
project(Ejercicio1_SSDD C)

set(CMAKE_C_STANDARD 11)

# Incluir los directorios donde se encuentran los headers
include_directories(
        ${CMAKE_SOURCE_DIR}/src/structs
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/client-side
        ${CMAKE_SOURCE_DIR}/src/server-side
)

# Executable del servidor: incluye los archivos que definen main y las funciones necesarias
add_executable(servidor
        src/server-side/servidor-rpc.c
        src/server-side/claves.c
        src/server-side/treat_sql.c
        src/common/socket_message.c
        src/server-side/treat_sql.cpp
        src/server-side/treat_sql.c
)
# Enlazar con SQLite3 para resolver las funciones relacionadas
target_link_libraries(servidor sqlite3)

# Executable del cliente: se elige uno de los main disponibles y se incluye el archivo con las implementaciones
add_executable(cliente
        src/client-side/app-cliente-inf.c
        src/client-side/proxy-rpc.c
        src/common/socket_message.c
        src/structs/claves.h
        src/structs/treat_sql.h
)
