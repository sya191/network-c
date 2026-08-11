#pragma once
#include <sys/types.h>
#include <stddef.h>

/**
 * Struct including all parameters to write to a defined interface
 */
typedef struct {
    ssize_t (*write_interface)(int fd, const void *buf, size_t len); // function pointer to a defined write function
    int fd; // file descriptor to write to
} iface_t;