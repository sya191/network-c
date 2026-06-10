#define DISABLEIPV6 true
#define TAPADDR "10.0.0.1" // THIS IS THE TAP DEVICE IP ADDRESS (STACK DOESN'T USE THIS)
#define IPADDR "10.0.0.123" // THE ACTUAL IP ADDRESS THIS STACK USES
#include "tap.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
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
    // locally assigned MAC address (note first byte is 0x2)
    .mac = {0x2, 0x0, 0x0, 0x0, 0x0, 0x1},
    .ip = 0,
    .start = false
};

void netdev_name(char *dest)
{
    strncpy(dest, netdev.dev, IFNAMSIZ);
}

static void disable_ipv6()
{
    char path[256] = "/proc/sys/net/ipv6/conf/";
    // append device name
    strncat(path, netdev.dev, 10);
    strncat(path, "/disable_ipv6", 15);
    int fd = open(path, O_TRUNC | O_WRONLY);
    if (fd == -1) {
        perror("open()");
        exit(EXIT_FAILURE);
    }
    if (write(fd, "1", 1) < 1) {
        perror("write()");
        exit(EXIT_FAILURE);
    }
    close(fd);
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

    // assign MAC address to TAP device in kernel
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, netdev.dev, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    memcpy(ifr.ifr_hwaddr.sa_data, netdev.mac, 6);

    if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0) {
        perror("SIOCSIFHWADDR");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // assign IP address (stack could actually not even use this ip address)

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, netdev.dev, IFNAMSIZ);
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
    addr->sin_family = AF_INET;
    inet_pton(AF_INET, TAPADDR, &addr->sin_addr);

    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
        perror("SIOCSIFADDR");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Assign network mask, implicitly defines the subnet.
    // Think of subnets as a strongly connected component on a graph.
    // If the TAP device has no range of IPs defined as the subnet,
    // The kernel won't even ARP it. Thus, we need to set the mask.
    // 10.0.0.1/24 means 3 bytes are set in stone (the first 3 because 24/8),
    // last one can be anything, so we have 255 possible devices on this subnet.
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, netdev.dev, IFNAMSIZ);
    addr = (struct sockaddr_in *)&ifr.ifr_netmask;
    addr->sin_family = AF_INET;
    // 3 byte mask
    inet_pton(AF_INET, "255.255.255.0", &addr->sin_addr);

    if (ioctl(sock, SIOCSIFNETMASK, &ifr) < 0) {
        perror("SIOCSIFNETMASK");
        close(sock);
        exit(EXIT_FAILURE);
    }

    close(sock);

    // Assign the real ip address for this stack
    inet_pton(AF_INET, IPADDR, &netdev.ip);

    // disable IPv6
    if (DISABLEIPV6) {
        disable_ipv6();
    }

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
