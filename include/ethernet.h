#include <stdint.h>

/**
 * Ethernet header format
 */

#pragma pack(push, 1)

struct eth_hdr {
    uint8_t mac_dest[6];
    uint8_t mac_src[6];
    uint16_t ethertype;
};

#pragma pack(pop)