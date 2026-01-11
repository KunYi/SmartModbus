/**
 * @file test_gap_merge.c
 * @brief Unit tests for gap-aware merge algorithm
 */

#include "unity.h"
#include "core/gap_merge.h"
#include "smartmodbus/mb_types.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_should_merge_blocks_beneficial(void) {
    mb_block_t block_a = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 100, .quantity = 3, .is_merged = false};
    mb_block_t block_b = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 105, .quantity = 3, .is_merged = false};
    mb_cost_params_t cost = {.req_fixed_chars = 6, .resp_fixed_chars = 5,
                             .gap_chars = 4, .latency_chars = 2};

    // Gap of 2 registers should be beneficial to merge
    bool should_merge = mb_should_merge_blocks(&block_a, &block_b, &cost);
    TEST_ASSERT_TRUE(should_merge);
}

void test_should_merge_blocks_not_beneficial(void) {
    mb_block_t block_a = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 100, .quantity = 3, .is_merged = false};
    mb_block_t block_b = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 150, .quantity = 3, .is_merged = false};
    mb_cost_params_t cost = {.req_fixed_chars = 6, .resp_fixed_chars = 5,
                             .gap_chars = 4, .latency_chars = 2};

    // Gap of 47 registers should not be beneficial to merge
    bool should_merge = mb_should_merge_blocks(&block_a, &block_b, &cost);
    TEST_ASSERT_FALSE(should_merge);
}

void test_should_merge_blocks_different_slaves(void) {
    mb_block_t block_a = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 100, .quantity = 3, .is_merged = false};
    mb_block_t block_b = {.slave_id = 2, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 105, .quantity = 3, .is_merged = false};
    mb_cost_params_t cost = {.req_fixed_chars = 6, .resp_fixed_chars = 5,
                             .gap_chars = 4, .latency_chars = 2};

    // Different slaves should not merge
    bool should_merge = mb_should_merge_blocks(&block_a, &block_b, &cost);
    TEST_ASSERT_FALSE(should_merge);
}

void test_merge_two_blocks(void) {
    mb_block_t block_a = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 100, .quantity = 3, .is_merged = false};
    mb_block_t block_b = {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
                          .start_address = 105, .quantity = 3, .is_merged = false};
    mb_block_t result;

    int ret = mb_merge_two_blocks(&block_a, &block_b, &result);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT8(1, result.slave_id);
    TEST_ASSERT_EQUAL_UINT8(MB_FC_READ_HOLDING_REGISTERS, result.function_code);
    TEST_ASSERT_EQUAL_UINT16(100, result.start_address);
    TEST_ASSERT_EQUAL_UINT16(8, result.quantity);  // 3 + gap(2) + 3
    TEST_ASSERT_TRUE(result.is_merged);
}

void test_merge_block_array(void) {
    mb_block_t blocks[] = {
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 100, .quantity = 3, .is_merged = false},
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 105, .quantity = 3, .is_merged = false},
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 200, .quantity = 5, .is_merged = false}
    };
    uint16_t count = 3;
    mb_cost_params_t cost = {.req_fixed_chars = 6, .resp_fixed_chars = 5,
                             .gap_chars = 4, .latency_chars = 2};

    int ret = mb_merge_block_array(blocks, &count, &cost);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(2, count);  // First two merged, third separate
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_should_merge_blocks_beneficial);
    RUN_TEST(test_should_merge_blocks_not_beneficial);
    RUN_TEST(test_should_merge_blocks_different_slaves);
    RUN_TEST(test_merge_two_blocks);
    RUN_TEST(test_merge_block_array);

    return UNITY_END();
}
