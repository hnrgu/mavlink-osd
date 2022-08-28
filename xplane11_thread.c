#include "xplane11_thread.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

#include "telem_data.h"

volatile static int running = 0;
static int sock_fd;

void start_xplane11_thread(void *arg) {
    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("[xplane11] sock_fd open");
        return;
    }

    running = 1;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(49002);

    if(bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("[xplane11] sock_fd bind");
        return;
    }

    while (running) {
        uint8_t buf[2048 + 3];
        ssize_t len = recv(sock_fd, buf + 3, 2048, 0);
        if (len == -1) {
            perror("[xplane11] sock_fd recv");
            return;
        }
        if (len < 5 || !(buf[3] == 'D' && buf[4] == 'A' && buf[5] == 'T' && buf[6] == 'A')) {
            continue;
        }

        ssize_t ptr = 8;
        while(ptr + sizeof(uint32_t) + sizeof(float) * 4 <= len + 8) {
            uint32_t msg_id = *(uint32_t *) (buf + ptr);
            ptr += sizeof(uint32_t);
            float data[8];
            for(int i = 0; i < 8; ++i) {
                data[i] = *(float *) (buf + ptr);
                ptr += sizeof(float);
            }
            switch(msg_id) {
                case 3: // Speeds
                    telem_feed(TELEM_AIRSPEED, data[0]);
                    telem_feed(TELEM_GROUNDSPEED, data[3]);
                    break;
                case 17: // pitch, roll, heading
                    telem_feed(TELEM_PITCH, data[0] / 180.0 * M_PI);
                    telem_feed(TELEM_ROLL, data[1] / 180.0 * M_PI);
                    telem_feed(TELEM_YAW, data[2] / 180.0 * M_PI);
                    break;
                case 20: // lat long alt
                    telem_feed(TELEM_LAT, data[0]);
                    telem_feed(TELEM_LON, data[1]);
                    telem_feed(TELEM_ALTITUDE, data[2]);
                    break;
                case 132: // climb stat
                    telem_feed(TELEM_CLIMBRATE, data[1]);
                    break;
            }
        }
    }
}

void stop_xplane11_thread() {
    if(running) {
        printf("[xplane11] sock_fd close\n");
        close(sock_fd);
    }
    running = 0;
}
