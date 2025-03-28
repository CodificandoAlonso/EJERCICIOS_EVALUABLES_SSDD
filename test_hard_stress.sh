#!/bin/bash
# Función llamada por la señal SIGINT configurada con trap
cleanup() {
    echo "Interrumpido. Matando procesos..."
    rm -rf /dev/mqueue/client*
    pkill -P $$  # Mata todos los procesos hijos del script
    exit 1
}
# Capturar SIGINT (Ctrl+C)
trap cleanup SIGINT


./app-cliente &
./app-cliente2 &
./app-cliente3 &
./app-cliente &
./app-cliente2 &
./app-cliente3 &
./app-cliente &
./app-cliente2 &
./app-cliente3 &
./app-cliente &
./app-cliente2 &
./app-cliente3 &
./app-cliente &
./app-cliente2 &
./app-cliente3 &




wait

exit 0