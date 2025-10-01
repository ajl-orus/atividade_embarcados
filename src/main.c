#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(atividade1, CONFIG_ATIVIDADE1_LOG_LEVEL);

void atividade1(struct k_timer *dummy)
{
    LOG_INF("Atividade 1 - Timer com intervalo de %d ms", CONFIG_TIMER_INTERVALO);
    LOG_DBG("Hello World");
    LOG_ERR("Hello World");
    LOG_WRN("Hello World");
}

K_TIMER_DEFINE(timer_atv1, atividade1, NULL);

int main(void)
{
    k_timer_start(&timer_atv1, K_MSEC(CONFIG_TIMER_INTERVALO), K_MSEC(CONFIG_TIMER_INTERVALO));  
    return 0;
}
