#ifndef TELEM_DATA_H
#define TELEM_DATA_H

struct telem_data {
	float roll; // rad
	float pitch; // rad
	float yaw; // rad
	float heading; // deg
	float altitude; // m, msl
	float airspeed; // m/s
	float groundspeed; // m/s
	float climbrate; // m/s
	int lat; // degE7
	int lon; // degE7
};

extern struct telem_data telem_data;

void telem_init();
void telem_lock();
void telem_unlock();

#endif
