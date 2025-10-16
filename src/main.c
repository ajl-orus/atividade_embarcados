#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/gpio.h>

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

struct sensor_data
{
    double temperature;
    double humidity;
};

K_MSGQ_DEFINE(filter_input_msgq, sizeof(struct sensor_data), 10, 1);
K_MSGQ_DEFINE(filter_output_msgq, sizeof(struct sensor_data), 10, 1);

/**
 * Simulate reading temperature and humidity from sensors.
 */
double read_temperature()
{
    return 10.0 + (rand() % 100) / 10.0; 
}

/**
 * Simulate reading humidity from a sensor.
 */
double read_humidity()
{
    return 10.0 + (rand() % 100) / 10.0; 
}

/**
 * Threads for reading temperature sensors and filtering data.
 */
void temperature_sensor(void *p1, void *p2, void *p3)
{
    struct sensor_data data;
    while (1) {
        LOG_DBG("Reading temperature sensor...");
        data.temperature = read_temperature();
        data.humidity = NULL;

        while(k_msgq_put(&filter_input_msgq, &data, K_MSEC(100)) != 0) {
            LOG_WRN("Filter input message queue full, dropping data");
            k_msgq_purge(&filter_input_msgq);
        }

        k_sleep(K_SECONDS(1));
    }
}

/**
 * Threads for reading humidity sensors and filtering data.
 */
void humidity_sensor(void *p1, void *p2, void *p3)
{
    struct sensor_data data;
    while (1) {
        LOG_DBG("Reading humidity sensor...");
        data.humidity = read_humidity();
        data.temperature = NULL;

        while(k_msgq_put(&filter_input_msgq, &data, K_MSEC(100)) != 0) {
            LOG_WRN("Filter input message queue full, dropping data");
            k_msgq_purge(&filter_input_msgq);
        }

        k_sleep(K_SECONDS(1));
    }
}

/**
 * Thread for filtering sensors data.
 */
void filter_data(void *p1, void *p2, void *p3)
{
    struct sensor_data data;

    while (1) {
        LOG_DBG("Filtering data...");

        k_msgq_get(&filter_input_msgq, &data, K_FOREVER);

        if(data.temperature == NULL && data.humidity == NULL) {
            LOG_WRN("Received data with no valid fields, dropping");
            k_msgq_purge(&filter_input_msgq);
            continue;
        }

        if(data.temperature != NULL) {
            if(data.temperature <= 18.0 || data.temperature >= 30.0) {
                LOG_WRN("Temperature out of range: %f", data.temperature);
                continue;
            }
        }

        if(data.humidity != NULL) {
            if(data.humidity <= 40.0 || data.humidity >= 70.0) {
                LOG_WRN("Humidity out of range: %f", data.humidity);
                continue;
            }
        }

        while(k_msgq_put(&filter_output_msgq, &data, K_MSEC(100)) != 0) {
            LOG_WRN("Filter output message queue full, dropping data");
            k_msgq_purge(&filter_output_msgq);
        }

        k_sleep(K_SECONDS(1));
    }
}


int main(void)
{
    struct sensor_data data;
    k_msgq_get(&filter_output_msgq, &data, K_FOREVER);
    return 0;
}