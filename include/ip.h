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

/**
 * Demux function which handles ICMP/TCP/etc.
 */
int handle_ip();