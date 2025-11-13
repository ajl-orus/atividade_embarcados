/* main.c
 * Exemplo Zephyr: SNTP publisher usando ZBus; dois subscribers (logger + application).
 * Compilar como parte de um projeto Zephyr com ZBus, SNTP e rede habilitados.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/net/sntp.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/net_if.h>
#include <stdint.h>
#include <inttypes.h>   /* Para usar PRIu64 e PRIu32 */

#include "aux.h"

LOG_MODULE_REGISTER(time_sync, LOG_LEVEL_INF);

/* --- Observers / Subscribers --- */
/* usar MSG subscriber (fila) já que usamos zbus_sub_wait_msg() */
ZBUS_MSG_SUBSCRIBER_DEFINE(logger_sub);
ZBUS_MSG_SUBSCRIBER_DEFINE(application_sub);

/* --- Canal ZBus --- 
 * args: name, type, validator, user_data, observers, init_val
 */
ZBUS_CHAN_DEFINE(time_channel,
                 struct time_msg,
                 NULL,
                 NULL,
                 ZBUS_OBSERVERS(logger_sub, application_sub),
                 {0});

static struct k_sem net_ready;
static struct net_mgmt_event_callback net_cb;

static void net_event_handler(struct net_mgmt_event_callback *cb,
                              uint64_t mgmt_event, struct net_if *iface)
{
    ARG_UNUSED(cb);
    ARG_UNUSED(iface);

    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        k_sem_give(&net_ready);
    }
}

/* --- SNTP publisher thread --- */
void sntp_client_thread(void *a, void *b, void *c)
{
	struct sntp_time ts;
	int rc;
	bool already_connected = false;

	const k_timeout_t poll_interval = K_SECONDS(1);

	/* aguarda um pouco antes de iniciar (boot) */
	k_sleep(K_SECONDS(5));

	/* inicializa mecanismo que espera a interface IPv4 */
	k_sem_init(&net_ready, 0, 1);
	net_mgmt_init_event_callback(&net_cb, net_event_handler, NET_EVENT_IPV4_ADDR_ADD);
	net_mgmt_add_event_callback(&net_cb);

	#if defined(CONFIG_NET_LOOPBACK) || defined(CONFIG_NET_NATIVE)
    	k_sem_give(&net_ready);
    	LOG_WRN("Rede simulada: semáforo liberado manualmente");
	#endif

	while (1) {
		if (!k_sem_take(&net_ready, K_SECONDS(5)) != 0 || !already_connected) {
			LOG_INF("Aguardando interface rede...");
			already_connected = true;
			continue;
		}

		LOG_DBG("Tentando sntp_simple...");
		rc = sntp_simple(CONFIG_SNTP_SERVER_ADDR, 5000, &ts);
		LOG_DBG("sntp_simple rc=%d", rc);

		if (rc == 0) {
			struct time_msg tm = {
				.seconds = ts.seconds,
				.fraction = ts.fraction
			};

			rc = zbus_chan_pub(&time_channel, &tm, K_SECONDS(1));
			if (rc == 0) {
				LOG_INF("SNTP sync ok, published seconds=%" PRIu64, tm.seconds);
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
		int rc = zbus_sub_wait_msg(&logger_sub, &chan, &msg, K_FOREVER);
		if (rc == 0) {
			LOG_INF("Logger recebeu tempo: seconds=%" PRIu64 " fraction=%" PRIu32,
			        msg.seconds, msg.fraction);
		} else {
			LOG_ERR("zbus_sub_wait_msg falhou (rc=%d)", rc);
		}
	}
}

/* --- Application subscriber --- */
void application_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
	const struct zbus_channel *chan = NULL;
	struct time_msg msg;

	LOG_INF("Application thread pronto; aguardando tempo...");

	while (1) {
		int rc = zbus_sub_wait_msg(&application_sub, &chan, &msg, K_FOREVER);
		if (rc == 0) {
			printk("Application: recebido tempo (sec=%" PRIu64 ")\n", msg.seconds);
		} else {
			LOG_ERR("zbus_sub_wait_msg (app) falhou (rc=%d)", rc);
		}
	}
}

/* --- Definição das threads --- */
K_THREAD_DEFINE(sntp_tid, CONFIG_STACK_SIZE, sntp_client_thread, NULL, NULL, NULL,
                CONFIG_PRIORITY, 0, 0);
K_THREAD_DEFINE(logger_tid, CONFIG_STACK_SIZE, logger_thread, NULL, NULL, NULL,
                CONFIG_PRIORITY + 3, 0, 0);
K_THREAD_DEFINE(app_tid, CONFIG_STACK_SIZE, application_thread, NULL, NULL, NULL,
                CONFIG_PRIORITY + 5, 0, 0);
