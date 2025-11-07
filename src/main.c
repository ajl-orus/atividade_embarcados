/* main.c
 * Exemplo Zephyr: SNTP publisher usando ZBus; dois subscribers (logger + application).
 * Compilar como parte de um projeto Zephyr com ZBus, SNTP e rede habilitados.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/net/sntp.h>
#include <zephyr/sys/printk.h>
#include <stdint.h>

LOG_MODULE_REGISTER(time_sync, LOG_LEVEL_INF);

/* --- Estrutura da mensagem de tempo --- */
struct time_msg {
	uint64_t seconds;   /* segundos desde 1970-01-01 (Unix epoch) */
	uint32_t fraction;  /* fração (32-bit) conforme sntp_time.fraction */
};

/* --- Observers / Subscribers --- */
ZBUS_SUBSCRIBER_DEFINE(logger_sub, 4);
ZBUS_SUBSCRIBER_DEFINE(application_sub, 4);

/* --- Canal ZBus --- 
 * args: name, type, validator, user_data, observers, init_val
 */
ZBUS_CHAN_DEFINE(time_channel,
                 struct time_msg,
                 NULL,
                 NULL,
                 ZBUS_OBSERVERS(logger_sub, application_sub),
                 {0});

/* Thread stacks / priorities */
#define STACK_SIZE 1024
#define PRIO 5

/* Forward declarations de threads (K_THREAD_DEFINE) */
void sntp_client_thread(void *a, void *b, void *c);
void logger_thread(void *a, void *b, void *c);
void application_thread(void *a, void *b, void *c);

K_THREAD_DEFINE(sntp_tid, STACK_SIZE, sntp_client_thread, NULL, NULL, NULL,
                PRIO, 0, 0);
K_THREAD_DEFINE(logger_tid, STACK_SIZE, logger_thread, NULL, NULL, NULL,
                PRIO, 0, 0);
K_THREAD_DEFINE(app_tid, STACK_SIZE, application_thread, NULL, NULL, NULL,
                PRIO, 0, 0);

/* --- SNTP publisher thread --- */
void sntp_client_thread(void *a, void *b, void *c)
{
	struct sntp_time ts;
	int rc;

	/* Tempo entre sincronizações (ajustável) */
	const k_timeout_t poll_interval = K_SECONDS(60);

	while (1) {
		/* Usamos a conveniência sntp_simple para uma consulta "one-shot". 
		 * O endereço vem de prj.conf via Kconfig (CONFIG_SNTP_SERVER_ADDR).
		 */
		rc = sntp_simple(CONFIG_SNTP_SERVER_ADDR, 3000, &ts);
		if (rc == 0) {
			struct time_msg tm = {
				.seconds = ts.seconds,
				.fraction = ts.fraction
			};

			/* Publica no canal. Timeout curto para evitar bloqueio grande. */
			rc = zbus_chan_pub(&time_channel, &tm, K_SECONDS(1));
			if (rc == 0) {
				LOG_INF("SNTP sync ok, published seconds=%llu", tm.seconds);
			} else {
				LOG_WRN("Falha ao publicar no time_channel (rc=%d)", rc);
			}
		} else {
			LOG_WRN("SNTP query failed (rc=%d)", rc);
		}

		k_sleep(poll_interval);
	}
}

/* --- Logger subscriber --- */
void logger_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	const struct zbus_channel *chan = NULL;
	struct time_msg msg;

	LOG_INF("Logger thread pronto; aguardando notificações...");

	while (1) {
		/* Espera por nova mensagem e copia para 'msg' */
		int rc = zbus_sub_wait_msg(&logger_sub, &chan, &msg, K_FOREVER);
		if (rc == 0) {
			/* Exibe log claro com o timestamp recebido */
			LOG_INF("Logger recebeu tempo: seconds=%llu fraction=%u",
			        msg.seconds, msg.fraction);
		} else {
			LOG_ERR("zbus_sub_wait_msg falhou (rc=%d)", rc);
		}
	}
}

/* --- Application subscriber (simples confirmação) --- */
void application_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	const struct zbus_channel *chan = NULL;
	struct time_msg msg;

	LOG_INF("Application thread pronto; aguardando tempo...");

	while (1) {
		int rc = zbus_sub_wait_msg(&application_sub, &chan, &msg, K_FOREVER);
		if (rc == 0) {
			/* Apenas confirma recebimento (poderia acionar RTC, timers, etc.) */
			printk("Application: recebido tempo (sec=%llu)\n", msg.seconds);
		} else {
			LOG_ERR("zbus_sub_wait_msg (app) falhou (rc=%d)", rc);
		}
	}
}