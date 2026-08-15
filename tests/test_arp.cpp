#include <gtest/gtest.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
extern "C" {
#include "ethernet.h"
#include "arp.h"
#include "interface.h"
#include "iface.h"
#include "utils.h"
}

class ARPTest: public ::testing::Test {
protected:
    void SetUp() override {
        int fd = open("./", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror("open()");
            exit(EXIT_FAILURE);
        }
        interface = {
            .write = write,
            .fd = fd,
            .src_ip = convert_ip("192.168.1.104"),  
            .src_mac = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6},
        };
    }

    void TearDown() override {
        close(interface.fd);
        clear_cache();
    }

    iface_t interface;
    uint8_t src_mac[6] = {0x6, 0x5, 0x4, 0x3, 0x2, 0x1};
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char src_protocol[10] = "192.1.1.1"; // sender not our interface
};

/**
 * Test whether an ARP for our interface IP correctly caches
 */
TEST_F(ARPTest, recvArpCacheTest) {
    ar_t ar;
    ar_t *arp = &ar; 

    // format arp message
    arp->hrd = htons(1);
    arp->pro = htons(ETH_P_IP);
    arp->hln = 6; // hardware proto. length
    arp->pln = 4; // protocol length (IPv4 = 4)
    arp->op = htons(ARPOP_REQUEST); // ARP REQUEST
    memcpy(arp->sha, src_mac, 6);
    inet_pton(AF_INET, src_protocol, &arp->spa);
    arp->tpa = htonl(convert_ip("192.168.1.104")); // target is us

    // Should return 0 as the ARP is for our interface IP
    ASSERT_EQ(recv_arp(arp, interface), 0);

    // check if ARP cache has cached IP addr
    uint8_t mac_value[6];
    uint32_t src_prot_network;
    inet_pton(AF_INET, src_protocol, &src_prot_network);
    src_prot_network = ntohl(src_prot_network);
    ASSERT_EQ(lookup_ip(mac_value, src_prot_network), 0);

    // check if ARP cache has correct IP -> MAC translation
    // Should be {0x6, 0x5, 0x4, 0x3, 0x2, 0x1}
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(mac_value[i], 6 - i);
    }
}

/**
 * Test whether an ARP for our interface IP correctly responds
 */
TEST_F(ARPTest, recvArpResponseTest) {
    ar_t ar;
    ar_t *arp = &ar; 

    // format arp message
    arp->hrd = htons(1);
    arp->pro = htons(ETH_P_IP);
    arp->hln = 6; // hardware proto. length
    arp->pln = 4; // protocol length (IPv4 = 4)
    arp->op = htons(ARPOP_REQUEST); // ARP REQUEST
    memcpy(arp->sha, src_mac, 6);
    inet_pton(AF_INET, src_protocol, &arp->spa);
    arp->tpa = htonl(convert_ip("192.168.1.104")); // target is us

    // Should return 0 as the ARP is for our interface IP
    EXPECT_EQ(recv_arp(arp, interface), 0);

    char buf[1500];
    lseek(interface.fd, 0, SEEK_SET);
    if (read(interface.fd, buf, 1500) < 0) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    // check if written REPLY is correct
    arp = (ar_t *)(buf + sizeof(eth_hdr_t));
    EXPECT_EQ(arp->hrd, htons(ARPHRD_ETHER)); // hardware address space
    EXPECT_EQ(arp->pro, htons(ETH_P_IP)); // protocol address space
    EXPECT_EQ(arp->hln, 6); // byte length of hardware address
    EXPECT_EQ(arp->pln, 4); // byte length of protocol address
    EXPECT_EQ(arp->op, htons(ARPOP_REPLY)); // Opcode
    // Source MAC should be {0x1, 0x2, 0x3, 0x4, 0x5, 0x6}
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(arp->sha[i], i + 1);
    }
    // Source Protocol IPv4 address should be us
    EXPECT_EQ(arp->spa, htonl(interface.src_ip));
    // Target MAC should be {0x6, 0x5, 0x4, ...}
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(arp->tha[i], 6 - i);
    }
    // Target protocol IPv4 address should be 192.1.1.1
    uint32_t tpa;
    inet_pton(AF_INET, src_protocol, &tpa);
    EXPECT_EQ(arp->tpa, tpa);
}

/**
 * Test whether or not an ARP NOT for our IP caches (should not)
 */
TEST_F(ARPTest, recvArpNotForUsTest) {
    ar_t ar;
    ar_t *arp = &ar;

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
    ASSERT_EQ(recv_arp(arp, interface), -1);

    // check if ARP cache has cached IP addr (should not because we've never talked)
    uint8_t mac_value[6];
    uint32_t src_prot_network;
    inet_pton(AF_INET, src_protocol, &src_prot_network);
    src_prot_network = ntohl(src_prot_network);
    ASSERT_EQ(lookup_ip(mac_value, src_prot_network), -1);
}

/**
 * Test ARP reply caching
 */
TEST_F(ARPTest, recvArpReply) {
    ar_t ar;
    ar_t *arp = &ar;

    // format arp message
    arp->hrd = htons(1); // Ethernet msg
    arp->pro = htons(ETH_P_IP); // IP
    arp->hln = 6; // hardware proto. length
    arp->pln = 4; // protocol length (IPv4 = 4)
    arp->op = htons(ARPOP_REPLY); // ARP REPLY
    memcpy(arp->sha, src_mac, 6); // source MAC
    arp->spa = htonl(convert_ip(src_protocol));
    arp->tpa = htonl(interface.src_ip); // target ip is US
    memcpy(arp->tha, interface.src_mac, 6); // target mac is US

    // Should return -1 as no ARP reply is needed from us
    ASSERT_EQ(recv_arp(arp, interface), -1);

    // check if ARP cache has cached IP addr
    uint8_t mac_value[6];
    ASSERT_EQ(lookup_ip(mac_value, convert_ip(src_protocol)), 0);

    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(mac_value[i], src_mac[i]);
    }
}