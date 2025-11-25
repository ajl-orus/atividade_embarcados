#include "radar.h"

// Variáveis globais
system_stats_t global_stats = {0};
struct k_sem sensor_sem;
struct k_mutex stats_mutex;

void radar_system_init(void)
{
    // Inicializa semáforos e mutexes
    k_sem_init(&sensor_sem, 1, 1);
    k_mutex_init(&stats_mutex);
    
    // Reseta estatísticas
    reset_system_stats();
    
    // Publica status inicial
    system_status_t status = SYSTEM_READY;
    zbus_chan_pub(&system_status_chan, &status, K_MSEC(100));
    
    RADAR_SUCCESS("Sistema radar inicializado com sucesso");
}

void radar_print_configuration(void)
{
    RADAR_INFO("=== CONFIGURACAO DO RADAR ===");
    RADAR_INFO("Distancia entre sensores: %d mm", SENSOR_DISTANCE_MM);
    RADAR_INFO("Limite veiculos leves: %d km/h", SPEED_LIMIT_LIGHT);
    RADAR_INFO("Limite veiculos pesados: %d km/h", SPEED_LIMIT_HEAVY);
    RADAR_INFO("Limite de alerta: %d%%", WARNING_THRESHOLD);
    RADAR_INFO("Taxa de falha da camera: %d%%", CAMERA_FAILURE_RATE);
    RADAR_INFO("Tempo de debounce: %d ms", DEBOUNCE_TIME_MS);
    RADAR_INFO("Fator de calibracao: %.2f", SPEED_CALIBRATION_FACTOR);
    RADAR_INFO("Classificacao por: %s", 
               CLASSIFICATION_BY_AXLE_COUNT ? "CONTAGEM DE EIXOS" : "TEMPO ENTRE EIXOS");
    RADAR_INFO("Validacao rigorosa: %s", PLATE_VALIDATION_STRICT ? "SIM" : "NAO");
    RADAR_INFO("Modo debug: %s", RADAR_DEBUG ? "ATIVADO" : "DESATIVADO");
    RADAR_INFO("Simulacao: %s", SENSOR_SIMULATION ? "ATIVADA" : "DESATIVADA");
}

void update_system_stats(vehicle_type_t type, bool infringement, bool camera_fail)
{
    k_mutex_lock(&stats_mutex, K_FOREVER);
    
    global_stats.total_vehicles++;
    
    if (type == VEHICLE_LIGHT) {
        global_stats.light_vehicles++;
    } else if (type == VEHICLE_HEAVY) {
        global_stats.heavy_vehicles++;
    }
    
    if (infringement) {
        global_stats.infringements++;
    }
    
    if (camera_fail) {
        global_stats.camera_failures++;
    }
    
    // Publica estatísticas atualizadas
    zbus_chan_pub(&system_stats_chan, &global_stats, K_NO_WAIT);
    
    k_mutex_unlock(&stats_mutex);
}

void reset_system_stats(void)
{
    k_mutex_lock(&stats_mutex, K_FOREVER);
    memset(&global_stats, 0, sizeof(system_stats_t));
    k_mutex_unlock(&stats_mutex);
}

uint32_t get_current_timestamp(void)
{
    return k_uptime_get_32();
}

float apply_calibration_factor(float speed)
{
    return speed * SPEED_CALIBRATION_FACTOR;
}

void handle_system_error(const char *error_message)
{
    RADAR_ERR("Erro do sistema: %s", error_message);
    
    k_mutex_lock(&stats_mutex, K_FOREVER);
    global_stats.system_errors++;
    k_mutex_unlock(&stats_mutex);
    
    // Publica status de erro
    system_status_t status = SYSTEM_ERROR;
    zbus_chan_pub(&system_status_chan, &status, K_MSEC(100));
}

void recover_from_error(void)
{
    RADAR_WARN("Tentando recuperar do erro...");
    
    // Publica status de calibração
    system_status_t status = SYSTEM_CALIBRATING;
    zbus_chan_pub(&system_status_chan, &status, K_MSEC(100));
    
    k_sleep(K_SECONDS(2));
    
    // Volta para ready
    status = SYSTEM_READY;
    zbus_chan_pub(&system_status_chan, &status, K_MSEC(100));
    
    RADAR_SUCCESS("Sistema recuperado com sucesso");
}