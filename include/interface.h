#define IPADDR "192.168.1.123"
// #define MAC {0x08, 0x00, 0x27, 0x63, 0x31, 0x63}
#define MAC {0x02, 0x00, 0x00, 0x00, 0x00, 0x01}
#include <sys/types.h>
#include <stdint.h>

/**
 * @returns the interface ip in host byte order
 */
uint32_t interface_ip();

/**
 * Sets up a socket using AF_PACKET & SOCK_RAW
 * 
 * @returns socket fd
 */
int create_interface();


/**
 * Writes to the interface
 */
ssize_t write_interface(int fd, char *buf, size_t len);


/**
 * Reads from the interface
 */
ssize_t recv_interface(int fd, char *buf, size_t len);

/**
 * Writes the mac address of the interface into dest (6 bytes)
 */
void interface_mac(uint8_t *dest);