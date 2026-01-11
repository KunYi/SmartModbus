/**
 * @file frame_builder.h
 * @brief Frame builder orchestrator for all protocols
 */

#ifndef SMARTMODBUS_FRAME_BUILDER_H
#define SMARTMODBUS_FRAME_BUILDER_H

#include "smartmodbus/mb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build Modbus frame for any protocol
 * @param slave_id Slave device ID
 * @param fc Function code
 * @param pdu_data PDU data
 * @param pdu_length PDU length
 * @param mode Protocol mode
 * @param frame_buffer Output frame buffer
 * @param buffer_size Buffer size
 * @param frame_length Output: actual frame length
 * @return 0 on success, negative error code on failure
 */
int mb_build_frame(uint8_t slave_id,
                   uint8_t fc,
                   const uint8_t *pdu_data,
                   uint16_t pdu_length,
                   mb_mode_t mode,
                   uint8_t *frame_buffer,
                   uint16_t buffer_size,
                   uint16_t *frame_length);

/**
 * @brief Parse Modbus frame for any protocol
 * @param frame_data Frame data
 * @param frame_length Frame length
 * @param mode Protocol mode
 * @param slave_id Output: slave ID
 * @param fc Output: function code
 * @param pdu_data Output: PDU data
 * @param pdu_length Output: PDU length
 * @return 0 on success, negative error code on failure
 */
int mb_parse_frame(const uint8_t *frame_data,
                   uint16_t frame_length,
                   mb_mode_t mode,
                   uint8_t *slave_id,
                   uint8_t *fc,
                   uint8_t *pdu_data,
                   uint16_t *pdu_length);

/**
 * @brief Calculate frame length for any protocol
 * @param pdu_length PDU length
 * @param mode Protocol mode
 * @return Total frame length
 */
uint16_t mb_calc_frame_length(uint16_t pdu_length, mb_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_FRAME_BUILDER_H
