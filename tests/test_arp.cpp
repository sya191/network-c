#include <gtest/gtest.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
extern "C" {
#include "ethernet.h"
#include "arp.h"
#include "interface.h"
}

class ARPTest: public ::testing::Test {
protected:
    void SetUp() override {
        int fd = open("./", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror("open()");
            exit(EXIT_FAILURE);
        }
        test_fd = fd;
    }

    void TearDown() override {
    }

    int test_fd;
    uint8_t src_mac[6] = {0x1, 0x0, 0x0, 0x0, 0x0, 0x1};
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char src_protocol[10] = "192.1.1.1";
};

TEST_F(ARPTest, recvArpTest) {
    int size = sizeof(eth_hdr_t) + sizeof(ar_t);
    char buf[size];

    eth_hdr_t *eth_hdr = (eth_hdr_t *)buf;
    ar_t *arp = (ar_t *)(eth_hdr + 1);

    // format ethernet header
    memcpy(eth_hdr->mac_src, src_mac, 6);
    memcpy(eth_hdr->mac_dest, broadcast_mac, 6);
    eth_hdr->ethertype = htons(ETH_P_ARP);

    // format arp message
    arp->hrd = htons(1);
    arp->pro = htons(ETH_P_IP);
    arp->hln = 6; // hardware proto. length
    arp->pln = 4; // protocol length (IPv4 = 4)
    arp->op = htons(1); // ARP REQUEST
    memcpy(arp->sha, src_mac, 6);
    inet_pton(AF_INET, src_protocol, &arp->spa);
    arp->tpa = htonl(interface_ip()); // target is us

    // Should return 0 as the ARP is for our interface IP
    EXPECT_EQ(recv_arp(buf, test_fd, write), 0);
}