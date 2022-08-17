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
        char buf[2048];
        ssize_t len = recv(sock_fd, buf, sizeof(buf), 0);
        if (len == -1) {
            perror("[xplane11] sock_fd recv");
            return;
        }
        if (len < 5 || *(uint32_t *) buf != 0x41544144) {
            continue;
        }

        ssize_t ptr = 5;
        while(ptr + sizeof(uint32_t) + sizeof(float) * 4 <= len) {
            uint32_t msg_id = *(uint32_t *) (buf + ptr);
            ptr += sizeof(uint32_t);
            float data[8];
            for(int i = 0; i < 8; ++i) {
                data[i] = *(float *) (buf + ptr);
                ptr += sizeof(float);
            }
            printf("[xplane11] DATA %u: %f %f %f %f %f %f %f %f\n", msg_id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
            telem_lock();
            switch(msg_id) {
                case 3: // Speeds
                    telem_data.airspeed = data[0];
                    telem_data.groundspeed = data[3];
                    break;
                case 17: // pitch, roll, heading
                    telem_data.pitch = data[0] / 180.0 * M_PI;
                    telem_data.roll = data[1] / 180.0 * M_PI;
                    telem_data.yaw = data[2] / 180.0 * M_PI;
                    break;
                case 20: // lat long alt
                    telem_data.lat = data[0] * 1e7;
                    telem_data.lon = data[1] * 1e7;
                    telem_data.altitude = data[2];
                    break;
                case 132: // climb stat
                    telem_data.climbrate = data[1];
                    break;
            }
            telem_unlock();
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
