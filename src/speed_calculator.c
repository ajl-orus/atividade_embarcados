#include "radar.h"

void calculate_speed(vehicle_data_t *vehicle)
{
    if (vehicle->time_between_sensors == 0) {
        vehicle->speed_kmh = 0.0f;
        return;
    }
    
    // Calcula velocidade em km/h
    // distância em metros / tempo em horas
    float distance_m = SENSOR_DISTANCE_MM / 1000.0f;
    float time_h = vehicle->time_between_sensors / 3600000.0f; // ms para horas
    
    vehicle->speed_kmh = distance_m / 1000.0f / time_h; // m/s para km/h
}

speed_status_t check_speed_status(float speed, vehicle_type_t type)
{
    int speed_limit = (type == VEHICLE_LIGHT) ? SPEED_LIMIT_LIGHT : SPEED_LIMIT_HEAVY;
    float warning_threshold = speed_limit * (WARNING_THRESHOLD / 100.0f);
    
    if (speed > speed_limit) {
        return SPEED_INFRACTION;
    } else if (speed > warning_threshold) {
        return SPEED_WARNING;
    } else {
        return SPEED_NORMAL;
    }
}