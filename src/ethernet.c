#include "ethernet.h"
#include "arp.h"
#include <linux/if_ether.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int recv_eth(void *addr, iface_t interface)
{
    if (addr == NULL) {
        return -1;
    }
    // cast addr to eth_hdr
    eth_hdr_t *hdr = (eth_hdr_t *)addr;
    void *payload = hdr + sizeof(eth_hdr_t);
    switch (ntohs(hdr->ethertype)) {
        case ETH_P_ARP:
            return recv_arp(payload, interface);
        default:
            break;
    }

    return 0;
}

int send_eth_to_ip(
    void *payload, 
    uint16_t ethertype,
    uint32_t target_ip,
    size_t len,
    iface_t interface)
{
    size_t size = sizeof(eth_hdr_t) + len;
    uint8_t buf[size];

    // check if target_ip is in ARP cache
    uint8_t *target_mac = ((eth_hdr_t *)buf)->mac_dest;
    if (lookup_ip(target_mac, target_ip) == -1) {
        // target mac not found, send ARP REQUEST
        broadcast_arp(target_ip, interface);
        return -1; // notify caller with -1 retval
    }

    uint8_t *source_mac = ((eth_hdr_t *)buf)->mac_src;
    memcpy(source_mac, interface.src_mac, 6);
    ((eth_hdr_t *)buf)->ethertype = htons(ethertype);
    // copy payload to buffer
    memcpy(buf + sizeof(eth_hdr_t), payload, len);
    interface.write_interface(interface.fd, buf, size);

    return 0;
}

int send_eth_to_mac(
    void *payload,
    uint16_t ethertype,
    uint8_t target_mac[6],
    size_t len,
    iface_t interface)
{
    size_t size = sizeof(eth_hdr_t) + len;
    uint8_t buf[size];

    // send ethernet frame directly to mac address
    uint8_t *dest_mac = ((eth_hdr_t *)buf)->mac_dest;
    memcpy(dest_mac, target_mac, 6);
    uint8_t *source_mac = ((eth_hdr_t *)buf)->mac_src;
    memcpy(source_mac, interface.src_mac, 6);
    ((eth_hdr_t *)buf)->ethertype = htons(ethertype);

    // copy payload to buffer
    memcpy(buf + sizeof(eth_hdr_t), payload, len);
    if (interface.write_interface(interface.fd, buf, size) < 0) {
        perror("Write Interface");
        exit(EXIT_FAILURE);
    }

    return 0;
}