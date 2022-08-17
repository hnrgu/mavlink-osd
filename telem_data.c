#include "telem_data.h"

#include "utils.h"
#include <math.h>
#include <pthread.h>

static pthread_mutex_t lock;

static struct telem_data_field fields[TELEM_FIELD_COUNT];

void telem_init() {
    pthread_mutex_init(&lock, NULL);
}

void telem_feed(enum telem_field field, float data) {
    pthread_mutex_lock(&lock);
    fields[field].prev_data = fields[field].data;
    fields[field].data = data;
    long cur_time = get_monotonic_time();
    float time_diff = (cur_time - fields[field].last_updated) / 1e9;
    fields[field].last_updated = cur_time;
    if(time_diff < 0.001) {
        time_diff = 0.001;
    }
    if(time_diff > 0.25) {
        fields[field].stale = 1;
    } else {
        fields[field].stale = 0;
        fields[field].update_rate = fields[field].update_rate * 0.5 + (1.0 / time_diff) * 0.5;
    }
    pthread_mutex_unlock(&lock);
}

float telem_get(enum telem_field field) {
    pthread_mutex_lock(&lock);
    long time = get_monotonic_time();
    float timediff = (time - fields[field].last_updated) / 1e9;
    float interp = timediff * fields[field].update_rate;
    if(interp > 1 || fields[field].stale) {
        interp = 1;
    }
    float data = fields[field].prev_data * (1 - interp) + fields[field].data * interp;
    pthread_mutex_unlock(&lock);
    return data;
}

float telem_get_360(enum telem_field field) {
    pthread_mutex_lock(&lock);
    long time = get_monotonic_time();
    float timediff = (time - fields[field].last_updated) / 1e9;
    float interp = timediff * fields[field].update_rate;
    if(interp > 1 || fields[field].stale) {
        interp = 1;
    }
    float datadiff = fields[field].data - fields[field].prev_data;
    datadiff = fmod(fmod(datadiff, 2 * M_PI) + 2 * M_PI, 2 * M_PI);
    if(datadiff >= M_PI) {
        datadiff -= 2 * M_PI;
    }
    float data = fields[field].prev_data * (1 - interp) + (fields[field].prev_data + datadiff) * interp;
    data = fmod(fmod(data, 2 * M_PI) + 2 * M_PI, 2 * M_PI);
    pthread_mutex_unlock(&lock);
    return data;
}

float telem_get_180(enum telem_field field) {
    float data = telem_get_360(field);
    if(data >= M_PI) {
        data -= 2 * M_PI;
    }
    return data;
}
