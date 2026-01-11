/**
 * @file test_rtu_frame.c
 * @brief Unit tests for RTU frame building and parsing
 */

#include "unity.h"
#include "protocol/rtu_frame.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_rtu_build_frame_basic(void) {
    uint8_t pdu[] = {0x00, 0x00, 0x00, 0x02};
    uint8_t frame[260];

    int result = mb_rtu_build_frame(1, 0x03, pdu, 4, frame, sizeof(frame));

    TEST_ASSERT_GREATER_THAN(0, result);
    TEST_ASSERT_EQUAL_UINT8(1, frame[0]);      // Slave ID
    TEST_ASSERT_EQUAL_UINT8(0x03, frame[1]);   // Function code
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[2]);   // PDU data
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[3]);
    TEST_ASSERT_EQUAL_UINT8(0x00, frame[4]);
    TEST_ASSERT_EQUAL_UINT8(0x02, frame[5]);
    // CRC16 at frame[6] and frame[7]
}

void test_rtu_build_frame_buffer_too_small(void) {
    uint8_t pdu[] = {0x00, 0x00, 0x00, 0x02};
    uint8_t frame[5];  // Too small

    int result = mb_rtu_build_frame(1, 0x03, pdu, 4, frame, sizeof(frame));

    TEST_ASSERT_EQUAL(MB_ERROR_BUFFER_TOO_SMALL, result);
}

void test_rtu_build_frame_null_buffer(void) {
    uint8_t pdu[] = {0x00, 0x00, 0x00, 0x02};

    int result = mb_rtu_build_frame(1, 0x03, pdu, 4, NULL, 260);

    TEST_ASSERT_EQUAL(MB_ERROR_INVALID_PARAM, result);
}

void test_rtu_parse_frame_valid(void) {
    // Valid RTU frame: slave 1, FC03, PDU, CRC
    uint8_t frame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};
    uint8_t slave_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_rtu_parse_frame(frame, 8, &slave_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT8(1, slave_id);
    TEST_ASSERT_EQUAL_UINT8(0x03, fc);
    TEST_ASSERT_EQUAL_UINT16(4, pdu_length);
    TEST_ASSERT_EQUAL_UINT8(0x00, pdu[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, pdu[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, pdu[2]);
    TEST_ASSERT_EQUAL_UINT8(0x02, pdu[3]);
}

void test_rtu_parse_frame_invalid_crc(void) {
    // Invalid CRC
    uint8_t frame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xFF, 0xFF};
    uint8_t slave_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_rtu_parse_frame(frame, 8, &slave_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_ERROR_CRC_MISMATCH, result);
}

void test_rtu_parse_frame_too_short(void) {
    uint8_t frame[] = {0x01, 0x03};
    uint8_t slave_id, fc;
    uint8_t pdu[256];
    uint16_t pdu_length;

    int result = mb_rtu_parse_frame(frame, 2, &slave_id, &fc, pdu, &pdu_length);

    TEST_ASSERT_EQUAL(MB_ERROR_INVALID_FRAME, result);
}

void test_rtu_calc_frame_length(void) {
    // PDU length 4 -> frame length = 1 (slave) + 1 (FC) + 4 (PDU) + 2 (CRC) = 8
    uint16_t length = mb_rtu_calc_frame_length(4);
    TEST_ASSERT_EQUAL_UINT16(8, length);
}

void test_rtu_calc_frame_length_zero(void) {
    uint16_t length = mb_rtu_calc_frame_length(0);
    TEST_ASSERT_EQUAL_UINT16(4, length);  // Minimum frame
}

void test_rtu_calc_frame_length_max(void) {
    // PDU length 252 -> frame = 1 (slave) + 1 (FC) + 252 (PDU) + 2 (CRC) = 256
    uint16_t length = mb_rtu_calc_frame_length(252);
    TEST_ASSERT_EQUAL_UINT16(256, length);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_rtu_build_frame_basic);
    RUN_TEST(test_rtu_build_frame_buffer_too_small);
    RUN_TEST(test_rtu_build_frame_null_buffer);
    RUN_TEST(test_rtu_parse_frame_valid);
    RUN_TEST(test_rtu_parse_frame_invalid_crc);
    RUN_TEST(test_rtu_parse_frame_too_short);
    RUN_TEST(test_rtu_calc_frame_length);
    RUN_TEST(test_rtu_calc_frame_length_zero);
    RUN_TEST(test_rtu_calc_frame_length_max);

    return UNITY_END();
}
