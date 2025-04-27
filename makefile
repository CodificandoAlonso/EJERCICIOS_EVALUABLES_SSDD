.DEFAULT_GOAL := all
# ------------------------------------------------------------------------------
# Clean y arranque
# ------------------------------------------------------------------------------
.PHONY: all clean
all: clean servidor libclaves.so clients
USER_NAME := $(shell whoami)


# ------------------------------------------------------------------------------
# 1) Flags y paths
# ------------------------------------------------------------------------------
CC      := gcc
CFLAGS  := -fPIC \
           -I src/structs \
           -I src/common \
           -I/usr/include/tirpc \
           -D_GNU_SOURCE \
           -D_REENTRANT
LDFLAGS := -lpthread -ltirpc -lsqlite3

# ------------------------------------------------------------------------------
# 2) RPCGEN: generar stubs MT-safe (-M)
# ------------------------------------------------------------------------------
CLAVES_X := src/common/claves_rpc.x
RPCGEN   := rpcgen -C -M

src/common/claves_rpc.h: $(CLAVES_X)
	cd src/common && $(RPCGEN) -h -o claves_rpc.h claves_rpc.x

src/common/claves_rpc_clnt.c: $(CLAVES_X)
	cd src/common && $(RPCGEN) -l -o claves_rpc_clnt.c claves_rpc.x

src/common/claves_rpc_xdr.c: $(CLAVES_X)
	cd src/common && $(RPCGEN) -c -o claves_rpc_xdr.c claves_rpc.x

src/common/claves_rpc_svc.c: $(CLAVES_X)
	cd src/common && $(RPCGEN) -m -o claves_rpc_svc.c claves_rpc.x

# Asegurar que el header existe antes de compilar stubs y proxy

RPC_OBJS = \
  src/common/claves_rpc_clnt.o \
  src/common/claves_rpc_xdr.o \
  src/common/claves_rpc_svc.o \
  src/client-side/proxy-rpc.o \
  src/server-side/servidor-rpc.o


$(RPC_OBJS): src/common/claves_rpc.h

# ------------------------------------------------------------------------------
# 3) Biblioteca compartida libclaves.so (wrappers + stubs)
# ------------------------------------------------------------------------------
PROXY_SRCS := \
  src/client-side/proxy-rpc.c \
  src/common/claves_rpc_clnt.c \
  src/common/claves_rpc_xdr.c
PROXY_OBJS := $(PROXY_SRCS:.c=.o)

libclaves.so: $(PROXY_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(PROXY_OBJS) $(LDFLAGS)

# ------------------------------------------------------------------------------
# 4) Servidor RPC (lógica de negocio + stubs de servicio)
# ------------------------------------------------------------------------------
SERVER_SRCS := \
  src/server-side/servidor-rpc.c \
  src/server-side/claves.c \
  src/server-side/treat_sql.c \
  src/server-side/free_xdr.c \
  src/common/claves_rpc_svc.c \
  src/common/claves_rpc_xdr.c
SERVER_OBJS := $(SERVER_SRCS:.c=.o)

servidor: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) $(LDFLAGS)

# ------------------------------------------------------------------------------
# 5) Clientes RPC (cada uno con su main)
# ------------------------------------------------------------------------------
CLIENT_SRCS := $(wildcard src/client-side/app-cliente*.c)
CLIENT_BINS := $(patsubst src/client-side/%.c,%,$(CLIENT_SRCS))

.PHONY: clients
clients: $(CLIENT_BINS)

# Regla genérica: src/client-side/%.c → binario (solo enlace contra libclaves.so)
%: src/client-side/%.c | libclaves.so
	$(CC) $(CFLAGS) -o $@ $< \
	  -L. -Wl,-rpath,'$$ORIGIN' \
	  -lclaves $(LDFLAGS)



clean:
	rm -f src/common/*.o src/client-side/*.o src/server-side/*.o \
	      servidor $(CLIENT_BINS) libclaves.so src/common/*.c src/common/*.h
	rm -rf /tmp/database-$(USER_NAME).db