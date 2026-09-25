#include "icmp.h"
#include "ip.h"
#include "checksum.h"
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

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

/**
 * src_ip = who sent the echo
 */
void echo_reply(
    uint32_t src_ip, 
    echo_hdr_t *hdr, // NETWORK ORDER
    void *data, 
    uint16_t len, 
    iface_t *interface)
{
    // create buf
    uint32_t size = sizeof(icmp_t) + sizeof(echo_hdr_t) + len;
    uint8_t buf[size];
    icmp_t *icmp = (icmp_t *)buf;
    icmp->type = 0; // ECHO REPLY
    icmp->code = 0; // ECHO
    echo_hdr_t *echo = (echo_hdr_t *)(icmp + 1);
    echo->ident = hdr->ident;
    echo->seq = hdr->seq;
    void *payload = echo + 1;
    memcpy(payload, data, len);
    // checksum over entire icmp frame
    icmp->checksum = 0;
    // no need to convert because 16-bit words for checksum calc is already in network order
    icmp->checksum = checksum(buf, size); 

    // TODO: send to IP
    send_ip(src_ip, buf, IPPROTO_ICMP, size, interface);
}

// len = size of payload not including icmp header
int recv_icmp(uint32_t src_ip, icmp_t *data, size_t len, iface_t *interface)
{
    if (checksum(data, len + sizeof(icmp_t)) != 0) {
        return -1;
    }

    // echo request
    if (data->code == 0 && data->type == 8) {
        void *payload = (void *)data + sizeof(icmp_t) + sizeof(echo_hdr_t);
        echo_hdr_t *hdr = (void *)data + sizeof(icmp_t);
        echo_reply(src_ip, hdr, payload, len - sizeof(echo_hdr_t), interface);
    }


    return 0;
}
