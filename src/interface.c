#include "interface.h"
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <linux/if_arp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void interface_mac(uint8_t *dest)
{
    uint8_t mac[6] = MAC;
    memcpy(dest, mac, sizeof(mac));
}

uint32_t interface_ip()
{
    uint32_t dest;
    if (inet_pton(AF_INET, IPADDR, &dest) < 1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }
    return ntohl(dest);
}

int create_interface()
{
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    struct packet_mreq mr = {0};
    mr.mr_ifindex = if_nametoindex("enp0s3");
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }

    return fd;
}

ssize_t write_interface(int fd, const void *buf, size_t len)
{
    struct sockaddr_ll sock_struct = {
        .sll_family = AF_PACKET,
        .sll_protocol = htons(ETH_P_ALL),
        .sll_ifindex = if_nametoindex("enp0s3"),
    };

    return sendto(fd, buf, len, 0, (struct sockaddr *)&sock_struct, sizeof(sock_struct));
}

ssize_t recv_interface(int fd, char *buf, size_t len)
{
    return read(fd, buf, len);
}
