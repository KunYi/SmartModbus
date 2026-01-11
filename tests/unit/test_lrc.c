/**
 * @file test_lrc.c
 * @brief Unit tests for LRC calculation
 */

#include "unity.h"
#include "protocol/lrc.h"

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

void test_lrc_empty_data(void) {
    uint8_t data[] = {};
    uint8_t lrc = mb_lrc(data, 0);
    TEST_ASSERT_EQUAL_HEX8(0x00, lrc);
}

void test_lrc_single_byte(void) {
    uint8_t data[] = {0x01};
    uint8_t lrc = mb_lrc(data, 1);
    TEST_ASSERT_EQUAL_HEX8(0xFF, lrc);  // Two's complement of 0x01
}

void test_lrc_modbus_example(void) {
    // Example: slave 1, FC03, start 0x0000, quantity 0x0002
    uint8_t data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    uint8_t lrc = mb_lrc(data, 6);
    // Sum = 0x06, LRC = -0x06 = 0xFA
    TEST_ASSERT_EQUAL_HEX8(0xFA, lrc);
}

void test_lrc_all_zeros(void) {
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00};
    uint8_t lrc = mb_lrc(data, 4);
    TEST_ASSERT_EQUAL_HEX8(0x00, lrc);
}

void test_lrc_all_ones(void) {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t lrc = mb_lrc(data, 4);
    // Sum = 0xFC (overflow), LRC = -0xFC = 0x04
    TEST_ASSERT_EQUAL_HEX8(0x04, lrc);
}

void test_lrc_overflow(void) {
    // Test that overflow is handled correctly
    uint8_t data[] = {0x80, 0x80, 0x80, 0x80};
    uint8_t lrc = mb_lrc(data, 4);
    // Sum = 0x00 (overflow), LRC = 0x00
    TEST_ASSERT_EQUAL_HEX8(0x00, lrc);
}

void test_lrc_sequential(void) {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t lrc = mb_lrc(data, 5);
    // Sum = 0x0F, LRC = -0x0F = 0xF1
    TEST_ASSERT_EQUAL_HEX8(0xF1, lrc);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_lrc_empty_data);
    RUN_TEST(test_lrc_single_byte);
    RUN_TEST(test_lrc_modbus_example);
    RUN_TEST(test_lrc_all_zeros);
    RUN_TEST(test_lrc_all_ones);
    RUN_TEST(test_lrc_overflow);
    RUN_TEST(test_lrc_sequential);

    return UNITY_END();
}
