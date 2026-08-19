#define FRAG_SIZ 65536
#include "ip.h"
#include "utils.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Need data structure to store fragmented IP datagrams
 * 
 * > Simple array of structs? 
 * Each entry would track: 
 *  - Currently tracking flag (boolean)
 *  - ID (uint16) => maximum 65536 values so 64KB array for O(1) map
 *  - bytes assembled 
 *  - total bytes needed (only known on last fragement)
 *  - dynamically sized array of offsets assembled {0, 6, 10}, ...}
 * send packet up to higher layer only when bytes assembled = total bytes needed
 * > Need to support time-out of reassembly (purge timedout fxn, call every time on recv_ip)
 */

typedef struct {
    uint16_t *data; // store offset ranges {{0, 5}, {6, 10}} (contiguous layout for cache perf)
    int size;
    int capacity;
} offsets_t;

typedef struct {
    uint8_t *data; // raw byte data of assemebled bytes
    int size;
    int capacity;
} reassm_buf_t;

typedef struct {
    bool tracking; // Currently tracking flag
    uint16_t ID; // Fragment ID
    size_t assembled; // bytes assembled so far
    ssize_t needed; // total bytes needed
    offsets_t offsets; // see offsets_t
    reassm_buf_t bytes;
    double timestamp; // monotonic timestamp
} fragments_t;

static fragments_t fragments[FRAG_SIZ] = {0};

static void purge_timedout()
{
    for (int i = 0; i < FRAG_SIZ; ++i) {
        if (!fragments[i].tracking) {
            continue;
        }
        double diff = get_timestamp() - fragments[i].timestamp;
        // purge if more than 30 seconds from first fragment
        if (diff > 30) {
            free_fragment(i);
        }
    }
}

static bool is_fragment(ip_t *hdr)
{
    // check MF bit & fragment offset
    uint16_t flags_offset = ntohs(hdr->flags_offset);
    uint8_t MF = (flags_offset >> 13) & 0x1;
    uint16_t offset = flags_offset & 0x1fff;

    return (MF != 0 || offset != 0);
}

static int add_offset(uint16_t ident, int start, int end) 
{
    fragments_t *frag = &fragments[ident];
    offsets_t *offsets = &frag->offsets;
    if (offsets->capacity == 0) {

    } else if (offsets->capacity <= offsets->size + 1) {
        int new_cap = offsets->capacity * 2;
        offsets->capacity = new_cap;
        offsets->data = realloc(offsets->data, new_cap * sizeof(uint16_t) * 2);
        if (offsets->data == NULL) {
            perror("realloc()");
            exit(EXIT_FAILURE);
        }
    }
    
    // add to end
    offsets->data[offsets->size * 2] = start;
    offsets->data[offsets->size * 2 + 1] = end;
}

static int copy_bytes(uint16_t ident, void *payload, int start, int len)
{
    fragments_t *frag = &fragments[ident];
    reassm_buf_t *reassembly = &frag->bytes;
    if (reassembly->capacity == 0) {
        // initalize buffer
        reassembly->data = malloc(3000);
        reassembly->capacity = 3000;
    }
    // this is wrong, need to bounds check the array instead of size check TODO
    if (reassembly->capacity < reassembly->size + len) {
        // increase buffer size by at least double required
        int new_cap = (reassembly->size + len) * 2;
        reassembly->data = realloc(reassembly->data, new_cap);
        if (reassembly->data == NULL) {
            perror("realloc()");
            exit(EXIT_FAILURE);
        }
        reassembly->capacity = new_cap;
    }
    // copy bytes
    memcpy(reassembly->data[start], payload, len);
    reassembly->size += len;
}

static int get_offset(uint16_t ident, int idx, int buf[2])
{
    fragments_t *frag = &fragments[ident];
    offsets_t *offsets = &frag->offsets;

    if (offsets->size - 1 < idx) {
        return -1;
    }

    buf[0] = offsets->data[idx * 2];
    buf[1] = offsets->data[idx * 2 + 1];

    return 0;
}

static bool range_overlaps(int s0, int e0, int s1, int e1)
{
    // derivation:
    // range fails to overlap if one range ends before the other starts
    // p = range 0 ends before range 1 starts (s1 > e1)
    // q = range 1 ends before range 0 starts (s0 > e1)
    // !(p OR q) = !p AND !q
    return s0 <= e1 && s1 <= e0;
}

static void free_fragment(uint16_t idx)
{
    if (fragments[idx].offsets.data != NULL) {
        free(fragments[idx].offsets.data);
    }
    if (fragments[idx].bytes.data != NULL) {
        free(fragments[idx].bytes.data);
    }
    memset(&fragments[idx], 0, sizeof(fragments_t));
}

// pass whole frame to this
static void add_fragment(ip_t *frame)
{
    uint16_t idx = htons(frame->identification);
    uint16_t flags_offset = htons(frame->flags_offset);
    // flag fragment ident as being tracked
    fragments[idx].tracking = true;

    // calculate start and stop offsets (not including ip header)
    uint16_t start, stop;
    start = (flags_offset & 0x1fff) * 8;
    stop = start + frame->total_len - sizeof(ip_t);

    fragments_t *frag = &fragments[idx];
    offsets_t *offsets = &frag->offsets;

    // check if offset range is in array
    for (int i = 0; i < offsets->size; ++i) {
        int range[2];
        if (get_offset(idx, i, range) == -1) {
            exit(EXIT_FAILURE);
        }
        if (range_overlaps(range[0], range[1], start, stop)) {
            // data may be malliciously sent
            free_fragment(idx);
            return;
        }
    }

    add_offset(idx, start, stop);

    // copy bytes to reassembly buffer
}

int recv_ip()
{
    purge_timedout();
    return 0;
}