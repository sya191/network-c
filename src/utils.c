#include "utils.h"
#include <string.h>
#include <stdio.h>

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