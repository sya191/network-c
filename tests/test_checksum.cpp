#include <gtest/gtest.h>

extern "C" {
#include "checksum.h"
}

// AAA Pattern (Arrange, Act, Assert)

/**
 * Checksum should return all ones for inputs of all zeros
 */
TEST(ChecksumTest, ReturnsAllOnesForZeroInputEven) {
    uint8_t data[4] = {0, 0, 0, 0};
    uint16_t result = checksum(data, sizeof(data));
    EXPECT_EQ(result, (uint16_t)~0);
}

/**
 * Checksum should return all ones for non 16-bit (2-byte) multiples
 */
TEST(ChecksumTest, ReturnsAllOnesForZeroInputOdd) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint16_t result = checksum(data, sizeof(data));
    EXPECT_EQ(result, (uint16_t)~0);
}

/**
 * One's complement checksum for 0xdeadbeef is 0x6262
 */
TEST(ChecksumTest, ReturnsCorrectForEvenInput) {
    uint8_t data[4] = {0xde, 0xad, 0xbe, 0xef};
    // 0xdead + 0xbeef = 0x19d9c
    // carry the 1 = 0x9d9d
    // flip bits = 0x6262
    uint16_t result = checksum(data, sizeof(data));
    EXPECT_EQ(result, 0x6262);
}

/**
 * One's complement checksum for 0xdeadbeefaa is 
 */
TEST(ChecksumTest, ReturnsCorrectForOddInput) {
    // 0xdead + 0xbeef + 0xaa00 (padded from lsb) = 0x2479c
    // carry the 2 = 0x479e
    // flip bits = 0xb861
    uint8_t data[5] = {0xde, 0xad, 0xbe, 0xef, 0xaa};
    uint16_t result = checksum(data, sizeof(data));
    EXPECT_EQ(result, 0xb861);
}