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
 * Big ass switch case to handle each ethertype
 */
int handle_eth(void *addr);