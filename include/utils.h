#include <stdlib.h>
#include <stdint.h>

void swap(void *src, void *dest, size_t size);
uint32_t convert_ip(char *src);
/**
 * @brief Computes the internet checksum
 * @param words the number of 16-bit words in the header
 * @param size size of the header/hdr + payload
 * 
 * @returns a 2-byte checksum
 */
uint16_t checksum(void *addr, size_t size);