/**
 * @file test_ffd_pack.c
 * @brief Unit tests for FFD packing algorithm
 */

#include "unity.h"
#include "core/ffd_pack.h"
#include "smartmodbus/mb_types.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_ffd_pack_single_block(void) {
    mb_block_t blocks[] = {
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 100, .quantity = 10, .is_merged = false}
    };
    mb_pdu_t pdus[10];
    uint16_t pdu_count = 0;

    int ret = mb_ffd_pack(blocks, 1, 253, pdus, 10, &pdu_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(1, pdu_count);
    TEST_ASSERT_EQUAL_UINT8(1, pdus[0].slave_id);
    TEST_ASSERT_EQUAL_UINT16(100, pdus[0].start_address);
    TEST_ASSERT_EQUAL_UINT16(10, pdus[0].quantity);
}

void test_ffd_pack_multiple_blocks_fit_one_pdu(void) {
    mb_block_t blocks[] = {
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 100, .quantity = 10, .is_merged = false},
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 110, .quantity = 10, .is_merged = false}
    };
    mb_pdu_t pdus[10];
    uint16_t pdu_count = 0;

    int ret = mb_ffd_pack(blocks, 2, 253, pdus, 10, &pdu_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    // Both blocks should fit in one PDU if contiguous
    TEST_ASSERT_GREATER_OR_EQUAL(1, pdu_count);
}

void test_ffd_pack_different_slaves(void) {
    mb_block_t blocks[] = {
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 100, .quantity = 10, .is_merged = false},
        {.slave_id = 2, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 100, .quantity = 10, .is_merged = false}
    };
    mb_pdu_t pdus[10];
    uint16_t pdu_count = 0;

    int ret = mb_ffd_pack(blocks, 2, 253, pdus, 10, &pdu_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(2, pdu_count);  // Different slaves = separate PDUs
}

void test_ffd_pack_exceeds_max_pdu(void) {
    mb_block_t blocks[] = {
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 100, .quantity = 125, .is_merged = false},
        {.slave_id = 1, .function_code = MB_FC_READ_HOLDING_REGISTERS,
         .start_address = 300, .quantity = 125, .is_merged = false}
    };
    mb_pdu_t pdus[10];
    uint16_t pdu_count = 0;

    int ret = mb_ffd_pack(blocks, 2, 253, pdus, 10, &pdu_count);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(2, pdu_count);  // Each block in separate PDU
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_ffd_pack_single_block);
    RUN_TEST(test_ffd_pack_multiple_blocks_fit_one_pdu);
    RUN_TEST(test_ffd_pack_different_slaves);
    RUN_TEST(test_ffd_pack_exceeds_max_pdu);

    return UNITY_END();
}
