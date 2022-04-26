#include "telem_data.h"

#include <pthread.h>

static pthread_mutex_t lock;

struct telem_data telem_data;

void telem_init() {
	pthread_mutex_init(&lock, NULL);
}

void telem_lock() {
	pthread_mutex_lock(&lock);
}

void telem_unlock() {
	pthread_mutex_unlock(&lock);
}