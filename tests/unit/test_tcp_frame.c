/**
 * @file test_tcp_frame.c
 * @brief Unit tests for TCP frame building and parsing
 */

#include "unity.h"
#include "protocol/tcp_frame.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_tcp_build_frame_basic(void) {
    uint8_t pdu[] = {0x00, 0x00, 0x00, 0x02};
    uint8_t frame[260];

    int result = mb_tcp_build_frame(0x1234, 1, 0x03, pdu, 4, frame, sizeof(frame));

    TEST_ASSERT_GREATER_THAN(0, result);
    TEST_ASSERT_EQUAL_UINT8(0x12, frame[0]);   // Transaction ID high
    TEST_ASSERT_EQUAL_UINT8(0x34, frame[1]);   // Transaction ID low
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[2]);   // Protocol ID high
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[3]);   // Protocol ID low
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[4]);   // Length high
    TEST_ASSERT_EQUAL_UINT8(0x06, frame[5]);   // Length low (1+1+4)
    TEST_ASSERT_EQUAL_UINT8(0x01, frame[6]);   // Unit ID
    TEST_ASSERT_EQUAL_UINT8(0x03, frame[7]);   // Function code
}

void test_tcp_parse_frame_valid(void) {
    uint8_t frame[] = {0x12, 0x34, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    uint16_t transaction_id;
    uint8_t unit_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_tcp_parse_frame(frame, 12, &transaction_id, &unit_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT16(0x1234, transaction_id);
    TEST_ASSERT_EQUAL_UINT8(1, unit_id);
    TEST_ASSERT_EQUAL_UINT8(0x03, fc);
    TEST_ASSERT_EQUAL_UINT16(4, pdu_length);
}

void test_tcp_parse_frame_invalid_protocol_id(void) {
    uint8_t frame[] = {0x12, 0x34, 0x00, 0x01, 0x00, 0x06, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02};
    uint16_t transaction_id;
    uint8_t unit_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_tcp_parse_frame(frame, 12, &transaction_id, &unit_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_ERROR_INVALID_FRAME, result);
}

void test_tcp_calc_frame_length(void) {
    // PDU length 4 -> MBAP(6) + UnitID(1) + FC(1) + PDU(4) = 12
    uint16_t length = mb_tcp_calc_frame_length(4);
    TEST_ASSERT_EQUAL_UINT16(12, length);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_tcp_build_frame_basic);
    RUN_TEST(test_tcp_parse_frame_valid);
    RUN_TEST(test_tcp_parse_frame_invalid_protocol_id);
    RUN_TEST(test_tcp_calc_frame_length);

    return UNITY_END();
}
