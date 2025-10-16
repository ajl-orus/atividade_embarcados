#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>
#include <time.h>


LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

struct sensor_data
{
    int temperature;
    int humidity;
};

K_MSGQ_DEFINE(filter_input_msgq, sizeof(struct sensor_data), 10, 1);
K_MSGQ_DEFINE(filter_output_msgq, sizeof(struct sensor_data), 10, 1);

/**
* Simulate reading temperature and humidity from sensors.
*/
int read_temperature()
{
    int value = 10.0 + (rand() % 100); 
    return value;
}

/**
* Simulate reading humidity from a sensor.
*/
int read_humidity()
{
    int value =  10.0 + (rand() % 100); 
    return value;
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
        data.humidity = 0.0;

        while(k_msgq_put(&filter_input_msgq, &data, K_MSEC(100)) != 0) {
            LOG_DBG("Filter input message queue full, dropping data");
            k_msgq_purge(&filter_input_msgq);
        }

        k_sleep(K_SECONDS(5));
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
        data.temperature = 0.0;
        
        while(k_msgq_put(&filter_input_msgq, &data, K_MSEC(100)) != 0) {
            LOG_DBG("Filter input message queue full, dropping data");
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
        
        if(data.temperature == 0.0 && data.humidity == 0.0) {
            LOG_WRN("Received data with no valid fields, dropping");
            k_msgq_purge(&filter_input_msgq);
            continue;
        }
        
        if(data.temperature != 0.0) {
            if(data.temperature <= 18 || data.temperature >= 30) {
                LOG_WRN("Temperature out of range: %d", data.temperature);
                continue;
            }
        }
        
        if(data.humidity != 0.0) {
            if(data.humidity <= 40 || data.humidity >= 70) {
                LOG_WRN("Humidity out of range: %d", data.humidity);
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
    srand(time(NULL)); 
    
    struct sensor_data data;

    while (1){
        if (k_msgq_get(&filter_output_msgq, &data, K_FOREVER) != 0) {
            LOG_ERR("Failed to get data from filter output message queue");
            continue;
        }
        LOG_INF("Filtered Data - Temperature: %d, Humidity: %d", data.temperature, data.humidity);

    }

    return 0;
}

K_THREAD_DEFINE(temperature_sensor_id, 1024, temperature_sensor, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(humidity_sensor_id, 1024, humidity_sensor, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(filter_data_id, 1024, filter_data, NULL, NULL, NULL, 8, 0, 0);