#pragma once
#define DEFAULT_ENTRIES_SIZ 4 // we just need 3 entries for now (default gateway, subnet, broadcast)
#include "iface.h"
#include <stdint.h>

/**
 * Standard IPv4 header
 */
#pragma pack(push, 1)

typedef struct {
    uint8_t ver_IHL; // ver (4-bit) + Internet Header Length/# of 32-bit words in the header (4-bit)
    uint8_t DSField_ECN; // DSField (6-bit) + ECN (2-bit)
    uint16_t total_len; // Total length of IPv4 datagram in bytes (16-bit)
    uint16_t identification; // 16-bit
    uint16_t flags_offset; // Flags (3-bit) + Fragment Offset (13-bit)
    uint8_t TTL; // 8-bit
    uint8_t protocol; // 8-bit
    uint16_t checksum; // Header checksum (16-bit)
    uint32_t src_addr; // Source IP address (32-bit)
    uint32_t dest_addr; // Destination IP address (32-bit)
} ip_t;

#pragma pack(pop)

typedef enum {
    ETHERNET,
    LOOPBACK // for simplicity no need to implement for now
} interface_t;

typedef struct {
    uint32_t dest; // Destination
    uint32_t mask; // Mask
    uint32_t next_hop; // Next-hop
    interface_t interface; // ETHERNET pretty much always
} routing_ent_t;

/**
 * Routing table for IP module
 * 
 * Defaults to size 4 routing entries
 */
typedef struct {
    routing_ent_t entries[DEFAULT_ENTRIES_SIZ]; // default = 4
    int size;
} routing_table;

// For configuration (DHCP needed for automatic population, for now just manual)
int delete_routing_entry(uint32_t dest);
int add_routing_entry(routing_ent_t entry);

/**
 * Demux function which hands payload to modules above
 * 
 * @returns protocol code (TCP, ICMP, etc.) on success, -1 on failure
 */
int recv_ip(ip_t *ip_msg, iface_t *interface);

/**
 * Attempts to send IP packet (may fragment)
 * 
 * @returns 0 on success, -1 on failure (upper layer modules must decide what to do)
 */
int send_ip(
    void *payload,
    uint8_t *protocol,
    size_t len,
    iface_t *interface);

/**
 * Pushes a generic payload up the stack
 */
void push_up_stack(uint8_t protocol, void *payload, size_t len, iface_t *interface);
