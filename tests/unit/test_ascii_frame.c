/**
 * @file test_ascii_frame.c
 * @brief Unit tests for ASCII frame building and parsing
 */

#include "unity.h"
#include "protocol/ascii_frame.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_ascii_build_frame_basic(void) {
    uint8_t pdu[] = {0x00, 0x00, 0x00, 0x02};
    uint8_t frame[520];

    int result = mb_ascii_build_frame(1, 0x03, pdu, 4, frame, sizeof(frame));

    TEST_ASSERT_GREATER_THAN(0, result);
    TEST_ASSERT_EQUAL_UINT8(':', frame[0]);    // Start character
    TEST_ASSERT_EQUAL_UINT8('0', frame[1]);    // Slave ID high nibble
    TEST_ASSERT_EQUAL_UINT8('1', frame[2]);    // Slave ID low nibble
    TEST_ASSERT_EQUAL_UINT8('0', frame[3]);    // FC high nibble
    TEST_ASSERT_EQUAL_UINT8('3', frame[4]);    // FC low nibble
}

void test_ascii_build_frame_buffer_too_small(void) {
    uint8_t pdu[] = {0x00, 0x00, 0x00, 0x02};
    uint8_t frame[10];  // Too small

    int result = mb_ascii_build_frame(1, 0x03, pdu, 4, frame, sizeof(frame));

    TEST_ASSERT_EQUAL(MB_ERROR_BUFFER_TOO_SMALL, result);
}

void test_ascii_parse_frame_valid(void) {
    // Valid ASCII frame: :010300000002FA\r\n
    uint8_t frame[] = ":010300000002FA\r\n";
    uint8_t slave_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_ascii_parse_frame(frame, 17, &slave_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT8(1, slave_id);
    TEST_ASSERT_EQUAL_UINT8(0x03, fc);
    TEST_ASSERT_EQUAL_UINT16(4, pdu_length);
}

void test_ascii_parse_frame_invalid_lrc(void) {
    // Invalid LRC
    uint8_t frame[] = ":010300000002FF\r\n";
    uint8_t slave_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_ascii_parse_frame(frame, 17, &slave_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_ERROR_LRC_MISMATCH, result);
}

void test_ascii_parse_frame_missing_start(void) {
    uint8_t frame[] = "010300000002FA\r\n";  // Missing ':'
    uint8_t slave_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_ascii_parse_frame(frame, 16, &slave_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_ERROR_INVALID_FRAME, result);
}

void test_ascii_calc_frame_length(void) {
    // PDU length 4 -> 1(':') + 2(slave) + 2(FC) + 8(PDU) + 2(LRC) + 2(CRLF) = 17
    uint16_t length = mb_ascii_calc_frame_length(4);
    TEST_ASSERT_EQUAL_UINT16(17, length);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_ascii_build_frame_basic);
    RUN_TEST(test_ascii_build_frame_buffer_too_small);
    RUN_TEST(test_ascii_parse_frame_valid);
    RUN_TEST(test_ascii_parse_frame_invalid_lrc);
    RUN_TEST(test_ascii_parse_frame_missing_start);
    RUN_TEST(test_ascii_calc_frame_length);

    return UNITY_END();
}
