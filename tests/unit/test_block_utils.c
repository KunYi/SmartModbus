/**
 * @file test_block_utils.c
 * @brief Unit tests for block utility functions
 */

#include "unity.h"
#include "utils/block_utils.h"
#include "smartmodbus/mb_types.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_addresses_to_blocks_contiguous(void) {
    uint16_t addresses[] = {100, 101, 102, 103};
    mb_block_t blocks[10];
    uint16_t block_count = 0;

    int ret = mb_addresses_to_blocks(addresses, 4, 1, MB_FC_READ_HOLDING_REGISTERS,
                                      blocks, 10, &block_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(1, block_count);
    TEST_ASSERT_EQUAL_UINT16(100, blocks[0].start_address);
    TEST_ASSERT_EQUAL_UINT16(4, blocks[0].quantity);
}

void test_addresses_to_blocks_non_contiguous(void) {
    uint16_t addresses[] = {100, 101, 105, 106};
    mb_block_t blocks[10];
    uint16_t block_count = 0;

    int ret = mb_addresses_to_blocks(addresses, 4, 1, MB_FC_READ_HOLDING_REGISTERS,
                                      blocks, 10, &block_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(2, block_count);
    TEST_ASSERT_EQUAL_UINT16(100, blocks[0].start_address);
    TEST_ASSERT_EQUAL_UINT16(2, blocks[0].quantity);
    TEST_ASSERT_EQUAL_UINT16(105, blocks[1].start_address);
    TEST_ASSERT_EQUAL_UINT16(2, blocks[1].quantity);
}

void test_addresses_to_blocks_unsorted(void) {
    uint16_t addresses[] = {105, 100, 102, 101};
    mb_block_t blocks[10];
    uint16_t block_count = 0;

    int ret = mb_addresses_to_blocks(addresses, 4, 1, MB_FC_READ_HOLDING_REGISTERS,
                                      blocks, 10, &block_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    // Should sort and create contiguous blocks
    TEST_ASSERT_EQUAL_UINT16(2, block_count);
}

void test_calc_gap_between_blocks(void) {
    mb_block_t block_a = {.start_address = 100, .quantity = 3};
    mb_block_t block_b = {.start_address = 105, .quantity = 3};

    uint16_t gap = mb_block_calc_gap(&block_a, &block_b);
    TEST_ASSERT_EQUAL_UINT16(2, gap);  // 105 - (100 + 3) = 2
}

void test_calc_gap_between_blocks_adjacent(void) {
    mb_block_t block_a = {.start_address = 100, .quantity = 3};
    mb_block_t block_b = {.start_address = 103, .quantity = 3};

    uint16_t gap = mb_block_calc_gap(&block_a, &block_b);
    TEST_ASSERT_EQUAL_UINT16(0, gap);
}

void test_blocks_are_compatible_same_slave_fc(void) {
    mb_block_t block_a = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS};
    mb_block_t block_b = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS};

    bool compatible = mb_block_are_compatible(&block_a, &block_b);
    TEST_ASSERT_TRUE(compatible);
}

void test_blocks_are_compatible_different_slave(void) {
    mb_block_t block_a = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS};
    mb_block_t block_b = {.slave_id = 2, .function_code = MB_FC_READ_HOLDING_REGISTERS};

    bool compatible = mb_block_are_compatible(&block_a, &block_b);
    TEST_ASSERT_FALSE(compatible);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_addresses_to_blocks_contiguous);
    RUN_TEST(test_addresses_to_blocks_non_contiguous);
    RUN_TEST(test_addresses_to_blocks_unsorted);
    RUN_TEST(test_calc_gap_between_blocks);
    RUN_TEST(test_calc_gap_between_blocks_adjacent);
    RUN_TEST(test_blocks_are_compatible_same_slave_fc);
    RUN_TEST(test_blocks_are_compatible_different_slave);

    return UNITY_END();
}
