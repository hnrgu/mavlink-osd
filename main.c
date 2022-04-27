#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <common/mavlink.h>
#include <pthread.h>

#include "ui.h"
#include "telem_data.h"

#define OK(val) if (val == -1) return 1

int set_message_interval(int fd, int cmd, int us) {
	mavlink_message_t msg;
	mavlink_msg_command_long_pack(1, 1, &msg, 1, 0, MAV_CMD_SET_MESSAGE_INTERVAL, 0, cmd, us, 0, 0, 0, 0, 0);

	uint8_t mav_buf[MAVLINK_MAX_PACKET_LEN];
	uint16_t mav_len = mavlink_msg_to_send_buffer(mav_buf, &msg);

	int written = 0;
	while (written < mav_len) {
		int len = write(fd, mav_buf + written, mav_len - written);

		if (len == -1) {
			perror("write");
			return -1;
		}

		written += len;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [tty]\n", argv[0]);
		return 1;
	}

	int fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("open");
		return 1;
	}

	struct termios tty;
	if (tcgetattr(fd, &tty) != 0) {
		perror("tcgetattr");
		return 1;
	}

	cfmakeraw(&tty);

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("tcsetattr");
		return 1;
	}

	OK(set_message_interval(fd, MAVLINK_MSG_ID_ATTITUDE, 33333) == -1);
	OK(set_message_interval(fd, MAVLINK_MSG_ID_VFR_HUD, 33333) == -1);
	OK(set_message_interval(fd, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 1000000) == -1);

	telem_init();

	pthread_t render_thread;
	pthread_create(&render_thread, NULL, render_thread_start, NULL);

	while (1) {
		char buf[2048];
		int len = read(fd, buf, sizeof(buf));

		if (len == -1) {
			perror("read");
			continue;
		}

		for (int i = 0; i < len; i ++) {
			mavlink_status_t status;
			mavlink_message_t msg;

			if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status)) {
				if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE) {
					mavlink_attitude_t attitude;
					mavlink_msg_attitude_decode(&msg, &attitude);

					telem_lock();
					telem_data.pitch = attitude.pitch;
					telem_data.roll = attitude.roll;
					telem_data.yaw = attitude.yaw;
					telem_unlock();
				} else if (msg.msgid == MAVLINK_MSG_ID_VFR_HUD) {
					mavlink_vfr_hud_t vfr_hud;
					mavlink_msg_vfr_hud_decode(&msg, &vfr_hud);

					telem_data.airspeed = vfr_hud.airspeed;
					telem_data.groundspeed = vfr_hud.groundspeed;
					telem_data.altitude = vfr_hud.alt;
					telem_data.climbrate = vfr_hud.climb;
				} else if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
					mavlink_global_position_int_t global_position;
					mavlink_msg_global_position_int_decode(&msg, &global_position);

					telem_data.lat = global_position.lat;
					telem_data.lon = global_position.lon;
				}
			}
		}
	}
}
