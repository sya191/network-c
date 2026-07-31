#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

void swap(void *src, void *dest, size_t size)
{
    void *tmp = malloc(size);
    if (tmp == NULL) {
        perror("malloc()");
        exit(EXIT_FAILURE);
    }
    memcpy(tmp, dest, size);
    memcpy(dest, src, size);
    memcpy(src, tmp, size);
    free(tmp);
    tmp = NULL;
}

uint32_t convert_ip(char *src)
{
    uint32_t dest;
    if (inet_pton(AF_INET, src, &dest) < 1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }
    return ntohl(dest);
}

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
        sum += last_byte;
    }

    // fold until no carry over bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    uint16_t checksum = sum;

    return ~checksum;
}