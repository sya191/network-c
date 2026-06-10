#include <stdint.h>

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
 * Big switch case to handle each ethertype
 * 
 * @param addr the ethernet frame address (usually in a buffer).
 * @param fd the network device to write to.
 */
int handle_eth(void *addr, int fd);