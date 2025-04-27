#!/usr/bin/env bash

# ─────────────────────────────────────────────────────────────────────────────
# Colores ANSI
# ─────────────────────────────────────────────────────────────────────────────
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
NC="\033[0m"  # No Color

# ─────────────────────────────────────────────────────────────────────────────
# Limpieza
# ─────────────────────────────────────────────────────────────────────────────
echo -e "${BLUE}→ Realizando make clean para limpiar el directorio...${NC}"
echo -en "${GREEN}"
make clean
echo -e "${NC}"

# ─────────────────────────────────────────────────────────────────────────────
# Compilación
# ─────────────────────────────────────────────────────────────────────────────
echo -e "${BLUE}→ Realizando make para generar ejecutables...${NC}"
echo -en "${GREEN}"
make
echo -e "${NC}"

# ─────────────────────────────────────────────────────────────────────────────
# Ajuste de permisos
# ─────────────────────────────────────────────────────────────────────────────
echo -e "${BLUE}→ Ajustando permisos de ejecución...${NC}"
chmod +x servidor \
         app-cliente \
         app-cliente2 \
         app-cliente3 \
         app-cliente-inf \
         app-cliente-pesao

# ─────────────────────────────────────────────────────────────────────────────
# Mensaje final
# ─────────────────────────────────────────────────────────────────────────────
echo -e "${GREEN}✔ ¡Todo listo!${NC}"
echo -e "${YELLOW}  • Servidor: ./servidor${NC}"
echo -e "${YELLOW}  • Clientes: ./app-cliente, ./app-cliente2, ./app-cliente3, ./app-cliente-inf, ./app-cliente-pesao${NC}"
