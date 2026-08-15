#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

// the test functions below should mimic a NIC read/write interface
// i.e. they read and write at most one frame per time
// no need to worry about multiple reads/writes in succession
// use case: read, then write. or write, then read

ssize_t test_read(int fd, void *buf) 
{
    ssize_t bytes = read(fd, buf, 1500);
    if (bytes < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    // delete all file contents (i.e. truncate file to 0 bytes)
    if (ftruncate(fd, 0) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    return bytes;
}

ssize_t test_write(int fd, const void *buf, size_t len)
{
    ssize_t bytes = write(fd, buf, len);
    // move read/write head to start of file
    if (lseek(fd, SEEK_SET, 0) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    return bytes;
}

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