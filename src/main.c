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
        .write = write_interface,
        .read = read_interface,
        .src_mac = {0x2, 0x0, 0x0, 0x0, 0x6, 0x7},
        .src_ip = convert_ip("192.168.1.104")
    };

    while (1) {
        printf("Ethertype: %hu\n", recv_eth(interface));
        // broadcast_arp(convert_ip("192.168.1.103"), interface);
    }
}