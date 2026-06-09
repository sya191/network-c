#include "tap.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

/**
 * global instance of virtual network device configuration
 * tap_fd is initialized to -1 to indicate <down> state
 * Any positive value for fd is considered <up> state
 */ 
static netdev_t netdev = {
    .tap_fd = -1,
    .dev = {0},
    .mac = {0},
    .ip = 0,
    .start = false
};

void netdev_name(char *dest)
{
    strncpy(dest, netdev.dev, IFNAMSIZ);
}

int netdev_create()
{
    if (netdev.tap_fd != -1) {
        return netdev.tap_fd;
    }
    struct ifreq ifr;
    int fd;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open()");
        exit(EXIT_FAILURE);
    }

    memset(&ifr, 0, sizeof(ifr));

    /**
     * FLAGS: IFF_TUN - TUN device (no Ethernet headers)
     *        IFF_TAP - TAP device 
     * bitwise OR with IFF_NO_PI = no packet information
     */
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
        perror("TUNSETIFF");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    strncpy(netdev.dev, ifr.ifr_name, IFNAMSIZ);

    netdev.tap_fd = fd;

    return fd;
}

int netdev_start()
{
    if (netdev.start)  {
        return -1;
    }
    // start up process
    struct ifreq ifr;
    // need a dummy socket to tell iocntl it needs to configure network subsystem
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, netdev.dev, IFNAMSIZ);

    ifr.ifr_flags = IFF_UP;

    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        close(sock);
        exit(EXIT_FAILURE);
    }

    close(sock);

    return 0;
}
