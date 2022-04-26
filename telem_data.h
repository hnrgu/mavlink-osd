#ifndef TELEM_DATA_H
#define TELEM_DATA_H

struct telem_data {
	float roll;
	float pitch;
	float yaw;
};

extern struct telem_data telem_data;

void telem_init();
void telem_lock();
void telem_unlock();

#endif
