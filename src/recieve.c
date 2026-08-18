#include "recieve.h"
#include "ethernet.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_t rx;
bool running = false;

static void *rx_thread(void *interface)
{
    iface_t *iface = (iface_t *)interface;
    while (running) {
        recv_eth(*iface);
    }

    return NULL;
}

void start_rx(iface_t interface)
{
    if (running) return;

    running = true;
    if (pthread_create(&rx, NULL, rx_thread, &interface) != 0) {
        perror("pthread_create()");
        exit(EXIT_FAILURE);
    }
}

void stop_rx()
{
    running = false;
    if (pthread_join(rx, NULL) != 0) {
        perror("pthread_join()");
        exit(EXIT_FAILURE);
    }
}