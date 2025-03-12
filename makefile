compiler = gcc
CFLAGS  = -Wall -g -Wextra -fPIC -lsqlite3 $(INCLUDES)
INCLUDES = -I src/
#LDFLAGS = -L. -lclaves

#SERVIDOR
SERVER_SRCS = src/server-side/servidor-mq.c src/server-side/claves.c src/server-side/treat_sql.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_BIN  = servidor


#CLIENTE-libclaves
CLIENT_LIB_SRCS = src/proxy-mq.c
CLIENT_LIB_OBJS = $(CLIENT_LIB_SRCS:.c=.o)
LIB_NAME = src/client-side/libclaves.so

#CLIENTE CLIENTE
CLIENT1_APP_SRCS = src/client-side/app-cliente.c
CLIENT1_APP_OBJS = $(CLIENT1_APP_SRCS:.c=.o)
CLIENT1_BIN = app-cliente

CLIENT2_APP_SRCS =src/client-side/app-cliente2.c
CLIENT2_APP_OBJS = $(CLIENT2_APP_SRCS:.c=.o)
CLIENT2_BIN = app-cliente2

CLIENT3_APP_SRCS =src/client-side/app-cliente3.c
CLIENT3_APP_OBJS = $(CLIENT3_APP_SRCS:.c=.o)
CLIENT3_BIN = app-cliente3

CLIENT4_APP_SRCS =src/client-side/prueba-cliente.c
CLIENT4_APP_OBJS = $(CLIENT4_APP_SRCS:.c=.o)
CLIENT4_BIN = prueba-cliente



all: $(SERVER_BIN) $(LIB_NAME) $(CLIENT1_BIN) $(CLIENT2_BIN) $(CLIENT3_BIN) $(CLIENT4_BIN)


#COMPILACION SERVER
$(SERVER_BIN): $(SERVER_OBJS)
	$(compiler) -o $@ $(SERVER_OBJS) $(CFLAGS)

#COMPILACION DE LA LIBRERIA
$(LIB_NAME): $(CLIENT_LIB_OBJS)
	$(compiler) -shared -o $@ $(CLIENT_LIB_OBJS)

#CLIENTE
$(CLIENT1_BIN): $(CLIENT1_APP_OBJS) $(LIB_NAME)
	$(compiler) -o $@ $(CLIENT1_APP_OBJS) $(LIB_NAME)

$(CLIENT2_BIN): $(CLIENT2_APP_OBJS) $(LIB_NAME)
	$(compiler) -o $@ $(CLIENT2_APP_OBJS) $(LIB_NAME)

$(CLIENT3_BIN): $(CLIENT3_APP_OBJS) $(LIB_NAME)
	$(compiler) -o $@ $(CLIENT3_APP_OBJS) $(LIB_NAME)

$(CLIENT4_BIN): $(CLIENT4_APP_OBJS) $(LIB_NAME)
	$(compiler) -o $@ $(CLIENT4_APP_OBJS) $(LIB_NAME)

%.o: %.c
	$(compiler) $(CFLAGS) -c $< -o $@

#LIMPIA; ME LO CARGO TODO
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_LIB_OBJS) $(CLIENT_APP_OBJS) \
	      $(SERVER_BIN) $(CLIENT1_BIN) $(CLIENT2_BIN) $(CLIENT3_BIN) $(CLIENT4_BIN) $(LIB_NAME)
	rm -r database.db
