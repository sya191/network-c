#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Computes the internet checksum
 * @param addr start address of bytes
 * @param size in bytes 
 * 
 * @returns a 2-byte checksum
 */
uint16_t checksum(void *addr, size_t size);