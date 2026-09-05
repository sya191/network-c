#include "fragment.h"
#include "utils.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

fragments_t frags[FRAG_SIZ] = {0};

bool is_fragment(ip_t *frame)
{
    // check MF bit & fragment offset
    uint16_t flags_offset = ntohs(frame->flags_offset);
    uint8_t MF = (flags_offset >> 13) & 0x1;
    uint16_t offset = flags_offset & 0x1fff;

    return (MF != 0 || offset != 0);
}

void free_fragment(int ident)
{
    fragments_t *frag = &frags[ident];
    if (frag->bytes.data != NULL) {
        free(frag->bytes.data);
    }
    if (frag->offsets.data != NULL) {
        free(frag->offsets.data);
    }
    // set all bits to 0 (tracking is false)
    memset(frag, 0, sizeof(*frag));
}

void get_offset(int *dst, int ident, int idx)
{
    if (!frags[ident].tracking) {
        return;
    }
    if (frags[ident].offsets.size - 1 < idx) {
        perror("OFFSET OUT OF BOUNDS");
        exit(EXIT_FAILURE);
    }
    dst[0] = frags[ident].offsets.data[idx * 2];
    dst[1] = frags[ident].offsets.data[idx * 2 + 1];
}

void add_offset(int *src, int ident)
{
    fragments_t *frag = &frags[ident];
    offsets_t *offsets = &frag->offsets;
    // init + resize if needed
    if (offsets->capacity == 0) {
        offsets->capacity = 10;
        offsets->size = 0;
        offsets->data = malloc(sizeof(int) * 2 * 10);
        if (offsets->data == NULL) {
            perror("malloc()");
            exit(EXIT_FAILURE);
        }
    } else if (offsets->capacity <= offsets->size) {
        offsets->capacity *= 2;
        offsets->data = realloc(
            offsets->data, 
            offsets->capacity * sizeof(int) * 2
        );
        if (offsets->data == NULL) {
            perror("realloc()");
            exit(EXIT_FAILURE);
        }
    }
    // add offsets
    offsets->data[offsets->size * 2] = src[0];
    offsets->data[offsets->size * 2 + 1] = src[1];
    offsets->size++;
}

void extract_offset(int *dst, ip_t *frame)
{
    uint16_t flags_offset = ntohs(frame->flags_offset);
    uint16_t offset = flags_offset & 0x1fff;
    uint16_t len = ntohs(frame->total_len);
    dst[0] = offset * 8; // offset field is in multiples of 8
    dst[1] = dst[0] + len - sizeof(ip_t) - 1; // minus 1 because 0 indexing
}

bool offsets_overlap(int src0[2], int src1[2])
{
    // they overlap if one starts before the other stops
    return (src0[0] <= src1[1]) && (src1[0] <= src0[1]);
}

bool offset_seen(int src[2], int ident)
{
    offsets_t *offsets = &frags[ident].offsets;
    for (int i = 0; i < offsets->size; ++i) {
        int src1[2];
        get_offset(src1, ident, i);
        if (offsets_overlap(src, src1)) {
            return true;
        }
    }
    return false;
}

void add_bytes(int offsets[2], ip_t *frame)
{
    int ident = ntohs(frame->identification);
    reassm_buf_t *bytes = &frags[ident].bytes;
    // init
    if (bytes->capacity <= bytes->size) {
        bytes->capacity = 1000;
        bytes->needed = -1; // invalid total needed until last fragment is seen
        bytes->size = 0;
        bytes->data = malloc(1000);
        if (bytes->data == NULL) {
            perror("malloc()");
            exit(EXIT_FAILURE);
        }
    }
    // bounds check and resize if needed
    if (bytes->capacity - 1 < offsets[1]) {
        // resize by double required
        bytes->capacity = offsets[1] * 2;
        bytes->data = realloc(bytes->data, bytes->capacity);
        if (bytes->data == NULL) {
            perror("realloc()");
            exit(EXIT_FAILURE);
        }
    }
    // copy bytes
    void *payload = frame + 1;
    int len = offsets[1] - offsets[0] + 1;
    memcpy(&bytes->data[offsets[0]], payload, len);
    bytes->size += len;
}

bool is_last_fragment(ip_t *frame)
{
    uint16_t flags_offset = ntohs(frame->flags_offset);
    uint8_t MF = (flags_offset >> 13) & 0x1;
    uint16_t offset = flags_offset & 0x1fff;
    return (offset != 0 && MF == 0);
}

int add_fragment(ip_t *frame, iface_t *interface)
{
    int ident = ntohs(frame->identification);
    fragments_t *frag = &frags[ident];
    frag->tracking = true;

    // check if range of bytes has already been seen
    int offsets[2];
    extract_offset(offsets, frame);
    if (offset_seen(offsets, ident)) {
        // drop all fragments if seen
        free_fragment(ident);
        return 0;
    }
    add_offset(offsets, ident);
    add_bytes(offsets, frame);
    // check if last fragment, if so update bytes needed
    if (is_last_fragment(frame)) {
        frag->bytes.needed = offsets[1] + 1;
    }
    // check if all bytes are copied, if so send up stack
    if (frag->bytes.needed == frag->bytes.size) {
        uint8_t protocol = frame->protocol;
        push_up_stack(protocol, frag->bytes.data, frag->bytes.size, interface);
        free_fragment(ident);
        return protocol;
    }
    frag->timestamp = get_timestamp();
    return 0;
}

void free_frags() 
{
    for (int i = 0; i < FRAG_SIZ; ++i) {
        free_fragment(i);
    }
}