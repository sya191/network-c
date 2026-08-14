#define ETHBUFSIZ 1518
#include "ethernet.h"
#include "arp.h"
#include "interface.h"
#include "iface.h"
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/if.h>
#include <arpa/inet.h>

/**
 * Need to run with sudo
 */
int main()
{
    iface_t interface = {
        .fd = create_interface(),
        .write_interface = write_interface,
        .src_mac = {0x2, 0x0, 0x0, 0x0, 0x6, 0x7}
    };
    inet_pton(AF_INET, "192.168.1.104", &interface.src_ip);
    interface.src_ip = ntohl(interface.src_ip);

    // set up buffer
    char buf[ETHBUFSIZ];
    while (1) {
        ssize_t bytes = recv_interface(interface.fd, buf, sizeof(buf));
        if (bytes < 0) {
            perror("recv_interface()");
            return -1;
        }
        recv_eth((void *)buf, interface);
        // broadcast_arp(convert_ip("192.168.1.103"), interface);
    }
}