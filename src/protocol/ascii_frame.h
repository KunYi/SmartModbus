/**
 * @file ascii_frame.h
 * @brief ASCII frame encoding/decoding
 */

#ifndef SMARTMODBUS_ASCII_FRAME_H
#define SMARTMODBUS_ASCII_FRAME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build ASCII frame
 * @param slave_id Slave device ID
 * @param fc Function code
 * @param pdu_data PDU data
 * @param pdu_length PDU length
 * @param frame_buffer Output frame buffer
 * @param buffer_size Buffer size
 * @return Frame length on success, negative error code on failure
 */
int mb_ascii_build_frame(uint8_t slave_id,
                         uint8_t fc,
                         const uint8_t *pdu_data,
                         uint16_t pdu_length,
                         uint8_t *frame_buffer,
                         uint16_t buffer_size);

/**
 * @brief Parse ASCII frame
 * @param frame_data Frame data
 * @param frame_length Frame length
 * @param slave_id Output: slave ID
 * @param fc Output: function code
 * @param pdu_data Output: PDU data buffer
 * @param pdu_length Output: PDU length
 * @return 0 on success, negative error code on failure
 */
int mb_ascii_parse_frame(const uint8_t *frame_data,
                         uint16_t frame_length,
                         uint8_t *slave_id,
                         uint8_t *fc,
                         uint8_t *pdu_data,
                         uint16_t *pdu_length);

/**
 * @brief Calculate ASCII frame length
 * @param pdu_length PDU length
 * @return Total frame length
 */
uint16_t mb_ascii_calc_frame_length(uint16_t pdu_length);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_ASCII_FRAME_H
