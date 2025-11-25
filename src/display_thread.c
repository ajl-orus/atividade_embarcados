#include "radar.h"

static const struct device *display_dev;

void update_display(float speed, vehicle_type_t type, speed_status_t status)
{
    const char *color;
    const char *status_text;
    int speed_limit;
    
    // Define cor e texto baseado no status
    switch (status) {
        case SPEED_NORMAL:
            color = COLOR_GREEN;
            status_text = "NORMAL";
            break;
        case SPEED_WARNING:
            color = COLOR_YELLOW;
            status_text = "ALERTA";
            break;
        case SPEED_INFRACTION:
            color = COLOR_RED;
            status_text = "INFRACAO";
            break;
        default:
            color = COLOR_NORMAL;
            status_text = "DESCONHECIDO";
    }
    
    // Define limite de velocidade
    speed_limit = (type == VEHICLE_LIGHT) ? SPEED_LIMIT_LIGHT : SPEED_LIMIT_HEAVY;
    
    // Formata saída com cores
    printf("\n" COLOR_BLUE "=== RADAR ELETRONICO ===\n" COLOR_NORMAL);
    printf("Veiculo: %s\n", type == VEHICLE_LIGHT ? "LEVE" : "PESADO");
    printf("Velocidade: " COLOR_BLUE "%.1f" COLOR_NORMAL " km/h\n", speed);
    printf("Limite: %d km/h\n", speed_limit);
    printf("Status: %s%s%s\n\n", color, status_text, COLOR_NORMAL);
}

void display_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    display_dev = device_get_binding("DISPLAY_0");
    if (!display_dev) {
        printf("Display dummy não encontrado, usando console apenas\n");
    }

    vehicle_data_t vehicle_data;
    speed_status_t last_status = SPEED_NORMAL;
    
    while (1) {
        // Verifica se há novos dados (não bloqueante)
        if (k_msgq_get(&vehicle_data_queue, &vehicle_data, K_MSEC(100)) == 0) {
            speed_status_t current_status = check_speed_status(vehicle_data.speed_kmh, 
                                                             vehicle_data.type);
            
            // Atualiza display apenas se houve mudança
            if (current_status != last_status || vehicle_data.speed_kmh > 0) {
                update_display(vehicle_data.speed_kmh, vehicle_data.type, current_status);
                last_status = current_status;
            }
        }
        
        k_sleep(K_MSEC(500));
    }
}

K_THREAD_DEFINE(display_thread_id, 2048, display_thread, NULL, NULL, NULL, 4, 0, 0);