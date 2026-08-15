#include <sys/types.h>
#include <stdint.h>

/**
 * Sets up a socket using AF_PACKET & SOCK_RAW
 * 
 * @returns socket fd
 */
int create_interface();


/**
 * Writes to the interface
 */
ssize_t write_interface(int fd, const void *buf, size_t len);


/**
 * Reads from the interface
 */
ssize_t read_interface(int fd, void *buf);