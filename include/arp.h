#define ARPCACHEMAX 256
#include <stdint.h>

/**
 * Defines the arp body structure
 * 
 * Naming scheme is taken directly from RFC826
 * 
 * For simplicity, we assume protocol addr is 4 bytes in length and hw is 6 bytes.
 */
#pragma pack(push, 1)

typedef struct {
    uint16_t hrd; // Hardware address space
    uint16_t pro; // Protocol address space
    uint8_t hln; // Byte length of each hardware address
    uint8_t pln; // Byte length of each protocol address
    uint16_t op; // Opcode (REQUEST OR REPLY)
    uint8_t sha[6]; // Source MAC
    uint32_t spa; // Source protocol address
    uint8_t tha[6]; // Target MAC (if known) 
    uint32_t tpa; // Target protocol address 
} ar_t;

#pragma pack(pop)

/**
 * Key value pair consisting of ip and mac address
 */
typedef struct {
    uint32_t ip;
    uint8_t mac[6];
} ip_mac_t;

/**
 * Data structure to store ip to mac translations
 */
typedef struct {
    unsigned int size;
    ip_mac_t data[ARPCACHEMAX];
} arp_cache_t;

/**
 * Populates <dest> with MAC address if found.
 * 
 * This function does a linear scan through the ARP cache for simplicity.
 * 
 * @returns 0 if mac is found. -1 if mac is not found.
 */
int lookup_ip(uint8_t dest[6], uint32_t ip);

/**
 * As a packet is sent down through the network layers, we need to resolve protocol address (IPv4)
 * to hardware address (MAC).
 * 
 * Two things can happen:
 * - We have the MAC address in our table -> translate the protoc addr to hw and send
 * - We don't have the MAC address in our table -> drop the packet and send ARP request to update table
 */
int recv_arp(void *payload, int fd);

/**
 * Broadcasts an ARP message via the broadcast MAC address (FF:FF:FF:FF:FF:FF)
 * 
 * @param target - the IP address we wish to know the MAC for
 * @param fd - file descriptor to a network device
 */
int broadcast_arp(uint32_t target, int fd);