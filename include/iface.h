#pragma once
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Struct including all parameters to write to a defined interface
 */
typedef struct {
    ssize_t (*write)(int fd, const void *buf, size_t len); // function pointer to a defined write function
    ssize_t (*read)(int fd, void *buf); // should read exactly one ethernet frame per call
    int fd; // file descriptor to write to
    uint32_t src_ip;
    uint8_t src_mac[6]; // make sure the LSB of the first byte is 0 for unicast address
} iface_t;