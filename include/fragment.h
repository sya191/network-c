#define FRAG_SIZ 65536
#include "ip.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Module to handle fragmenting of IPv4 datagrams
 */

typedef struct {
    uint16_t *data; // store offset ranges {{0, 5}, {6, 10}} (contiguous layout for cache perf)
    int size;
    int capacity;
} offsets_t;

typedef struct {
    uint8_t *data; // raw byte data of assemebled bytes
    int needed; // total bytes needed
    int size; // total bytes assembled
    int capacity; // size of heap buffer
} reassm_buf_t;

typedef struct {
    bool tracking; // Currently tracking flag
    uint16_t ID; // Fragment ID
    offsets_t offsets; // see offsets_t
    reassm_buf_t bytes;
    double timestamp; // monotonic timestamp
} fragments_t;

/**
 * @returns true if IPv4 datagram is a fragment
 */
bool is_fragment(ip_t *frame);

/**
 * Adds fragment payload to internal data structure
 * 
 * If payload is fully assembled, sends payload up the stack using ip.h module
 */
int add_fragment(ip_t *frame, iface_t *interface);

bool offsets_overlap(int src0[2], int src1[2]);

/**
 * Purges timed out fragments
 */
int purge();

void free_frags();

void extract_offset(int *dst, ip_t *frame);