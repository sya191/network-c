#include <gtest/gtest.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
extern "C" {
#include "fragment.h"
#include "ip.h"
#include "utils.h"
}

class FragmentTest: public ::testing::Test {
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
        };
    }

    void TearDown() override {
        close(interface.fd);
        free_frags();
    }

    iface_t interface;
    uint8_t target_mac[6] = {0x6, 0x5, 0x4, 0x3, 0x2, 0x1};
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    char target_ip[10] = "192.1.1.1"; // sender not our interface
};

/**
 * Tests if correctly identifies fragment
 */
TEST_F(FragmentTest, isFragment) {
    // format IP hdr
    ip_t frame = {
        .total_len = 10,
        .flags_offset = htons(0b0010000000000000),
    };

    // More fragment bit 1, offset not set
    ASSERT_EQ(is_fragment(&frame), true);

    frame.total_len = 10;
    frame.flags_offset = htons(0b0000000000000001);

    // More fragment bit 0, but offset is set
    ASSERT_EQ(is_fragment(&frame), true);

    frame.total_len = 10;
    frame.flags_offset = htons(0b0010000000000001);

    // More fragment bit 1, offset is set
    ASSERT_EQ(is_fragment(&frame), true);
}

/**
 * Tests if correctly identifies non fragment
 */
TEST_F(FragmentTest, isNotFragment) {
    // format IP hdr
    ip_t frame = {
        .total_len = 10,
        .flags_offset = htons(0b0000000000000000),
    };

    ASSERT_EQ(is_fragment(&frame), false);
}

TEST_F(FragmentTest, offsetCalc) {
    // format IP hdr 
    ip_t hdr = {
        .total_len = ntohs(sizeof(ip_t) + 5),
        .flags_offset = ntohs(0x1)
    };
    int offsets[2];
    extract_offset(offsets, &hdr);
    ASSERT_EQ(offsets[0], 8); // inclusive
    ASSERT_EQ(offsets[1], 12); // inclusive
}

/**
 * Tests if correctly identifies overlapping ranges/offsets
 */
TEST_F(FragmentTest, rangeOverlaps) {
    // {0, 1} AND {0, 1} == TRUE
    int src0[2] = {0, 1};
    int src1[2] = {0, 1};
    ASSERT_EQ(offsets_overlap(src0, src1), true);

    // {0, 1} AND {2, 3} == FALSE
    src1[0] = 2;
    src1[1] = 3;
    ASSERT_EQ(offsets_overlap(src0, src1), false);

    // {0, 1} AND {1, 2} == TRUE
    src1[0] = 1;
    src1[1] = 2;
    ASSERT_EQ(offsets_overlap(src0, src1), true);

    //{0, 0} AND {0, 0} == TRUE
    src0[0] = 0;
    src0[1] = 0;
    src1[0] = 0;
    src1[1] = 0;
    ASSERT_EQ(offsets_overlap(src0, src1), true);
}



/**
 * Tests if correctly pushes fragmented datagrams up stack
 */
TEST_F(FragmentTest, recvTwoFragments) {
    // assemble IPv4 hdr + payload
    uint16_t size = sizeof(ip_t) + 8;
    uint8_t frag1[sizeof(ip_t) + 8] = {0};
    ip_t *hdr = (ip_t *)frag1;
    hdr->total_len = htons(size);
    // more fragments = 1, offset = 0
    hdr->flags_offset = htons(0x2000);
    hdr->identification = htons(0x1);
    hdr->protocol = 254;

    // set payload
    char *payload = (char *)(hdr + 1);
    // fragment by multiple of 8
    strncpy(payload, "Hello wo", 8);

    // recv fragment
    ASSERT_EQ(add_fragment((ip_t *)frag1, &interface), 0);

    // set up second fragment
    size = sizeof(ip_t) + 5;
    uint8_t frag2[size] = {0};
    hdr = (ip_t *)frag2;
    hdr->total_len = htons(size);
    // more fragments = 0, offset = 1
    hdr->flags_offset = htons(0x1);
    hdr->identification = htons(0x1);
    hdr->protocol = 254;
    
    payload = (char *)(hdr + 1);
    strncpy(payload, "rld!", 5);

    ASSERT_EQ(add_fragment((ip_t *)frag2, &interface), 254);
}

/**
 * Tests if correctly assembles fragmented datagrams
 */
TEST_F(FragmentTest, correctAssembly) {
    // assemble IPv4 hdr + payload
    uint16_t size = sizeof(ip_t) + 8;
    uint8_t frag1[sizeof(ip_t) + 8] = {0};
    ip_t *hdr = (ip_t *)frag1;
    hdr->total_len = htons(size);
    // more fragments = 1, offset = 0
    hdr->flags_offset = htons(0x2000);
    hdr->identification = htons(0x1);
    hdr->protocol = 254;

    // set payload
    char *payload = (char *)(hdr + 1);
    // fragment by multiple of 8
    strncpy(payload, "Hello Wo", 8);

    // recv fragment
    add_fragment((ip_t *)frag1, &interface);

    // set up second fragment
    size = sizeof(ip_t) + 5;
    uint8_t frag2[size] = {0};
    hdr = (ip_t *)frag2;
    hdr->total_len = htons(size);
    // more fragments = 0, offset = 1
    hdr->flags_offset = htons(0x1);
    hdr->identification = htons(0x1);
    hdr->protocol = 254;
    
    payload = (char *)(hdr + 1);
    strncpy(payload, "rld!", 5);

    add_fragment((ip_t *)frag2, &interface);

    char msg[13];
    read(interface.fd, msg, 13);
    ASSERT_EQ(strcmp(msg, "Hello World!"), 0);
}