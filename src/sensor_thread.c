#include "radar.h"

static const struct device *gpio_dev;
static struct gpio_callback sensor1_cb, sensor2_cb;

// Variáveis para máquina de estados
static volatile uint32_t sensor1_time = 0;
static volatile uint32_t sensor2_time = 0;
static volatile uint8_t axle_count = 0;
static volatile uint32_t last_axle_time = 0;
static volatile bool sensor1_triggered = false;

// Interrupção do Sensor 1 (contagem de eixos)
void sensor1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    uint32_t current_time = k_uptime_get_32();
    
    // Filtro de bouncing (mínimo 50ms entre pulsos)
    if ((current_time - last_axle_time) > 50) {
        axle_count++;
        last_axle_time = current_time;
    }
    
    sensor1_triggered = true;
    sensor1_time = current_time;
}

// Interrupção do Sensor 2 (cálculo de velocidade)
void sensor2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    if (sensor1_triggered) {
        sensor2_time = k_uptime_get_32();
        
        // Prepara dados do veículo
        vehicle_data_t vehicle_data = {
            .timestamp = k_uptime_get_32(),
            .time_between_sensors = sensor2_time - sensor1_time,
            .axle_count = axle_count
        };
        
        // Classifica veículo baseado no número de eixos
        if (axle_count >= 3) {
            vehicle_data.type = VEHICLE_HEAVY;
        } else if (axle_count == 2) {
            vehicle_data.type = VEHICLE_LIGHT;
        } else {
            vehicle_data.type = VEHICLE_UNKNOWN;
        }
        
        // Envia dados para a thread de controle
        k_msgq_put(&vehicle_data_queue, &vehicle_data, K_NO_WAIT);
        
        // Reseta contadores
        axle_count = 0;
        sensor1_triggered = false;
    }
}

void sensor_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    gpio_dev = device_get_binding("GPIO_0");
    if (!gpio_dev) {
        printf("Erro: GPIO device not found\n");
        return;
    }

    // Configura Sensor 1 (contagem de eixos)
    gpio_pin_configure(gpio_dev, SENSOR_1_PIN, 
                      GPIO_INPUT | GPIO_PULL_UP | GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&sensor1_cb, sensor1_handler, BIT(SENSOR_1_PIN));
    gpio_add_callback(gpio_dev, &sensor1_cb);
    gpio_pin_interrupt_configure(gpio_dev, SENSOR_1_PIN, GPIO_INT_EDGE_FALLING);

    // Configura Sensor 2 (velocidade)
    gpio_pin_configure(gpio_dev, SENSOR_2_PIN, 
                      GPIO_INPUT | GPIO_PULL_UP | GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&sensor2_cb, sensor2_handler, BIT(SENSOR_2_PIN));
    gpio_add_callback(gpio_dev, &sensor2_cb);
    gpio_pin_interrupt_configure(gpio_dev, SENSOR_2_PIN, GPIO_INT_EDGE_FALLING);

    printf("Sensores inicializados - GPIO %d (eixos) e GPIO %d (velocidade)\n", 
           SENSOR_1_PIN, SENSOR_2_PIN);

    while (1) {
        k_sleep(K_SECONDS(1));
    }
}

K_THREAD_DEFINE(sensor_thread_id, 2048, sensor_thread, NULL, NULL, NULL, 6, 0, 0);