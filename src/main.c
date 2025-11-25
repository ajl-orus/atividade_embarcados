#include "radar.h"

void main(void)
{
    printf(COLOR_BLUE "\n=== Sistema Radar Eletronico Iniciado ===\n" COLOR_NORMAL);
    printf("Distancia entre sensores: %d mm\n", SENSOR_DISTANCE_MM);
    printf("Limite veiculos leves: %d km/h\n", SPEED_LIMIT_LIGHT);
    printf("Limite veiculos pesados: %d km/h\n", SPEED_LIMIT_HEAVY);
    printf("Limite de alerta: %d%%\n", WARNING_THRESHOLD);
    printf("Taxa de falha da camera: %d%%\n\n", CAMERA_FAILURE_RATE);
    
    // Threads são iniciadas automaticamente pelo Zephyr
    while (1) {
        k_sleep(K_SECONDS(10));
    }
}