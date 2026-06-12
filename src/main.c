#define ETHBUFSIZ 1518
#include "ethernet.h"
#include "arp.h"
#include "interface.h"
#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/if.h>

/**
 * Need to run with sudo
 */
int main()
{
    int fd = create_interface();
    char buf[ETHBUFSIZ];
    // try 5 times
    for (int i = 0; i < 5; ++i) {
        broadcast_arp(convert_ip("192.168.1.82"), fd);
    }
    while (1) {
        ssize_t bytes = recv_interface(fd, buf, sizeof(buf));
        if (bytes < 0) {
            perror("recv_interface()");
            return -1;
        }
        handle_eth((void *)buf, fd);
    }
}