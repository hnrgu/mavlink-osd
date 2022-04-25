#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <common/mavlink.h>

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
				printf("received message: %d\n", msg.msgid);
				if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE) {
					mavlink_attitude_t attitude;
					mavlink_msg_attitude_decode(&msg, &attitude);

					printf("attitude: roll: %f, pitch: %f, yaw: %f\n", attitude.roll, attitude.pitch, attitude.yaw);
				}
			}
		}
	}
}
