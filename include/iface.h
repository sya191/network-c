#pragma once
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Struct including all parameters to write to a defined interface
 */
typedef struct {
    ssize_t (*write_interface)(int fd, const void *buf, size_t len); // function pointer to a defined write function
    int fd; // file descriptor to write to
    uint32_t src_ip;
    uint8_t src_mac[6];
} iface_t;