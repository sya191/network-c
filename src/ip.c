#define FRAG_SIZ 65536
#define TEST_MACRO 254
#include "ip.h"
#include "utils.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>

void push_up_stack(uint8_t protocol, void *payload, size_t len, iface_t *interface)
{
    switch (protocol) {
        case IPPROTO_IP:
            break;
        case IPPROTO_ICMP:
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
   return 0;
}