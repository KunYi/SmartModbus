/**
 * @file response_parser.c
 * @brief Response parsing implementation
 *
 * Parses Modbus response frames and extracts data based on function code.
 */

#include "response_parser.h"

#include "../core/fc_policy.h"
#include "smartmodbus/mb_error.h"

#include <string.h>

/**
 * @brief Parse read coils/discrete inputs response (FC01/02)
 */
static int parse_read_bits_response(const uint8_t *pdu_data,
                                    uint16_t pdu_length,
                                    uint16_t quantity,
                                    uint8_t *data_buffer) {
    if (pdu_length < 1) {
        return MB_ERROR_INVALID_FRAME;
    }

    uint8_t byte_count = pdu_data[0];
    uint16_t expected_bytes = (quantity + 7) / 8;

    if (byte_count != expected_bytes || pdu_length < 1 + byte_count) {
        return MB_ERROR_INVALID_FRAME;
    }

    memcpy(data_buffer, &pdu_data[1], byte_count);
    return MB_SUCCESS;
}

/**
 * @brief Parse read registers response (FC03/04)
 */
static int parse_read_registers_response(const uint8_t *pdu_data,
                                         uint16_t pdu_length,
                                         uint16_t quantity,
                                         uint16_t *data_buffer) {
    if (pdu_length < 1) {
        return MB_ERROR_INVALID_FRAME;
    }

    uint8_t byte_count = pdu_data[0];
    uint16_t expected_bytes = quantity * 2;

    if (byte_count != expected_bytes || pdu_length < 1 + byte_count) {
        return MB_ERROR_INVALID_FRAME;
    }

    // Convert big-endian to host byte order
    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t offset = 1 + (i * 2);
        data_buffer[i] = ((uint16_t)pdu_data[offset] << 8) | pdu_data[offset + 1];
    }

    return MB_SUCCESS;
}

/**
 * @brief Parse write single coil response (FC05)
 */
static int parse_write_single_coil_response(const uint8_t *pdu_data,
                                            uint16_t pdu_length,
                                            uint16_t expected_addr,
                                            bool expected_value) {
    if (pdu_length < 4) {
        return MB_ERROR_INVALID_FRAME;
    }

    uint16_t addr = ((uint16_t)pdu_data[0] << 8) | pdu_data[1];
    uint16_t value = ((uint16_t)pdu_data[2] << 8) | pdu_data[3];

    if (addr != expected_addr) {
        return MB_ERROR_INVALID_FRAME;
    }

    bool actual_value = (value == 0xFF00);
    if (actual_value != expected_value) {
        return MB_ERROR_INVALID_FRAME;
    }

    return MB_SUCCESS;
}

/**
 * @brief Parse write single register response (FC06)
 */
static int parse_write_single_register_response(const uint8_t *pdu_data,
                                                uint16_t pdu_length,
                                                uint16_t expected_addr,
                                                uint16_t expected_value) {
    if (pdu_length < 4) {
        return MB_ERROR_INVALID_FRAME;
    }

    uint16_t addr = ((uint16_t)pdu_data[0] << 8) | pdu_data[1];
    uint16_t value = ((uint16_t)pdu_data[2] << 8) | pdu_data[3];

    if (addr != expected_addr || value != expected_value) {
        return MB_ERROR_INVALID_FRAME;
    }

    return MB_SUCCESS;
}

/**
 * @brief Parse write multiple registers response (FC16)
 */
static int parse_write_multiple_response(const uint8_t *pdu_data,
                                         uint16_t pdu_length,
                                         uint16_t expected_addr,
                                         uint16_t expected_quantity) {
    if (pdu_length < 4) {
        return MB_ERROR_INVALID_FRAME;
    }

    uint16_t addr = ((uint16_t)pdu_data[0] << 8) | pdu_data[1];
    uint16_t quantity = ((uint16_t)pdu_data[2] << 8) | pdu_data[3];

    if (addr != expected_addr || quantity != expected_quantity) {
        return MB_ERROR_INVALID_FRAME;
    }

    return MB_SUCCESS;
}

int mb_parse_read_response(uint8_t fc,
                           const uint8_t *pdu_data,
                           uint16_t pdu_length,
                           uint16_t quantity,
                           void *data_buffer) {
    if (pdu_data == NULL || data_buffer == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Check for exception response
    if (fc & 0x80) {
        if (pdu_length >= 1) {
            // Exception code is in first byte of PDU
            return MB_ERROR_EXCEPTION_RESPONSE;
        }
        return MB_ERROR_INVALID_FRAME;
    }

    switch (fc) {
    case MB_FC_READ_COILS:
    case MB_FC_READ_DISCRETE_INPUTS:
        return parse_read_bits_response(pdu_data, pdu_length, quantity, (uint8_t *)data_buffer);

    case MB_FC_READ_HOLDING_REGISTERS:
    case MB_FC_READ_INPUT_REGISTERS:
        return parse_read_registers_response(pdu_data, pdu_length, quantity, (uint16_t *)data_buffer);

    default:
        return MB_ERROR_INVALID_FC;
    }
}

int mb_parse_write_response(uint8_t fc,
                            const uint8_t *pdu_data,
                            uint16_t pdu_length,
                            uint16_t address,
                            uint16_t quantity,
                            const void *expected_data) {
    if (pdu_data == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Check for exception response
    if (fc & 0x80) {
        if (pdu_length >= 1) {
            return MB_ERROR_EXCEPTION_RESPONSE;
        }
        return MB_ERROR_INVALID_FRAME;
    }

    switch (fc) {
    case MB_FC_WRITE_SINGLE_COIL:
        if (expected_data == NULL) {
            return MB_ERROR_INVALID_PARAM;
        }
        return parse_write_single_coil_response(pdu_data, pdu_length, address,
                                                *(const bool *)expected_data);

    case MB_FC_WRITE_SINGLE_REGISTER:
        if (expected_data == NULL) {
            return MB_ERROR_INVALID_PARAM;
        }
        return parse_write_single_register_response(pdu_data, pdu_length, address,
                                                    *(const uint16_t *)expected_data);

    case MB_FC_WRITE_MULTIPLE_COILS:
    case MB_FC_WRITE_MULTIPLE_REGISTERS:
        return parse_write_multiple_response(pdu_data, pdu_length, address, quantity);

    default:
        return MB_ERROR_INVALID_FC;
    }
}

uint8_t mb_get_exception_code(const uint8_t *pdu_data, uint16_t pdu_length) {
    if (pdu_data == NULL || pdu_length < 1) {
        return 0;
    }
    return pdu_data[0];
}
