/**
 * @file tcp_frame.c
 * @brief TCP/IP frame implementation
 *
 * Implements Modbus TCP/IP frame encoding and decoding with MBAP header.
 * Frame format: [TransID:2][ProtoID:2][Length:2][UnitID:1][FC:1][PDU:N]
 */

#include "tcp_frame.h"

#include "smartmodbus/mb_error.h"

#include <stddef.h>
#include <string.h>

int mb_tcp_build_frame(uint16_t transaction_id,
                       uint8_t unit_id,
                       uint8_t fc,
                       const uint8_t *pdu_data,
                       uint16_t pdu_length,
                       uint8_t *frame_buffer,
                       uint16_t buffer_size) {
    if (frame_buffer == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Calculate required size: MBAP header(7) + FC(1) + PDU
    uint16_t required_size = 7 + 1 + pdu_length;

    if (buffer_size < required_size) {
        return MB_ERROR_BUFFER_TOO_SMALL;
    }

    uint16_t pos = 0;

    // 1. Transaction ID (2 bytes, big-endian)
    frame_buffer[pos++] = (uint8_t)((transaction_id >> 8) & 0xFF);
    frame_buffer[pos++] = (uint8_t)(transaction_id & 0xFF);

    // 2. Protocol ID (2 bytes, always 0x0000 for Modbus)
    frame_buffer[pos++] = 0x00;
    frame_buffer[pos++] = 0x00;

    // 3. Length field (2 bytes, big-endian): UnitID(1) + FC(1) + PDU length
    uint16_t length = 1 + 1 + pdu_length;
    frame_buffer[pos++] = (uint8_t)((length >> 8) & 0xFF);
    frame_buffer[pos++] = (uint8_t)(length & 0xFF);

    // 4. Unit ID (1 byte)
    frame_buffer[pos++] = unit_id;

    // 5. Function code (1 byte)
    frame_buffer[pos++] = fc;

    // 6. PDU data
    if (pdu_data != NULL && pdu_length > 0) {
        memcpy(&frame_buffer[pos], pdu_data, pdu_length);
        pos += pdu_length;
    }

    return (int)pos;
}

int mb_tcp_parse_frame(const uint8_t *frame_data,
                       uint16_t frame_length,
                       uint16_t *transaction_id,
                       uint8_t *unit_id,
                       uint8_t *fc,
                       uint8_t *pdu_data,
                       uint16_t *pdu_length) {
    if (frame_data == NULL || transaction_id == NULL || unit_id == NULL ||
        fc == NULL || pdu_length == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Minimum frame: MBAP header(7) + FC(1) = 8 bytes
    if (frame_length < 8) {
        return MB_ERROR_INVALID_FRAME;
    }

    uint16_t pos = 0;

    // 1. Parse transaction ID (big-endian)
    *transaction_id = ((uint16_t)frame_data[pos] << 8) | frame_data[pos + 1];
    pos += 2;

    // 2. Parse protocol ID (should be 0x0000)
    uint16_t protocol_id = ((uint16_t)frame_data[pos] << 8) | frame_data[pos + 1];
    pos += 2;
    if (protocol_id != 0x0000) {
        return MB_ERROR_INVALID_FRAME;
    }

    // 3. Parse length field (big-endian)
    uint16_t length = ((uint16_t)frame_data[pos] << 8) | frame_data[pos + 1];
    pos += 2;

    // Verify length matches frame
    if (pos + length != frame_length) {
        return MB_ERROR_INVALID_FRAME;
    }

    // 4. Parse unit ID
    *unit_id = frame_data[pos++];

    // 5. Parse function code
    *fc = frame_data[pos++];

    // 6. Calculate PDU length: length - UnitID(1) - FC(1)
    *pdu_length = length - 2;

    // 7. Copy PDU data if buffer provided
    if (pdu_data != NULL && *pdu_length > 0) {
        memcpy(pdu_data, &frame_data[pos], *pdu_length);
    }

    return MB_SUCCESS;
}

uint16_t mb_tcp_calc_frame_length(uint16_t pdu_length) {
    // MBAP header(7) + FC(1) + PDU
    return 7 + 1 + pdu_length;
}
