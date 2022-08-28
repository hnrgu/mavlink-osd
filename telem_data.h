#ifndef TELEM_DATA_H
#define TELEM_DATA_H

#include <stdint.h>

enum telem_field {
    TELEM_ROLL,
    TELEM_PITCH,
    TELEM_YAW,
    TELEM_ALTITUDE,
    TELEM_AIRSPEED,
    TELEM_GROUNDSPEED,
    TELEM_CLIMBRATE,
    TELEM_LAT,
    TELEM_LON,
    TELEM_ARMED,
    TELEM_MODE,
    TELEM_FIELD_COUNT,
};

struct telem_data_field {
    float prev_data, data, update_rate;
    uint64_t last_updated;
    int stale;
};

void telem_init();
void telem_feed(enum telem_field field, float data);
float telem_get(enum telem_field field);
float telem_get_180(enum telem_field field);
float telem_get_360(enum telem_field field);
float telem_get_raw(enum telem_field field);

#endif
