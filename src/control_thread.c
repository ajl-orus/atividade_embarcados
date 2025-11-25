#include "radar.h"

// Fila para dados do veículo
K_MSGQ_DEFINE(vehicle_data_queue, sizeof(vehicle_data_t), 10, 4);

void control_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    vehicle_data_t vehicle_data;
    camera_data_t camera_data;
    bool camera_trigger = true;

    while (1) {
        // Aguarda dados do veículo
        if (k_msgq_get(&vehicle_data_queue, &vehicle_data, K_FOREVER) == 0) {
            
            // Calcula velocidade
            calculate_speed(&vehicle_data);
            
            // Verifica status da velocidade
            speed_status_t status = check_speed_status(vehicle_data.speed_kmh, 
                                                     vehicle_data.type);
            
            // Se for infração, aciona a câmera
            if (status == SPEED_INFRACTION) {
                printf(COLOR_RED "INFRACAO DETECTADA! " COLOR_NORMAL);
                printf("Veiculo: %s, Velocidade: %.1f km/h\n",
                       vehicle_data.type == VEHICLE_LIGHT ? "Leve" : "Pesado",
                       vehicle_data.speed_kmh);
                
                // Dispara câmera via ZBUS
                zbus_chan_pub(&camera_trigger_chan, &camera_trigger, K_MSEC(250));
                
                // Aguarda resultado da câmera
                if (zbus_chan_read(&camera_result_chan, &camera_data, K_MSEC(1000)) == 0) {
                    if (camera_data.captured) {
                        printf("Placa: %s - %s\n", 
                               camera_data.plate,
                               camera_data.valid ? "VALIDA" : "INVALIDA");
                    }
                }
            }
        }
        
        k_sleep(K_MSEC(10));
    }
}

K_THREAD_DEFINE(control_thread_id, 2048, control_thread, NULL, NULL, NULL, 5, 0, 0);