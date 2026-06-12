#define IPADDR "192.168.1.123"
#define MAC {2, 0, 0, 0, 0, 1}
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

void interface_mac(uint8_t *dest);