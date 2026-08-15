#include "iface.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
/**
 * Ethernet header format
 */

#pragma pack(push, 1)

typedef struct {
    uint8_t mac_dest[6];
    uint8_t mac_src[6];
    uint16_t ethertype; // in Big-Endian
} eth_hdr_t;

#pragma pack(pop)

/**
 * Reads an ethernet frame from the defined interface and handles the payload accordingly
 * 
 * @param interface @see iface.h
 * 
 * @returns the ethertype handled on success, otherwise -1 on read failure
 */
int recv_eth(iface_t interface);

/**
 * Adds ethernet header and sends payload to an IPv4 address
 * 
 * @note if the target_ip address does not have an associated MAC, it will send an ARP REQUEST
 * 
 * @param payload the payload address
 * @param target_ip target IPv4 address in host order
 * @param len size of payload in bytes
 * @param interface @see iface.h
 * 
 * @returns 0 on success, -1 on fail (due to unavailable MAC translation)
 */
int send_eth_to_ip(
    void *payload, 
    uint16_t ethertype,
    uint32_t target_ip,
    size_t len,
    iface_t interface);

int send_eth_to_mac(
    void *payload,
    uint16_t ethertype,
    uint8_t target_mac[6],
    size_t len,
    iface_t interface);