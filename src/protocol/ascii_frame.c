/**
 * @file ascii_frame.c
 * @brief ASCII frame implementation
 *
 * Implements Modbus ASCII frame encoding and decoding.
 * Frame format: [':'][SlaveID:2hex][FC:2hex][PDU:2Nhex][LRC:2hex][CR][LF]
 */

#include "ascii_frame.h"

#include "lrc.h"
#include "smartmodbus/mb_error.h"

#include <ctype.h>
#include <string.h>

/**
 * @brief Convert byte to 2 ASCII hex characters
 */
static void byte_to_hex(uint8_t byte, char *hex) {
    const char hex_chars[] = "0123456789ABCDEF";
    hex[0] = hex_chars[(byte >> 4) & 0x0F];
    hex[1] = hex_chars[byte & 0x0F];
}

/**
 * @brief Convert 2 ASCII hex characters to byte
 */
static int hex_to_byte(const char *hex, uint8_t *byte) {
    uint8_t high, low;

    // Convert high nibble
    if (hex[0] >= '0' && hex[0] <= '9') {
        high = hex[0] - '0';
    } else if (hex[0] >= 'A' && hex[0] <= 'F') {
        high = hex[0] - 'A' + 10;
    } else if (hex[0] >= 'a' && hex[0] <= 'f') {
        high = hex[0] - 'a' + 10;
    } else {
        return -1;
    }

    // Convert low nibble
    if (hex[1] >= '0' && hex[1] <= '9') {
        low = hex[1] - '0';
    } else if (hex[1] >= 'A' && hex[1] <= 'F') {
        low = hex[1] - 'A' + 10;
    } else if (hex[1] >= 'a' && hex[1] <= 'f') {
        low = hex[1] - 'a' + 10;
    } else {
        return -1;
    }

    *byte = (high << 4) | low;
    return 0;
}

int mb_ascii_build_frame(uint8_t slave_id,
                         uint8_t fc,
                         const uint8_t *pdu_data,
                         uint16_t pdu_length,
                         uint8_t *frame_buffer,
                         uint16_t buffer_size) {
    if (frame_buffer == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Calculate required size: ':'(1) + SlaveID(2) + FC(2) + PDU(2N) + LRC(2) + CRLF(2)
    uint16_t required_size = 1 + 2 + 2 + (pdu_length * 2) + 2 + 2;

    if (buffer_size < required_size) {
        return MB_ERROR_BUFFER_TOO_SMALL;
    }

    uint16_t pos = 0;

    // 1. Start character
    frame_buffer[pos++] = ':';

    // 2. Slave ID (2 hex chars)
    byte_to_hex(slave_id, (char *)&frame_buffer[pos]);
    pos += 2;

    // 3. Function code (2 hex chars)
    byte_to_hex(fc, (char *)&frame_buffer[pos]);
    pos += 2;

    // 4. PDU data (2 hex chars per byte)
    for (uint16_t i = 0; i < pdu_length; i++) {
        byte_to_hex(pdu_data[i], (char *)&frame_buffer[pos]);
        pos += 2;
    }

    // 5. Calculate LRC on binary data (slave_id + fc + pdu_data)
    uint8_t lrc_data[256];
    uint16_t lrc_len = 0;
    lrc_data[lrc_len++] = slave_id;
    lrc_data[lrc_len++] = fc;
    if (pdu_data != NULL && pdu_length > 0) {
        memcpy(&lrc_data[lrc_len], pdu_data, pdu_length);
        lrc_len += pdu_length;
    }
    uint8_t lrc = mb_lrc(lrc_data, lrc_len);

    // 6. Append LRC (2 hex chars)
    byte_to_hex(lrc, (char *)&frame_buffer[pos]);
    pos += 2;

    // 7. CR LF
    frame_buffer[pos++] = '\r';
    frame_buffer[pos++] = '\n';

    return (int)pos;
}

int mb_ascii_parse_frame(const uint8_t *frame_data,
                         uint16_t frame_length,
                         uint8_t *slave_id,
                         uint8_t *fc,
                         uint8_t *pdu_data,
                         uint16_t *pdu_length) {
    if (frame_data == NULL || slave_id == NULL || fc == NULL || pdu_length == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Minimum frame: ':'(1) + SlaveID(2) + FC(2) + LRC(2) + CRLF(2) = 9 bytes
    if (frame_length < 9) {
        return MB_ERROR_INVALID_FRAME;
    }

    // Check start character
    if (frame_data[0] != ':') {
        return MB_ERROR_INVALID_FRAME;
    }

    // Check end characters
    if (frame_data[frame_length - 2] != '\r' || frame_data[frame_length - 1] != '\n') {
        return MB_ERROR_INVALID_FRAME;
    }

    uint16_t pos = 1; // Skip ':'

    // Parse slave ID
    if (hex_to_byte((const char *)&frame_data[pos], slave_id) != 0) {
        return MB_ERROR_INVALID_FRAME;
    }
    pos += 2;

    // Parse function code
    if (hex_to_byte((const char *)&frame_data[pos], fc) != 0) {
        return MB_ERROR_INVALID_FRAME;
    }
    pos += 2;

    // Calculate PDU length: (frame_length - start(1) - slaveID(2) - FC(2) - LRC(2) - CRLF(2)) / 2
    uint16_t pdu_hex_len = frame_length - 1 - 2 - 2 - 2 - 2;
    *pdu_length = pdu_hex_len / 2;

    // Parse PDU data
    if (pdu_data != NULL && *pdu_length > 0) {
        for (uint16_t i = 0; i < *pdu_length; i++) {
            if (hex_to_byte((const char *)&frame_data[pos], &pdu_data[i]) != 0) {
                return MB_ERROR_INVALID_FRAME;
            }
            pos += 2;
        }
    } else {
        pos += pdu_hex_len;
    }

    // Parse LRC
    uint8_t frame_lrc;
    if (hex_to_byte((const char *)&frame_data[pos], &frame_lrc) != 0) {
        return MB_ERROR_INVALID_FRAME;
    }

    // Verify LRC
    uint8_t lrc_data[256];
    uint16_t lrc_len = 0;
    lrc_data[lrc_len++] = *slave_id;
    lrc_data[lrc_len++] = *fc;
    if (pdu_data != NULL && *pdu_length > 0) {
        memcpy(&lrc_data[lrc_len], pdu_data, *pdu_length);
        lrc_len += *pdu_length;
    }
    uint8_t calculated_lrc = mb_lrc(lrc_data, lrc_len);

    if (calculated_lrc != frame_lrc) {
        return MB_ERROR_LRC_MISMATCH;
    }

    return MB_SUCCESS;
}

uint16_t mb_ascii_calc_frame_length(uint16_t pdu_length) {
    // ':'(1) + SlaveID(2) + FC(2) + PDU(2N) + LRC(2) + CRLF(2)
    return 1 + 2 + 2 + (pdu_length * 2) + 2 + 2;
}
