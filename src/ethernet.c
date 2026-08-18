#include "ethernet.h"
#include "arp.h"
#include "utils.h"
#include <linux/if_ether.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// untested
static bool 
mac_for_us(uint8_t mac[6], iface_t interface)
{
    // check if it is broadcast/interface mac
    bool is_us = true;
    bool is_broadcast = true;
    for (int i = 0; i < 6; ++i) {
        if (mac[i] != interface.src_mac[i]) {
            is_us = false;
        }
    }

    if (is_us) return true;

    for (int i = 0; i < 6; ++i) {
        if (mac[i] != 0xff) {
            is_broadcast = false;
        }
    }

    return is_broadcast;
}

int recv_eth(iface_t interface)
{
    // read eth frame into buffer
    uint8_t buf[1500];
    if (interface.read(interface.fd, buf) < 0) {
        return -1;
    }
    // cast addr to eth_hdr
    eth_hdr_t *hdr = (eth_hdr_t *)buf;

    // filter mac destination
    if (!mac_for_us(hdr->mac_dest, interface)) {
        return -1;
    }

    void *payload = (void *)hdr + sizeof(eth_hdr_t);
    uint16_t ethertype = ntohs(hdr->ethertype);
    switch (ethertype) {
        case ETH_P_ARP:
            recv_arp(payload, interface);
        default:
            break;
    }

    return ethertype;
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
    double time_start = get_timestamp();
    double last_arp = -1;
    // TODO: MAKE lookup_ip and update_ip thread safe
    while (lookup_ip(target_mac, target_ip) < 0) {
        // calculate difference between last time stamp
        double current_time = get_timestamp();
        double diff = current_time - last_arp;
        // broadcast arp every 0.25 second
        if (diff > 0.25) {
            broadcast_arp(target_ip, interface);
            last_arp = current_time;
        }
        // try for 5 seconds
        if (current_time - time_start > 5) {
            return -1;
        }
    }

    uint8_t *source_mac = ((eth_hdr_t *)buf)->mac_src;
    memcpy(source_mac, interface.src_mac, 6);
    ((eth_hdr_t *)buf)->ethertype = htons(ethertype);
    // copy payload to buffer
    memcpy(buf + sizeof(eth_hdr_t), payload, len);
    interface.write(interface.fd, buf, size);

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
    if (interface.write(interface.fd, buf, size) < 0) {
        perror("Write Interface");
        exit(EXIT_FAILURE);
    }

    return 0;
}