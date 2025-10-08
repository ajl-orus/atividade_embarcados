#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL); 

#define LED_NODE DT_ALIAS(led0)
#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button   = GPIO_DT_SPEC_GET(SW0_NODE, gpios);


static struct gpio_callback button_cb_data;

static bool mode = false;
static uint8_t brightness;
static int8_t step = 5;


static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    mode = !mode;
    brightness = 0;
    step = 5;
}

void set_led_brightness(uint8_t duty_percent)
{
    uint32_t high_time = (CONFIG_PERIOD * duty_percent) / 100;
    uint32_t low_time = CONFIG_PERIOD - high_time;

    gpio_pin_set_dt(&led, 1);
    k_msleep(high_time);

    gpio_pin_set_dt(&led, 0);
    k_msleep(low_time);
}

int main(void)
{
    int ret;

    /*
    *
    * Configure the button to work with edge rising like input
    *
    */

    if(!gpio_is_ready_dt(&button)) {
        LOG_ERR("GPIOS not suported!");
        return 0;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return 0;
	}

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
    LOG_INF("Set up button at %s pin %d\n", button.port->name, button.pin);

    /*
    *
    * General configuration to the led
    *
    */

    if (led.port && !gpio_is_ready_dt(&led)) {
		LOG_ERR("Error %d: LED device %s is not ready; ignoring it\n", ret, led.port->name);
		return 0;
	}

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_LOW);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure LED device %s pin %d\n", ret, led.port->name, led.pin);
        return 0;
    } 

    while(true) {
        if (mode){
            k_msleep(CONFIG_BLINK_TIME);
            gpio_pin_toggle_dt(&led);
            continue;
        }
        set_led_brightness(brightness);

        brightness += step;
        if (brightness == 0 || brightness == 100) {
            step = -step;
        }

        k_msleep(500);
    }

    return 0;
}