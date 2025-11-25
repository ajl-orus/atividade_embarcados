#ifndef RADAR_H
#define RADAR_H

#include <zephyr.h>
#include <drivers/gpio.h>
#include <drivers/display.h>
#include <zbus.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

// Configurações via Kconfig
#define SENSOR_DISTANCE_MM          CONFIG_RADAR_SENSOR_DISTANCE_MM
#define SPEED_LIMIT_LIGHT           CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH
#define SPEED_LIMIT_HEAVY           CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH
#define WARNING_THRESHOLD           CONFIG_RADAR_WARNING_THRESHOLD_PERCENT
#define CAMERA_FAILURE_RATE         CONFIG_RADAR_CAMERA_FAILURE_RATE_PERCENT
#define DEBOUNCE_TIME_MS            CONFIG_RADAR_DEBOUNCE_TIME_MS
#define DISPLAY_UPDATE_INTERVAL_MS  CONFIG_RADAR_DISPLAY_UPDATE_INTERVAL_MS
#define CAMERA_PROCESSING_TIME_MS   CONFIG_RADAR_CAMERA_PROCESSING_TIME_MS
#define MAX_VEHICLE_QUEUE_SIZE      CONFIG_RADAR_MAX_VEHICLE_QUEUE_SIZE
#define AXLE_TIMEOUT_MS             CONFIG_RADAR_AXLE_TIMEOUT_MS
#define PLATE_VALIDATION_STRICT     CONFIG_RADAR_PLATE_VALIDATION_STRICT
#define SPEED_CALIBRATION_FACTOR    (CONFIG_RADAR_SPEED_CALIBRATION_FACTOR / 100.0f)

// Configurações de classificação
#if defined(CONFIG_RADAR_CLASSIFICATION_AXLE_COUNT)
#define CLASSIFICATION_BY_AXLE_COUNT 1
#define CLASSIFICATION_BY_TIME 0
#elif defined(CONFIG_RADAR_CLASSIFICATION_TIME_BASED)
#define CLASSIFICATION_BY_AXLE_COUNT 0
#define CLASSIFICATION_BY_TIME 1
#else
#define CLASSIFICATION_BY_AXLE_COUNT 1
#define CLASSIFICATION_BY_TIME 0
#endif

// Configurações de debug
#if defined(CONFIG_RADAR_DEBUG_ENABLED)
#define RADAR_DEBUG 1
#else
#define RADAR_DEBUG 0
#endif

#if defined(CONFIG_RADAR_SENSOR_SIMULATION)
#define SENSOR_SIMULATION 1
#define SIMULATION_INTERVAL_MS CONFIG_RADAR_SIMULATION_INTERVAL_MS
#else
#define SENSOR_SIMULATION 0
#endif

// Níveis de log
#if CONFIG_RADAR_LOG_LEVEL >= 4
#define RADAR_LOG_DEBUG 1
#else
#define RADAR_LOG_DEBUG 0
#endif

#if CONFIG_RADAR_LOG_LEVEL >= 3
#define RADAR_LOG_INFO 1
#else
#define RADAR_LOG_INFO 0
#endif

#if CONFIG_RADAR_LOG_LEVEL >= 2
#define RADAR_LOG_WARN 1
#else
#define RADAR_LOG_WARN 0
#endif

#if CONFIG_RADAR_LOG_LEVEL >= 1
#define RADAR_LOG_ERR 1
#else
#define RADAR_LOG_ERR 0
#endif

// GPIO dos sensores
#define SENSOR_1_PIN         5
#define SENSOR_2_PIN         6

// Constantes de conversão
#define MS_TO_HOURS          3.6e6f    // 1 hora = 3.600.000 ms
#define MM_TO_KM             1e-6f     // 1 mm = 0.000001 km
#define MIN_TIME_BETWEEN_SENSORS 10    // Tempo mínimo válido entre sensores (ms)

// Cores ANSI
#define COLOR_NORMAL         "\033[0m"
#define COLOR_GREEN          "\033[32m"
#define COLOR_YELLOW         "\033[33m"
#define COLOR_RED            "\033[31m"
#define COLOR_BLUE           "\033[34m"
#define COLOR_CYAN           "\033[36m"
#define COLOR_MAGENTA        "\033[35m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_BLUE    "\033[94m"

// Macros para logging
#define RADAR_DBG(fmt, ...) \
    do { \
        if (RADAR_DEBUG && RADAR_LOG_DEBUG) { \
            printk(COLOR_CYAN "[RADAR-DBG] %s:%d: " fmt COLOR_NORMAL "\n", \
                   __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define RADAR_INFO(fmt, ...) \
    do { \
        if (RADAR_LOG_INFO) { \
            printk(COLOR_BLUE "[RADAR-INFO] " fmt COLOR_NORMAL "\n", ##__VA_ARGS__); \
        } \
    } while (0)

#define RADAR_WARN(fmt, ...) \
    do { \
        if (RADAR_LOG_WARN) { \
            printk(COLOR_YELLOW "[RADAR-WARN] " fmt COLOR_NORMAL "\n", ##__VA_ARGS__); \
        } \
    } while (0)

#define RADAR_ERR(fmt, ...) \
    do { \
        if (RADAR_LOG_ERR) { \
            printk(COLOR_RED "[RADAR-ERR] " fmt COLOR_NORMAL "\n", ##__VA_ARGS__); \
        } \
    } while (0)

#define RADAR_SUCCESS(fmt, ...) \
    do { \
        if (RADAR_LOG_INFO) { \
            printk(COLOR_BRIGHT_GREEN "[RADAR] " fmt COLOR_NORMAL "\n", ##__VA_ARGS__); \
        } \
    } while (0)

// Tipos de veículo
typedef enum {
    VEHICLE_UNKNOWN = 0,
    VEHICLE_LIGHT,
    VEHICLE_HEAVY
} vehicle_type_t;

// Status de velocidade
typedef enum {
    SPEED_NORMAL = 0,
    SPEED_WARNING,
    SPEED_INFRACTION
} speed_status_t;

// Status do sistema
typedef enum {
    SYSTEM_INIT = 0,
    SYSTEM_READY,
    SYSTEM_CALIBRATING,
    SYSTEM_ERROR
} system_status_t;

// Direção do veículo
typedef enum {
    DIRECTION_UNKNOWN = 0,
    DIRECTION_FORWARD,   // Sensor1 -> Sensor2
    DIRECTION_BACKWARD   // Sensor2 -> Sensor1
} direction_t;

// Estrutura de dados do veículo
typedef struct {
    uint32_t timestamp;
    uint32_t time_between_sensors;
    vehicle_type_t type;
    float speed_kmh;
    uint8_t axle_count;
    direction_t direction;
    uint32_t total_passage_time;
    bool valid_measurement;
} vehicle_data_t;

// Estrutura de dados da câmera
typedef struct {
    char plate[8]; // AAA1A23 ou AAA1234 + null terminator
    bool valid;
    bool captured;
    uint32_t capture_time;
    vehicle_type_t vehicle_type;
    float vehicle_speed;
} camera_data_t;

// Estrutura de estatísticas do sistema
typedef struct {
    uint32_t total_vehicles;
    uint32_t light_vehicles;
    uint32_t heavy_vehicles;
    uint32_t infringements;
    uint32_t camera_failures;
    uint32_t system_errors;
} system_stats_t;

// Mensagens ZBUS
ZBUS_CHAN_DECLARE(camera_trigger_chan);
ZBUS_CHAN_DECLARE(camera_result_chan);
ZBUS_CHAN_DECLARE(system_status_chan);
ZBUS_CHAN_DECLARE(system_stats_chan);

// Estruturas ZBUS
ZBUS_CHAN_DEFINE(camera_trigger_chan,     /* Name */
                 vehicle_data_t,          /* Message type */
                 NULL,                    /* Validator */
                 NULL,                    /* User data */
                 ZBUS_OBSERVERS_EMPTY,    /* Observers */
                 ZBUS_MSG_INIT({0})       /* Initial value */
);

ZBUS_CHAN_DEFINE(camera_result_chan,      /* Name */
                 camera_data_t,           /* Message type */
                 NULL,                    /* Validator */
                 NULL,                    /* User data */
                 ZBUS_OBSERVERS_EMPTY,    /* Observers */
                 ZBUS_MSG_INIT({0})       /* Initial value */
);

ZBUS_CHAN_DEFINE(system_status_chan,      /* Name */
                 system_status_t,         /* Message type */
                 NULL,                    /* Validator */
                 NULL,                    /* User data */
                 ZBUS_OBSERVERS_EMPTY,    /* Observers */
                 ZBUS_MSG_INIT(SYSTEM_INIT) /* Initial value */
);

ZBUS_CHAN_DEFINE(system_stats_chan,       /* Name */
                 system_stats_t,          /* Message type */
                 NULL,                    /* Validator */
                 NULL,                    /* User data */
                 ZBUS_OBSERVERS_EMPTY,    /* Observers */
                 ZBUS_MSG_INIT({0})       /* Initial value */
);

// Filas de mensagens
extern struct k_msgq vehicle_data_queue;

// Semáforos e mutexes
extern struct k_sem sensor_sem;
extern struct k_mutex stats_mutex;

// Estatísticas globais do sistema
extern system_stats_t global_stats;

// Protótipos de funções públicas

// Funções de inicialização
void radar_system_init(void);
void radar_print_configuration(void);

// Funções de sensores
void sensor_interrupts_init(void);
void simulate_sensor_events(void);
bool validate_vehicle_data(const vehicle_data_t *data);

// Funções de cálculo e classificação
void calculate_speed(vehicle_data_t *vehicle);
speed_status_t check_speed_status(float speed, vehicle_type_t type);
vehicle_type_t classify_vehicle(const vehicle_data_t *data);
direction_t determine_direction(uint32_t sensor1_time, uint32_t sensor2_time);

// Funções de display
void update_display(float speed, vehicle_type_t type, speed_status_t status);
void display_system_status(system_status_t status);
void display_statistics(void);
void clear_display(void);

// Funções de câmera e placas
bool validate_license_plate(const char *plate);
void simulate_license_plate(char *plate, bool *valid);
void camera_capture_plate(const vehicle_data_t *vehicle_data);

// Funções de utilidade
uint32_t get_current_timestamp(void);
void update_system_stats(vehicle_type_t type, bool infringement, bool camera_fail);
void reset_system_stats(void);
float apply_calibration_factor(float speed);

// Funções de debug
void radar_debug_print(const char *message);
void print_vehicle_data(const vehicle_data_t *data);
void print_camera_data(const camera_data_t *data);

// Funções de teste (apenas quando em modo de teste)
#ifdef CONFIG_ZTEST
void test_calculate_speed(void);
void test_classify_vehicle(void);
void test_validate_license_plate(void);
#endif

// Funções de tratamento de erro
void handle_system_error(const char *error_message);
void recover_from_error(void);

// Constantes para validação
static const char *VEHICLE_TYPE_STR[] = {
    "DESCONHECIDO",
    "LEVE",
    "PESADO"
};

static const char *SPEED_STATUS_STR[] = {
    "NORMAL",
    "ALERTA",
    "INFRACAO"
};

static const char *DIRECTION_STR[] = {
    "DESCONHECIDA",
    "FRENTE",
    "TRAS"
};

// Inline functions para melhor performance
static inline bool is_valid_speed(float speed) {
    return (speed >= 1.0f && speed <= 300.0f); // 1-300 km/h
}

static inline bool is_valid_axle_count(uint8_t axles) {
    return (axles >= 1 && axles <= 10); // 1-10 eixos
}

static inline bool is_valid_time_between_sensors(uint32_t time_ms) {
    return (time_ms >= MIN_TIME_BETWEEN_SENSORS && time_ms <= 10000); // 10ms - 10s
}

static inline const char* get_vehicle_type_str(vehicle_type_t type) {
    if (type >= VEHICLE_UNKNOWN && type <= VEHICLE_HEAVY) {
        return VEHICLE_TYPE_STR[type];
    }
    return "INVALIDO";
}

static inline const char* get_speed_status_str(speed_status_t status) {
    if (status >= SPEED_NORMAL && status <= SPEED_INFRACTION) {
        return SPEED_STATUS_STR[status];
    }
    return "INVALIDO";
}

static inline const char* get_direction_str(direction_t direction) {
    if (direction >= DIRECTION_UNKNOWN && direction <= DIRECTION_BACKWARD) {
        return DIRECTION_STR[direction];
    }
    return "INVALIDO";
}

#endif /* RADAR_H */