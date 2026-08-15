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

class EthernetTest: public ::testing::Test {
protected:
    void SetUp() override {
        int fd = open("./", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror("open()");
            exit(EXIT_FAILURE);
        }
        interface = {
            .write = test_write,
            .read = test_read,
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
    uint8_t target_mac[6] = {0x6, 0x5, 0x4, 0x3, 0x2, 0x1};
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char target_ip[10] = "192.1.1.1"; // sender not our interface
};

// Should get the correct EtherType to hand to upper level modules
TEST_F(EthernetTest, DemuxOnEthertype) {
    uint8_t buf[sizeof(eth_hdr_t)];
    eth_hdr_t *eth_hdr = (eth_hdr_t *)buf;
    eth_hdr->ethertype = htons(ETH_P_ARP);
    memcpy(eth_hdr->mac_dest, interface.src_mac, 6);
    // write to test file
    test_write(interface.fd, buf, sizeof(buf));
    ASSERT_EQ(recv_eth(interface), ETH_P_ARP);
}

// Tests whether ethernet frame is correctly formatted on send to MAC
TEST_F(EthernetTest, SendToMacDirect) {
    const char *msg = "Hello World!";
    int res = send_eth_to_mac(
        (void *)msg,
        ETH_P_IP,
        target_mac,
        sizeof("Hello World!"),
        interface
    );

    ASSERT_EQ(res, 0);

    // read msg sent to fd
    uint8_t buf[1500]; // MTU
    test_read(interface.fd, buf);
    eth_hdr_t *eth_hdr = (eth_hdr_t *)buf;
    // check ethernet header fields
    uint16_t sent_ethertype = ntohs(eth_hdr->ethertype);
    ASSERT_EQ(sent_ethertype, ETH_P_IP);
    for (int i = 0; i < 6; ++i) {
        ASSERT_EQ(eth_hdr->mac_dest[i], target_mac[i]);
        ASSERT_EQ(eth_hdr->mac_src[i], interface.src_mac[i]);
    }
    char *payload = (char *)buf + sizeof(eth_hdr_t);
    ASSERT_EQ(0, strcmp(payload, msg));
}

// Tests direct sending to IP address without a MAC available.
// Should send an ARP Request on behalf
TEST_F(EthernetTest, SendToIPNoMAC) {
    const char *msg = "Hello World!";
    int res = send_eth_to_ip(
        (void *)msg,
        ETH_P_IP,
        convert_ip(target_ip),
        sizeof("Hello World!"),
        interface
    );

    // Should be unable to find IP in ARP cache
    EXPECT_EQ(res, -1);

    // Should send ARP REQUEST on behalf of caller
    uint8_t buf[1500]; // MTU
    test_read(interface.fd, buf);
    eth_hdr_t *eth_hdr = (eth_hdr_t *)buf;
    uint16_t sent_ethertype = ntohs(eth_hdr->ethertype);
    ASSERT_EQ(sent_ethertype, ETH_P_ARP);
    for (int i = 0; i < 6; ++i) {
        ASSERT_EQ(eth_hdr->mac_dest[i], 0xff);
    }
}

// Tests direct sending to IP address with a MAC available.
TEST_F(EthernetTest, SendToIPHaveMAC) {
    // mock ARP REPLY to our stack to cache ip/mac translation
    ar_t ar;
    ar_t *arp = &ar;

    // format arp message
    arp->hrd = htons(1); // Ethernet msg
    arp->pro = htons(ETH_P_IP); // IP
    arp->hln = 6; // hardware proto. length
    arp->pln = 4; // protocol length (IPv4 = 4)
    arp->op = htons(ARPOP_REPLY); // ARP REPLY
    memcpy(arp->sha, target_mac, 6); // source MAC
    arp->spa = htonl(convert_ip(target_ip));
    arp->tpa = htonl(interface.src_ip); // target ip is US
    memcpy(arp->tha, interface.src_mac, 6); // target mac is US

    EXPECT_EQ(recv_arp(arp, interface), -1);

    const char *msg = "Hello World!";
    int res = send_eth_to_ip(
        (void *)msg,
        ETH_P_IP,
        convert_ip(target_ip),
        sizeof("Hello World!"),
        interface
    );

    // Should be able to find IP in ARP cache
    EXPECT_EQ(res, 0);

    // read msg sent to fd
    uint8_t buf[1500]; // MTU
    test_read(interface.fd, buf);
    eth_hdr_t *eth_hdr = (eth_hdr_t *)buf;
    // check ethernet header fields
    uint16_t sent_ethertype = ntohs(eth_hdr->ethertype);
    ASSERT_EQ(sent_ethertype, ETH_P_IP);
    for (int i = 0; i < 6; ++i) {
        ASSERT_EQ(eth_hdr->mac_dest[i], target_mac[i]);
        ASSERT_EQ(eth_hdr->mac_src[i], interface.src_mac[i]);
    }
    char *payload = (char *)buf + sizeof(eth_hdr_t);
    ASSERT_EQ(0, strcmp(payload, msg));
}