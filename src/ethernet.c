#include "ethernet.h"
#include <linux/if_ether.h>
#include <stdio.h>
#include <arpa/inet.h>

int handle_eth(void *addr)
{
    // cast addr to eth_hdr
    eth_hdr_t hdr = *(eth_hdr_t *)addr;
    switch (ntohs(hdr.ethertype)) {
        case ETH_P_ARP:
            printf("THIS IS AN ARP\n");
            break;
        default:
            break;
    }
}