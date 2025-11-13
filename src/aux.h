#ifndef AUX_H
#define AUX_H

/* Forward declarations de threads (K_THREAD_DEFINE) */
void sntp_client_thread(void *a, void *b, void *c);
void logger_thread(void *a, void *b, void *c);
void application_thread(void *a, void *b, void *c);

/* --- Estrutura da mensagem de tempo --- */
struct time_msg {
	uint64_t seconds;   /* segundos desde 1970-01-01 (Unix epoch) */
	uint32_t fraction;  /* fração (32-bit) conforme sntp_time.fraction */
};

#endif /* AUX_H */