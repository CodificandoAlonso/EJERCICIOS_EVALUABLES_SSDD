# ===================================
# CONFIGURACIÓN GENERAL
# ===================================
CC         = gcc
CFLAGS     = -Wall -Wextra -I src/structs -I src/common -I/usr/include/tirpc
CLAVES_X   = src/common/claves_rpc.x
RPCGEN     = rpcgen -C

# ===================================
# REGLA GENÉRICA: .c → .o
# ===================================
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ===================================
# GENERACIÓN AUTOMÁTICA DE ARCHIVOS RPC
# ===================================
src/common/claves_rpc.h: $(CLAVES_X)
	cd src/common && $(RPCGEN) -h -o claves_rpc.h claves_rpc.x

src/common/claves_rpc_clnt.c: $(CLAVES_X)
	cd src/common && $(RPCGEN) -l -o claves_rpc_clnt.c claves_rpc.x

src/common/claves_rpc_xdr.c: $(CLAVES_X)
	cd src/common && $(RPCGEN) -c -o claves_rpc_xdr.c claves_rpc.x

src/common/claves_rpc_svc.c: $(CLAVES_X)
	cd src/common && $(RPCGEN) -m -o claves_rpc_svc.c claves_rpc.x

# ===================================
# DEPENDENCIAS PARA CLIENTE RPC
# ===================================
# proxy-rpc.o necesita los stubs de cliente y el header RPC
src/client-side/proxy-rpc.o: src/common/claves_rpc.h \
                             src/common/claves_rpc_clnt.c \
                             src/common/claves_rpc_xdr.c

# ===================================
# DEPENDENCIAS PARA SERVIDOR RPC
# ===================================
# servidor-rpc.o necesita el header RPC
src/server-side/servidor-rpc.o: src/common/claves_rpc.h

# ===================================
# CLIENTE RPC
# ===================================
CLIENT_SRCS = \
	src/client-side/proxy-rpc.c \
	src/common/claves_rpc_clnt.c \
	src/common/claves_rpc_xdr.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

app-cliente-rpc: $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CFLAGS) -ltirpc

# ===================================
# SERVIDOR RPC
# ===================================
SERVER_SRCS = \
	src/server-side/servidor-rpc.c \
	src/server-side/claves.c \
	src/server-side/treat_sql.c \
	src/common/claves_rpc_svc.c \
	src/common/claves_rpc_xdr.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

servidor-rpc: $(SERVER_OBJS)
	$(CC) -o $@ $(SERVER_OBJS) -lpthread -lsqlite3 -ltirpc

# Directorios
SRCDIR     = src/client-side
COMMON_OBJS = src/common/claves_rpc_clnt.o src/common/claves_rpc_xdr.o
PROXY_OBJ   = src/client-side/proxy-rpc.o

# Todos los tests app-cliente*.c
CLIENT_TESTS = $(notdir $(wildcard $(SRCDIR)/app-cliente*.c))
CLIENT_EXES  = $(patsubst app-cliente%.c,app-cliente%,$(CLIENT_TESTS))

.PHONY: clients

clients: $(CLIENT_EXES)

# patrón para construir cada test
app-cliente%: $(SRCDIR)/app-cliente%.c $(COMMON_OBJS) $(PROXY_OBJ)
	$(CC) $(CFLAGS) \
	  -o $@ $< $(COMMON_OBJS) $(PROXY_OBJ) \
	  -I src/structs -I src/common \
	  -ltirpc -lpthread

# limpia también los tests
clean:
	rm -f $(CLIENT_EXES) bin/*

# ===================================
# META “all”
# ===================================
.PHONY: all
all: servidor-rpc app-cliente-rpc

# ===================================
# LIMPIEZA
# ===================================
.PHONY: clean
clean:
	rm -f src/common/claves_rpc.h \
	      src/common/claves_rpc_clnt.c \
	      src/common/claves_rpc_svc.c \
	      src/common/claves_rpc_xdr.c \
	      src/client-side/*.o \
	      src/server-side/*.o \
	      src/common/*.o \
	      servidor-rpc \
	      app-cliente-rpc
