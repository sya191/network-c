#include "ethernet.h"
#include "arp.h"
#include <linux/if_ether.h>
#include <stdio.h>
#include <arpa/inet.h>

int handle_eth(void *addr, int fd)
{
    if (addr == NULL) {
        return -1;
    }
    // cast addr to eth_hdr
    eth_hdr_t *hdr = (eth_hdr_t *)addr;
    switch (ntohs(hdr->ethertype)) {
        case ETH_P_ARP:
            return recv_arp(hdr, fd);
        default:
            break;
    }
}