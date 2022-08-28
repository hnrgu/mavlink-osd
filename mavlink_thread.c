#include "mavlink_thread.h"

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <common/mavlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "telem_data.h"
#include "utils.h"

volatile static int running = 0;
static int sock_fd;

#define OK(val) if (val == -1) return

int set_message_interval(int fd, int cmd, int us) {
    mavlink_message_t msg;
    mavlink_msg_command_long_pack(5, 5, &msg, 1, 0, MAV_CMD_SET_MESSAGE_INTERVAL, 0, cmd, us, 0, 0, 0, 0, 0);

    uint8_t mav_buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t mav_len = mavlink_msg_to_send_buffer(mav_buf, &msg);

    int written = 0;
    while (written < mav_len) {
        int len = write(fd, mav_buf + written, mav_len - written);

        if (len == -1) {
            perror("[mavlink] sock fd write");
            return -1;
        }

        written += len;
    }

    return 0;
}

void start_mavlink_thread(void *arg) {
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd == -1) {
	    perror("[mavlink] sock failed");
	    return;
    }

    running = 1;

    struct sockaddr_in local_addr = {};
    local_addr.sin_family    = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY; 
    local_addr.sin_port = htons(22222);
    if(bind(sock_fd, (const struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
	    perror("[mavlink] bind failed");
    }

    struct sockaddr_in remote_addr = {};
    remote_addr.sin_family    = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("192.168.12.201");
    remote_addr.sin_port = htons(12346);
    if(connect(sock_fd, (const struct sockaddr *) &remote_addr, sizeof(remote_addr)) < 0) {
	    perror("[mavlink] connect failed");
    }


    OK(set_message_interval(sock_fd, MAVLINK_MSG_ID_ATTITUDE, 33333));
    OK(set_message_interval(sock_fd, MAVLINK_MSG_ID_VFR_HUD, 33333));
    OK(set_message_interval(sock_fd, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 1000000));

    telem_init();

    uint64_t last_time = 0;

    while (running) {
        char buf[2048];
        ssize_t len = recv(sock_fd, buf, sizeof(buf), 0);

        if (len == -1) {
            perror("[mavlink] sock read");
            break;
        }

        for (int i = 0; i < len; i ++) {
            mavlink_status_t status;
            mavlink_message_t msg;

            if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status)) {
		if(get_monotonic_time() - last_time > 1000000000LLU * 60LLU) {
		    OK(set_message_interval(sock_fd, MAVLINK_MSG_ID_ATTITUDE, 33333));
		    OK(set_message_interval(sock_fd, MAVLINK_MSG_ID_VFR_HUD, 33333));
		    OK(set_message_interval(sock_fd, MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 1000000));
		    last_time = get_monotonic_time();
		}
                if (msg.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                   mavlink_heartbeat_t heartbeat;
                   mavlink_msg_heartbeat_decode(&msg, &heartbeat);

                   if (msg.sysid != 1) {
                       continue;
                   }

                   telem_feed(TELEM_ARMED, heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED ? 1 : 0);
                   telem_feed(TELEM_MODE, heartbeat.custom_mode);
                } else if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE) {
                    mavlink_attitude_t attitude;
                    mavlink_msg_attitude_decode(&msg, &attitude);

                    telem_feed(TELEM_PITCH, attitude.pitch);
                    telem_feed(TELEM_ROLL, attitude.roll);
                    telem_feed(TELEM_YAW, attitude.yaw);
                } else if (msg.msgid == MAVLINK_MSG_ID_VFR_HUD) {
                    mavlink_vfr_hud_t vfr_hud;
                    mavlink_msg_vfr_hud_decode(&msg, &vfr_hud);

                    telem_feed(TELEM_AIRSPEED, vfr_hud.airspeed);
                    telem_feed(TELEM_GROUNDSPEED, vfr_hud.groundspeed);
                    telem_feed(TELEM_ALTITUDE, vfr_hud.alt);
                    telem_feed(TELEM_CLIMBRATE, vfr_hud.climb);
                } else if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
                    mavlink_global_position_int_t global_position;
                    mavlink_msg_global_position_int_decode(&msg, &global_position);

                    telem_feed(TELEM_LAT, global_position.lat / 1e7);
                    telem_feed(TELEM_LON, global_position.lon / 1e7);
                }
            }
        }
    }
}

void stop_mavlink_thread() {
    if(running) {
        printf("[mavlink] sock closing\n");
        close(sock_fd);
    }
    running = 0;
}
