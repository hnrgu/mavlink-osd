#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "ui_thread.h"
#include "mavlink_thread.h"
#include "xplane11_thread.h"

//#define XPLANE11

void *start_kill_thread(void *arg) {
    usleep(100000);
    printf("Timed out waiting for threads to exit, forcibly exiting\n");
    exit(1);
}

volatile static int stop_requested = 0;

void stop_all_threads() {
    if(stop_requested) {
        return;
    }
    stop_requested = 1;
    stop_ui_thread();
#ifdef XPLANE11
    stop_xplane11_thread();
#else
    stop_mavlink_thread();
#endif
    pthread_t kill_thread;
    pthread_create(&kill_thread, NULL, start_kill_thread, NULL);
}

void *ui_thread_wrapper(void *arg) {
    start_ui_thread(arg);
    printf("[main] ui thread exiting\n");
    stop_all_threads();
}

#ifdef XPLANE11
void *xplane11_thread_wrapper(void *arg) {
    start_xplane11_thread(arg);
    printf("[main] xplane11 thread exiting\n");
    stop_all_threads();
}
#else
void *mavlink_thread_wrapper(void *arg) {
    start_mavlink_thread(arg);
    printf("[main] mavlink thread exiting\n");
    stop_all_threads();
}
#endif

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s [tty]\n", argv[0]);
		return 1;
	}

	pthread_t ui_thread;
    pthread_t datafeed_thread;
    pthread_create(&ui_thread, NULL, ui_thread_wrapper, NULL);
#ifdef XPLANE11
    pthread_create(&datafeed_thread, NULL, xplane11_thread_wrapper, argv[1]);
#else
    pthread_create(&datafeed_thread, NULL, mavlink_thread_wrapper, argv[1]);
#endif
    pthread_join(ui_thread, NULL);
    pthread_join(datafeed_thread, NULL);
}
