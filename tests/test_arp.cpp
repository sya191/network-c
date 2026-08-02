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
        close(test_fd);
        clear_cache();
    }

    int test_fd;
    uint8_t src_mac[6] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6};
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char src_protocol[10] = "192.1.1.1";
};

TEST_F(ARPTest, recvArpForUsTest) {
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
    ASSERT_EQ(recv_arp(buf, test_fd, write), 0);

    // check if ARP cache has cached IP addr
    uint8_t mac_value[6];
    uint32_t src_prot_network;
    inet_pton(AF_INET, src_protocol, &src_prot_network);
    src_prot_network = ntohl(src_prot_network);
    ASSERT_EQ(lookup_ip(mac_value, src_prot_network), 0);

    // check if ARP cache has correct IP -> MAC translation
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(mac_value[i], i + 1);
    }
}

TEST_F(ARPTest, recvArpNotForUsTest) {
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
    arp->tpa = htonl(0); // target is NOT us

    // Should return -1 as the ARP is not for our IP addr
    ASSERT_EQ(recv_arp(buf, test_fd, write), -1);

    // check if ARP cache has cached IP addr (should not because we've never talked)
    uint8_t mac_value[6];
    uint32_t src_prot_network;
    inet_pton(AF_INET, src_protocol, &src_prot_network);
    src_prot_network = ntohl(src_prot_network);
    ASSERT_EQ(lookup_ip(mac_value, src_prot_network), -1);
}