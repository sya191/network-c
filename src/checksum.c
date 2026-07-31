#include "checksum.h"

uint16_t checksum(void *addr, size_t size)
{
    uint16_t *hdr = (uint16_t *)addr;
    uint32_t sum = 0; // twos complement sum (extra space at MS bits)

    for (size_t i = 0; i < size / 2; ++i) {
        uint16_t word = hdr[i];
        sum += word;
    }

    // zero pad last byte if odd size
    if (size % 2 != 0) {
        uint16_t last_byte = ((uint8_t *)addr)[size - 1];
        // shift byte left by 8 because C pads from LSB
        sum += last_byte << 8;
    }

    // fold until no carry over bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    uint16_t checksum = sum;

    return ~checksum;
}