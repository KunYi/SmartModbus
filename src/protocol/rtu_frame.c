/**
 * @file rtu_frame.c
 * @brief RTU frame implementation
 *
 * Implements Modbus RTU frame encoding and decoding.
 * Frame format: [SlaveID:1][FC:1][PDU:N][CRC16:2]
 */

#include "rtu_frame.h"

#include "crc16.h"
#include "smartmodbus/mb_error.h"

#include <string.h>

int mb_rtu_build_frame(uint8_t slave_id,
                       uint8_t fc,
                       const uint8_t *pdu_data,
                       uint16_t pdu_length,
                       uint8_t *frame_buffer,
                       uint16_t buffer_size) {
    if (frame_buffer == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Calculate required buffer size: SlaveID(1) + FC(1) + PDU + CRC(2)
    uint16_t required_size = 1 + 1 + pdu_length + 2;

    if (buffer_size < required_size) {
        return MB_ERROR_BUFFER_TOO_SMALL;
    }

    // Build frame
    uint16_t pos = 0;

    // 1. Write slave ID
    frame_buffer[pos++] = slave_id;

    // 2. Write function code
    frame_buffer[pos++] = fc;

    // 3. Write PDU data
    if (pdu_data != NULL && pdu_length > 0) {
        memcpy(&frame_buffer[pos], pdu_data, pdu_length);
        pos += pdu_length;
    }

    // 4. Calculate and append CRC16 (little-endian)
    uint16_t crc = mb_crc16(frame_buffer, pos);
    frame_buffer[pos++] = (uint8_t)(crc & 0xFF);        // CRC low byte
    frame_buffer[pos++] = (uint8_t)((crc >> 8) & 0xFF); // CRC high byte

    return (int)pos; // Return frame length
}

int mb_rtu_parse_frame(const uint8_t *frame_data,
                       uint16_t frame_length,
                       uint8_t *slave_id,
                       uint8_t *fc,
                       uint8_t *pdu_data,
                       uint16_t *pdu_length) {
    if (frame_data == NULL || slave_id == NULL || fc == NULL || pdu_length == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Minimum frame: SlaveID(1) + FC(1) + CRC(2) = 4 bytes
    if (frame_length < 4) {
        return MB_ERROR_INVALID_FRAME;
    }

    // Verify CRC
    if (!mb_crc16_verify(frame_data, frame_length)) {
        return MB_ERROR_CRC_MISMATCH;
    }

    // Parse frame
    *slave_id = frame_data[0];
    *fc = frame_data[1];

    // PDU length = frame_length - SlaveID(1) - FC(1) - CRC(2)
    *pdu_length = frame_length - 4;

    // Copy PDU data if buffer provided
    if (pdu_data != NULL && *pdu_length > 0) {
        memcpy(pdu_data, &frame_data[2], *pdu_length);
    }

    return MB_SUCCESS;
}

uint16_t mb_rtu_calc_frame_length(uint16_t pdu_length) {
    // SlaveID(1) + FC(1) + PDU + CRC(2)
    return 1 + 1 + pdu_length + 2;
}
