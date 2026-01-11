/**
 * @file test_response_parser.c
 * @brief Unit tests for response parser
 */

#include "unity.h"
#include "master/response_parser.h"
#include "smartmodbus/mb_error.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_parse_read_coils_response(void) {
    // Response: byte_count=2, data=0xCD 0x6B
    uint8_t pdu[] = {0x02, 0xCD, 0x6B};
    uint8_t data[16];

    int ret = mb_parse_read_response(MB_FC_READ_COILS, pdu, 3, 13, data);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_HEX8(0xCD, data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x6B, data[1]);
}

void test_parse_read_registers_response(void) {
    // Response: byte_count=4, data=0x0001 0x0002 (big-endian)
    uint8_t pdu[] = {0x04, 0x00, 0x01, 0x00, 0x02};
    uint16_t data[2];

    int ret = mb_parse_read_response(MB_FC_READ_HOLDING_REGISTERS, pdu, 5, 2, data);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
    TEST_ASSERT_EQUAL_UINT16(0x0001, data[0]);
    TEST_ASSERT_EQUAL_UINT16(0x0002, data[1]);
}

void test_parse_read_response_invalid_byte_count(void) {
    // Wrong byte count
    uint8_t pdu[] = {0x05, 0x00, 0x01, 0x00, 0x02};
    uint16_t data[2];

    int ret = mb_parse_read_response(MB_FC_READ_HOLDING_REGISTERS, pdu, 5, 2, data);

    TEST_ASSERT_EQUAL(MB_ERROR_INVALID_FRAME, ret);
}

void test_parse_read_response_exception(void) {
    // Exception response (FC with 0x80 bit set)
    uint8_t pdu[] = {0x02};  // Exception code
    uint8_t dummy_buffer[1];  // Provide a valid buffer

    int ret = mb_parse_read_response(0x83, pdu, 1, 2, dummy_buffer);

    TEST_ASSERT_EQUAL(MB_ERROR_EXCEPTION_RESPONSE, ret);
}

void test_parse_write_single_coil_response(void) {
    // Echo response: address=0x00AC, value=0xFF00
    uint8_t pdu[] = {0x00, 0xAC, 0xFF, 0x00};
    bool expected_value = true;

    int ret = mb_parse_write_response(MB_FC_WRITE_SINGLE_COIL, pdu, 4, 0x00AC, 1, &expected_value);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
}

void test_parse_write_single_register_response(void) {
    // Echo response: address=0x0001, value=0x0003
    uint8_t pdu[] = {0x00, 0x01, 0x00, 0x03};
    uint16_t expected_value = 0x0003;

    int ret = mb_parse_write_response(MB_FC_WRITE_SINGLE_REGISTER, pdu, 4, 0x0001, 1, &expected_value);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
}

void test_parse_write_multiple_registers_response(void) {
    // Response: address=0x0001, quantity=0x0002
    uint8_t pdu[] = {0x00, 0x01, 0x00, 0x02};

    int ret = mb_parse_write_response(MB_FC_WRITE_MULTIPLE_REGISTERS, pdu, 4, 0x0001, 2, NULL);

    TEST_ASSERT_EQUAL(MB_SUCCESS, ret);
}

void test_get_exception_code(void) {
    uint8_t pdu[] = {0x02};  // Exception code 2 (illegal data address)

    uint8_t code = mb_get_exception_code(pdu, 1);

    TEST_ASSERT_EQUAL_UINT8(0x02, code);
}

void test_get_exception_code_invalid(void) {
    uint8_t code = mb_get_exception_code(NULL, 0);

    TEST_ASSERT_EQUAL_UINT8(0, code);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parse_read_coils_response);
    RUN_TEST(test_parse_read_registers_response);
    RUN_TEST(test_parse_read_response_invalid_byte_count);
    RUN_TEST(test_parse_read_response_exception);
    RUN_TEST(test_parse_write_single_coil_response);
    RUN_TEST(test_parse_write_single_register_response);
    RUN_TEST(test_parse_write_multiple_registers_response);
    RUN_TEST(test_get_exception_code);
    RUN_TEST(test_get_exception_code_invalid);

    return UNITY_END();
}
