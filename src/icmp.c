#include "icmp.h"
#include "ip.h"
#include "checksum.h"
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

static void echo_reply()
{

}

/**
 * len = total length of data
 */
void echo_request(
    uint32_t target_ip, 
    uint16_t seq, 
    void *data, 
    uint16_t len, 
    iface_t *interface)
{
    // create buf
    uint32_t size = sizeof(icmp_t) + sizeof(echo_hdr_t) + len;
    uint8_t buf[size];
    uint16_t pid = (uint16_t)getpid();
    // mimic UNIX behaviour of using pid in ident
    icmp_t *icmp = (icmp_t *)buf;
    icmp->type = 8; // ECHO REQUEST
    icmp->code = 0; // ECHO
    echo_hdr_t *echo = (echo_hdr_t *)(icmp + 1);
    echo->ident = htons(pid);
    echo->seq = htons(seq);
    void *payload = echo + 1;
    memcpy(payload, data, len);
    // checksum over entire icmp frame
    icmp->checksum = 0;
    // no need to convert because 16-bit words for checksum calc is already in network order
    icmp->checksum = checksum(buf, size); 

    // TODO: send to IP
    send_ip(target_ip, buf, IPPROTO_ICMP, size, interface);
}

int send_echo(echo_t type, iface_t *interface)
{
    switch (type) {
        case ECHO_REPLY:
            break;
        case ECHO_REQUEST:
            break;
    }

    return 0;
}

int recv_echo(void *payload) 
{
    return 0;
}