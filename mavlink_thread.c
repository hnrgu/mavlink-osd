#include "mavlink_thread.h"

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <common/mavlink.h>

#include "telem_data.h"

volatile static int running = 0;
static int tty_fd;

#define OK(val) if (val == -1) return

int set_message_interval(int fd, int cmd, int us) {
    mavlink_message_t msg;
    mavlink_msg_command_long_pack(1, 1, &msg, 1, 0, MAV_CMD_SET_MESSAGE_INTERVAL, 0, cmd, us, 0, 0, 0, 0, 0);

    uint8_t mav_buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t mav_len = mavlink_msg_to_send_buffer(mav_buf, &msg);

    int written = 0;
    while (written < mav_len) {
        int len = write(fd, mav_buf + written, mav_len - written);

        if (len == -1) {
            perror("[mavlink] tty fd write");
            return -1;
        }

        written += len;
    }

    return 0;
}

void start_mavlink_thread(void *arg) {
    tty_fd = open((const char *) arg, O_RDWR);
    if (tty_fd == -1) {
        perror("[mavlink] tty open");
        return;
    }

    running = 1;

    struct termios tty;
    if (tcgetattr(tty_fd, &tty) != 0) {
        perror("[mavlink] tty tcgetattr");
        return;
    }

    cfmakeraw(&tty);
    cfsetispeed(&tty, B921600);
    cfsetospeed(&tty, B921600);

    if (tcsetattr(tty_fd, TCSANOW, &tty) != 0) {
        perror("[mavlink] tty tcsetattr");
        return;
    }

    OK(set_message_interval(tty_fd, MAVLINK_MSG_ID_ATTITUDE, 33333));
    OK(set_message_interval(tty_fd, MAVLINK_MSG_ID_VFR_HUD, 33333));
    OK(set_message_interval(tty_fd, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 1000000));

    telem_init();

    while (running) {
        char buf[2048];
        ssize_t len = read(tty_fd, buf, sizeof(buf));

        if (len == -1) {
            perror("[mavlink] tty read");
            break;
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

void stop_mavlink_thread() {
    if(running) {
        printf("[mavlink] tty closing\n");
        close(tty_fd);
    }
    running = 0;
}
