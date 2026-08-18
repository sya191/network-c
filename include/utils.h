#include <stdlib.h>
#include <stdint.h>
// the test functions below should mimic a NIC read/write interface
// i.e. they read and write at most one frame per time
// no need to worry about multiple reads/writes in succession
// use case: read, then write. or write, then read
ssize_t test_read(int fd, void *buf);
ssize_t test_write(int fd, const void *buf, size_t len);
void swap(void *src, void *dest, size_t size);
uint32_t convert_ip(const char *src);

/**
 * @returns double precision float of current timestamp (MONOTONIC)
 * 
 * @note <seconds.milliseconds>
 */
double get_timestamp();