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

uint32_t convert_ip(const char *src)
{
    uint32_t dest;
    if (inet_pton(AF_INET, src, &dest) < 1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }
    return ntohl(dest);
}