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

/**
 * Need to run with sudo
 */
int main()
{
    int fd = create_interface();
    iface_t interface = {
        .fd = fd,
        .write_interface = write_interface
    };
    // set up buffer
    char buf[ETHBUFSIZ];
    while (1) {
        ssize_t bytes = recv_interface(fd, buf, sizeof(buf));
        if (bytes < 0) {
            perror("recv_interface()");
            return -1;
        }
        recv_eth((void *)buf, interface);
    }
}