#define FRAG_SIZ 65536
#define TEST_MACRO 254
#include "ip.h"
#include "utils.h"
#include "checksum.h"
#include "ethernet.h"
#include "icmp.h"
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>

// len = size of payload not including header
void push_up_stack(uint8_t protocol, void *payload, size_t len, iface_t *interface)
{
    switch (protocol) {
        case IPPROTO_IP:
            break;
        case IPPROTO_ICMP:
            icmp_t *hdr = payload + sizeof(ip_t);
            uint32_t src_ip = ntohl(((ip_t *)payload)->src_addr);
            recv_icmp(src_ip, hdr, len - sizeof(icmp_t), interface);
            break;
        case IPPROTO_IGMP:
            break;
        case IPPROTO_IPIP:
            break;
        case IPPROTO_TCP:
            break;
        case IPPROTO_UDP:
            break;
        case IPPROTO_IPV6:
            break;
        case IPPROTO_RAW:
            break;
        case TEST_MACRO:
            // dump payload to fd
            test_write(interface->fd, payload, len);
            break;
    }
}

int recv_ip(ip_t *ip_msg, iface_t *interface)
{
    if (checksum(ip_msg, sizeof(ip_t)) != 0) {
        printf("Checksum failed: %x\n", checksum(ip_msg, sizeof(ip_t)));
        return -1;
    }

    size_t len = ntohs(ip_msg->total_len) - sizeof(ip_t);
    printf("Recieved payload of size %d\n", len);

    push_up_stack(ip_msg->protocol, ip_msg, len, interface);

    return 0;
}

static int send_fragments(
    uint32_t dest_addr,
    void *payload,
    uint8_t protocol, 
    size_t len,
    iface_t *interface)
{
    // divide payload up into len / MTU (1500) fragments
    int num_frags = len / 1500;
    for (int i = 0; i < num_frags; i++) {
        // TODO:
        // repeated code from send_ip
        // remember flags_offset ident needs to actually be set here
    }
}

int send_ip(
    uint32_t dest_addr,
    void *payload,
    uint8_t protocol, 
    size_t len,
    iface_t *interface)
{
    // check if packet is larger than MTU
    // assume that MTU is 1500 for ethernet
    if (len > 1500) {
        printf("TOO LARGE FRAGMENTING: SIZE [%d]\n", len);
        return send_fragments(
            dest_addr, 
            payload, 
            protocol, 
            len, 
            interface
        );
    }
    // build header
    uint16_t total_len = sizeof(ip_t) + len;
    uint8_t buf[total_len];
    ip_t *hdr = (ip_t *)buf;
    hdr->checksum = htons(0); // 0 for now
    hdr->dest_addr = htonl(dest_addr);
    hdr->DSField_ECN = 0;
    hdr->flags_offset = htons(0);
    hdr->identification = htons(0);
    hdr->protocol = protocol;
    hdr->src_addr = htonl(interface->src_ip);
    hdr->total_len = htons(total_len);
    hdr->TTL = 64; // default
    hdr->ver_IHL = 0x45; // IPv4 + min header length (20 bytes)
    // calc hdr checksum (don't convert because previous fields are already in network byte order)
    hdr->checksum = checksum(buf, sizeof(ip_t));
    // copy payload
    memcpy(buf + sizeof(ip_t), payload, len);

    // send to ethernet module
    return send_eth_to_ip(
        buf, 
        ETH_P_IP, 
        dest_addr, 
        total_len, 
        interface
    );
}