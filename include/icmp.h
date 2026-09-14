#include "iface.h"
#include <stdint.h>

typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum; // covers the entire icmp message (not including IP)
} icmp_t;

typedef struct {
    uint16_t ident;
    uint16_t seq;
} echo_hdr_t;

typedef enum {
    ECHO_REPLY,
    ECHO_REQUEST
} echo_t;

// TODO: Implement the most used type/code 
/*
Echo Reply
Destination Unreachable
Echo
Time Exceeded
Parameter Problem
*/

int send_echo(echo_t type, iface_t *interface);
int recv_echo(void *payload);
void echo_request(
    uint32_t target_ip, 
    uint16_t seq, 
    void *data, 
    uint16_t len,
    iface_t *interface);