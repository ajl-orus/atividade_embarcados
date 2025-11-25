#include "radar.h"

void camera_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    bool trigger;
    camera_data_t camera_data;

    while (1) {
        // Aguarda trigger da câmera
        if (zbus_chan_read(&camera_trigger_chan, &trigger, K_FOREVER) == 0) {
            if (trigger) {
                printf(COLOR_BLUE "Camera: Capturando placa...\n" COLOR_NORMAL);
                
                // Simula processamento
                k_sleep(K_MSEC(500));
                
                // Simula captura da placa (com possibilidade de falha)
                simulate_license_plate(camera_data.plate, &camera_data.valid);
                camera_data.captured = true;
                
                printf(COLOR_BLUE "Camera: Placa %s capturada - %s\n" COLOR_NORMAL,
                       camera_data.plate,
                       camera_data.valid ? "Valida" : "Invalida");
                
                // Publica resultado
                zbus_chan_pub(&camera_result_chan, &camera_data, K_MSEC(250));
            }
        }
    }
}

K_THREAD_DEFINE(camera_thread_id, 2048, camera_thread, NULL, NULL, NULL, 3, 0, 0);