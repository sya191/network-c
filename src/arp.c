#include "arp.h"
#include "utils.h"
#include "ethernet.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>

static pthread_rwlock_t CACHE_LOCK = PTHREAD_RWLOCK_INITIALIZER;
static arp_cache_t arp_cache = {
    .data = {{0}},
    .size = 0
};

int lookup_ip(uint8_t mac_dest[6], uint32_t ip)
{
    if (pthread_rwlock_rdlock(&CACHE_LOCK) != 0) {
        perror("pthread_rwlock_rdlock()");
        exit(EXIT_FAILURE);
    }
    int retval = -1;
    for (unsigned int i = 0; i < arp_cache.size; ++i) {
        if (arp_cache.data[i].ip == ip) {
            memcpy(mac_dest, arp_cache.data[i].mac, 6);
            retval = 0;
            goto cleanup;
        }
    }
cleanup:
    if (pthread_rwlock_unlock(&CACHE_LOCK) != 0) {
        perror("pthread_rwlock_rdlock()");
        exit(EXIT_FAILURE);
    }

    return retval;
}

// Update an ip entry with new MAC
static void update_ip(uint32_t ip, uint8_t src[6])
{
    if (pthread_rwlock_wrlock(&CACHE_LOCK) != 0) {
        perror("pthread_rwlock_wrlock()");
        exit(EXIT_FAILURE);
    }
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
    if (pthread_rwlock_unlock(&CACHE_LOCK) != 0) {
        perror("pthread_rwlock_wrlock()");
        exit(EXIT_FAILURE);
    }
}

// TODO: recv_arp should call the ethernet module to send frames
int recv_arp(ar_t *arp_msg, iface_t interface) 
{   
    // take care to convert byte order for multibyte values
    uint16_t hardware_id = ntohs(arp_msg->hrd);
    if (hardware_id == ARPHRD_ETHER) {
        uint16_t protocol = ntohs(arp_msg->pro);
        if (protocol == ETH_P_IP) {
            bool merge_flag = false;
            uint8_t mac_dest[6];
            uint32_t target_ip = ntohl(arp_msg->tpa);
            uint32_t sender_ip = ntohl(arp_msg->spa);
            // if the sender exists in our table
            // Why the merge flag? Because we should only update our cache if we've talked to them before.
            // If we've never talked to them, then just drop the packet silently.
            if (lookup_ip(mac_dest, sender_ip) == 0) {
                // update the MAC address of the sender in the table
                update_ip(sender_ip, arp_msg->sha);
                merge_flag = true;
            }
            // if we are the target ip address
            if (target_ip == interface.src_ip) {
                // fill the hardware target address with our MAC
                memcpy(arp_msg->tha, interface.src_mac, 6);
                // if the sender wasn't in our table
                if (merge_flag == false) {
                    uint8_t *mac = arp_msg->sha;
                    printf("Cached new arp: ");
                    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    update_ip(sender_ip, arp_msg->sha);
                }
                // TODO: refactor below this line to a send_arp() function
                if (ntohs(arp_msg->op) == ARPOP_REQUEST) {
                    // swap hardware
                    swap(&arp_msg->sha, &arp_msg->tha, sizeof(arp_msg->sha));
                    // swap protocol
                    swap(&arp_msg->spa, &arp_msg->tpa, sizeof(arp_msg->tpa));
                    arp_msg->op = htons(ARPOP_REPLY);

                    // send
                    send_eth_to_mac(
                        arp_msg, 
                        ETH_P_ARP, 
                        arp_msg->tha, 
                        sizeof(ar_t), 
                        interface
                    );

                    return 0;
                }
            }
        }
    }

    return -1;
}

void broadcast_arp(uint32_t target, iface_t interface)
{
    // Build ARP payload
    ar_t arp_msg;
    ar_t *ar = &arp_msg;
    ar->hrd = htons(ARPHRD_ETHER);
    ar->pro = htons(ETH_P_IP);
    ar->hln = ETH_ALEN;
    ar->pln = sizeof(struct in_addr);
    ar->op = htons(ARPOP_REQUEST);
    memcpy(ar->sha, interface.src_mac, 6);
    ar->spa = htonl(interface.src_ip);
    ar->tpa = htonl(target);

    uint8_t broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    send_eth_to_mac(
        ar,
        ETH_P_ARP,
        broadcast_addr,
        sizeof(ar_t),
        interface
    );
}

void clear_cache()
{
    arp_cache.size = 0;
    memset(arp_cache.data, 0, sizeof(arp_cache.data));
}