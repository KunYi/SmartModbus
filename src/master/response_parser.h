/**
 * @file response_parser.h
 * @brief Response parsing and validation
 */

#ifndef SMARTMODBUS_RESPONSE_PARSER_H
#define SMARTMODBUS_RESPONSE_PARSER_H

#include "smartmodbus/mb_types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse read response (FC01/02/03/04)
 * @param fc Function code
 * @param pdu_data PDU data (without slave ID and FC)
 * @param pdu_length PDU length
 * @param quantity Expected quantity
 * @param data_buffer Output buffer (uint8_t* for bits, uint16_t* for registers)
 * @return 0 on success, negative error code on failure
 */
int mb_parse_read_response(uint8_t fc,
                           const uint8_t *pdu_data,
                           uint16_t pdu_length,
                           uint16_t quantity,
                           void *data_buffer);

/**
 * @brief Parse write response (FC05/06/15/16)
 * @param fc Function code
 * @param pdu_data PDU data
 * @param pdu_length PDU length
 * @param address Expected address
 * @param quantity Expected quantity
 * @param expected_data Expected data for verification (bool* for FC05, uint16_t* for FC06)
 * @return 0 on success, negative error code on failure
 */
int mb_parse_write_response(uint8_t fc,
                            const uint8_t *pdu_data,
                            uint16_t pdu_length,
                            uint16_t address,
                            uint16_t quantity,
                            const void *expected_data);

/**
 * @brief Get exception code from exception response
 * @param pdu_data PDU data
 * @param pdu_length PDU length
 * @return Exception code, or 0 if invalid
 */
uint8_t mb_get_exception_code(const uint8_t *pdu_data, uint16_t pdu_length);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_RESPONSE_PARSER_H
