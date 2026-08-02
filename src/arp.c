#include "arp.h"
#include "utils.h"
#include "ethernet.h"
#include "interface.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>

static arp_cache_t arp_cache = {
    .data = {{0}},
    .size = 0
};

int lookup_ip(uint8_t mac_dest[6], uint32_t ip)
{
    for (unsigned int i = 0; i < arp_cache.size; ++i) {
        if (arp_cache.data[i].ip == ip) {
            memcpy(mac_dest, arp_cache.data[i].mac, 6);
            return 0;
        }
    }

    return -1;
}

// Update an ip entry with new MAC
static void update_ip(uint32_t ip, uint8_t src[6])
{
    // check if ip is in table
    int idx = -1;
    for (unsigned int i = 0; i < arp_cache.size; ++i) {
        if (arp_cache.data[i].ip == ip) {
            idx = i;
            break;
        }
    }
    // if yes, update with mac
    if (idx > -1) {
        memcpy(arp_cache.data[idx].mac, src, 6);
    } else { // add new entry (circular buffer)
        unsigned int sz = arp_cache.size;
        arp_cache.data[sz].ip = ip;
        memcpy(arp_cache.data[sz].mac, src, 6);
        arp_cache.size = (sz + 1) % ARPCACHEMAX;
    }
}

int recv_arp(void *eth_frame, int fd, ssize_t (*write_interface)(int fd, const void *buf, size_t len))
{   
    ar_t *ar = (ar_t *)((eth_hdr_t *)eth_frame + 1);
    // take care to convert byte order for multibyte values
    uint16_t hardware_id = ntohs(ar->hrd);
    if (hardware_id == ARPHRD_ETHER) {
        uint16_t protocol = ntohs(ar->pro);
        if (protocol == ETH_P_IP) {
            bool merge_flag = false;
            uint8_t mac_dest[6];
            uint32_t target_ip = ntohl(ar->tpa);
            uint32_t sender_ip = ntohl(ar->spa);
            // if the sender exists in our table
            // Why the merge flag? Because we should only update our cache if we've talked to them before.
            // If we've never talked to them, then just drop the packet silently.
            if (lookup_ip(mac_dest, sender_ip) == 0) {
                // update the MAC address of the sender in the table
                update_ip(sender_ip, ar->sha);
                merge_flag = true;
            }
            // if we are the target ip address
            if (target_ip == interface_ip()) {
                printf("THIS IS AN ARP FOR US\n");
                // fill the hardware target address with our MAC
                interface_mac(ar->tha);
                // if the sender wasn't in our table
                if (merge_flag == false) {
                    uint8_t *mac = ar->sha;
                    printf("Cached new arp: ");
                    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    update_ip(sender_ip, ar->sha);
                }
                if (ntohs(ar->op) == ARPOP_REQUEST) {
                    // swap hardware
                    swap(&ar->sha, &ar->tha, sizeof(ar->sha));
                    // swap protocol
                    swap(&ar->spa, &ar->tpa, sizeof(ar->tpa));
                    ar->op = htons(ARPOP_REPLY);

                    // send
                    eth_hdr_t *eth = eth_frame;
                    memcpy(&eth->mac_src, ar->sha, ar->hln);
                    memcpy(&eth->mac_dest, ar->tha, ar->hln);
                    size_t total_size = sizeof(eth_hdr_t) + sizeof(ar_t);
                    if (write_interface(fd, eth_frame, total_size) < 0) {
                        perror("write_interface()");
                        exit(EXIT_FAILURE);
                    }

                    return 0;
                }
            }
        }
    }

    return -1;
}

void broadcast_arp(uint32_t target, int fd)
{
    // Build ethernet header with broadcast address
    int size = sizeof(eth_hdr_t) + sizeof(ar_t);
    uint8_t eth_frame[size];
    memset(eth_frame, 0, size);
    eth_hdr_t *eth_hdr = (eth_hdr_t *)eth_frame;
    eth_hdr->ethertype = htons(ETH_P_ARP);
    interface_mac(eth_hdr->mac_src);
    uint8_t broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(eth_hdr->mac_dest, broadcast_addr, 6);
    // Build ARP payload
    ar_t *ar = (ar_t *)((eth_hdr_t *)eth_frame + 1);
    ar->hrd = htons(ARPHRD_ETHER);
    ar->pro = htons(ETH_P_IP);
    ar->hln = ETH_ALEN;
    ar->pln = sizeof(struct in_addr);
    ar->op = htons(ARPOP_REQUEST);
    interface_mac(ar->sha);
    ar->spa = htonl(interface_ip());
    ar->tpa = htonl(target);

    if (write_interface(fd, (void *)eth_frame, size) < 0) {
        perror("write_interface()");
        exit(EXIT_FAILURE);
    }
}