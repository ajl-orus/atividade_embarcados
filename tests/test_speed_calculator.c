#include <ztest.h>
#include "radar.h"

void test_speed_calculation(void)
{
    vehicle_data_t vehicle = {
        .time_between_sensors = 100 // 100ms para 500mm
    };
    
    calculate_speed(&vehicle);
    zassert_true(vehicle.speed_kmh > 17.9 && vehicle.speed_kmh < 18.1, 
                "Velocidade calculada incorretamente");
}

void test_speed_status(void)
{
    // Teste veículo leve
    zassert_equal(check_speed_status(70, VEHICLE_LIGHT), SPEED_NORMAL, 
                 "Deveria ser normal");
    zassert_equal(check_speed_status(75, VEHICLE_LIGHT), SPEED_WARNING, 
                 "Deveria ser alerta");
    zassert_equal(check_speed_status(85, VEHICLE_LIGHT), SPEED_INFRACTION, 
                 "Deveria ser infração");
}

void test_main(void)
{
    ztest_test_suite(radar_tests,
        ztest_unit_test(test_speed_calculation),
        ztest_unit_test(test_speed_status)
    );
    ztest_run_test_suite(radar_tests);
}