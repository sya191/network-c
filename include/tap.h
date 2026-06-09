#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <linux/if.h>
#include <stdint.h>

typedef struct {
    int tap_fd;
    char dev[IFNAMSIZ];
    uint8_t mac[6];
    uint32_t ip;
    bool start;
} netdev_t; // network device

/**
 * Sets up a TAP device.
 * 
 * If TAP device has already been set up, then it returns the associated tap fd.
 * 
 * Stores configuration information in struct netdev.
 * 
 * @returns fd associated with TAP device
 */
int netdev_create();

/**
 * Copies TAP device name to dest.
 * 
 * @param dest MUST have buffer size of at least 16!
 */
void netdev_name(char *dest);

/**
 * Updates the state of the tap interface to be UP
 * 
 * Allows TAP device to start processing traffic
 */
int netdev_start();