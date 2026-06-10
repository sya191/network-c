#define ETHBUFSIZ 1500
#include "tap.h"
#include "ethernet.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/if.h>

/**
 * Need to run with sudo
 */
int main()
{
    int fd = netdev_create();
    netdev_start();
    char dev_name[IFNAMSIZ];
    netdev_name(dev_name);
    printf("TAP fd: %d, dev: %s\n", fd, dev_name);
    char buf[ETHBUFSIZ];
    while (1) {
        ssize_t bytes = read(fd, buf, sizeof(buf));
        if (bytes < 0) {
            perror("read()");
            return -1;
        }
        printf("%ld bytes read from TAP\n", bytes);
        handle_eth((void *)buf, fd);
    }
}