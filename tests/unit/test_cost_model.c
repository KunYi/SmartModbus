/**
 * @file test_cost_model.c
 * @brief Unit tests for character-based cost model
 */

#include "unity.h"
#include "core/char_model.h"
#include "smartmodbus/mb_types.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_calc_overhead_chars_rtu(void) {
    uint16_t overhead = mb_calc_overhead_chars(MB_MODE_RTU, MB_FC_READ_HOLDING_REGISTERS, 4, 2);
    // REQ_FIXED(6) + RESP_FIXED(5) + GAP(4) + LATENCY(2) = 17
    TEST_ASSERT_EQUAL_UINT16(17, overhead);
}

void test_calc_overhead_chars_tcp(void) {
    uint16_t overhead = mb_calc_overhead_chars(MB_MODE_TCP, MB_FC_READ_HOLDING_REGISTERS, 0, 1);
    // REQ_FIXED(6) + RESP_FIXED(5) + GAP(0 for TCP) + LATENCY(1) = 12
    TEST_ASSERT_EQUAL_UINT16(12, overhead);
}

void test_calc_gap_cost_registers(void) {
    // FC03: 2 bytes per register, gap of 10 registers = 20 bytes
    uint16_t cost = mb_calc_gap_cost(MB_FC_READ_HOLDING_REGISTERS, 10);
    TEST_ASSERT_EQUAL_UINT16(20, cost);
}

void test_calc_gap_cost_coils(void) {
    // FC01: 1/8 byte per coil, gap of 16 coils = 2 bytes
    uint16_t cost = mb_calc_gap_cost(MB_FC_READ_COILS, 16);
    TEST_ASSERT_EQUAL_UINT16(2, cost);
}

void test_get_extra_unit_chars_fc03(void) {
    uint16_t extra = mb_get_extra_unit_chars(MB_FC_READ_HOLDING_REGISTERS);
    TEST_ASSERT_EQUAL_UINT16(200, extra);  // 2 bytes × 100
}

void test_get_extra_unit_chars_fc01(void) {
    uint16_t extra = mb_get_extra_unit_chars(MB_FC_READ_COILS);
    TEST_ASSERT_EQUAL_UINT16(12, extra);  // 0.125 bytes × 100
}

void test_init_cost_params(void) {
    mb_cost_params_t params;
    mb_init_cost_params(MB_MODE_RTU, MB_FC_READ_HOLDING_REGISTERS, 2, &params);

    TEST_ASSERT_EQUAL_UINT8(6, params.req_fixed_chars);
    TEST_ASSERT_EQUAL_UINT8(5, params.resp_fixed_chars);
    TEST_ASSERT_EQUAL_UINT8(4, params.gap_chars);
    TEST_ASSERT_EQUAL_UINT8(2, params.latency_chars);
}

void test_calc_merge_savings_beneficial(void) {
    mb_cost_params_t params = {
        .req_fixed_chars = 6,
        .resp_fixed_chars = 5,
        .gap_chars = 4,
        .latency_chars = 2
    };

    // Gap of 5 registers: gap_cost = 5 * 2 = 10, overhead = 17
    // Savings = 17 - 10 = 7 (beneficial)
    int16_t savings = mb_calc_merge_savings(5, MB_FC_READ_HOLDING_REGISTERS, &params);
    TEST_ASSERT_GREATER_THAN(0, savings);
}

void test_calc_merge_savings_not_beneficial(void) {
    mb_cost_params_t params = {
        .req_fixed_chars = 6,
        .resp_fixed_chars = 5,
        .gap_chars = 4,
        .latency_chars = 2
    };

    // Gap of 20 registers: gap_cost = 20 * 2 = 40, overhead = 17
    // Savings = 17 - 40 = -23 (not beneficial)
    int16_t savings = mb_calc_merge_savings(20, MB_FC_READ_HOLDING_REGISTERS, &params);
    TEST_ASSERT_LESS_THAN(0, savings);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_calc_overhead_chars_rtu);
    RUN_TEST(test_calc_overhead_chars_tcp);
    RUN_TEST(test_calc_gap_cost_registers);
    RUN_TEST(test_calc_gap_cost_coils);
    RUN_TEST(test_get_extra_unit_chars_fc03);
    RUN_TEST(test_get_extra_unit_chars_fc01);
    RUN_TEST(test_init_cost_params);
    RUN_TEST(test_calc_merge_savings_beneficial);
    RUN_TEST(test_calc_merge_savings_not_beneficial);

    return UNITY_END();
}
