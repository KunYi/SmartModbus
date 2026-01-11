/**
 * @file lrc.c
 * @brief LRC calculation implementation for Modbus ASCII
 *
 * Implements Longitudinal Redundancy Check (LRC) for Modbus ASCII.
 * LRC is calculated as the two's complement of the sum of all bytes.
 */

#include "lrc.h"

uint8_t mb_lrc(const uint8_t *data, size_t length) {
    if (data == NULL || length == 0) {
        return 0;
    }

    uint8_t lrc = 0;

    // Sum all bytes
    for (size_t i = 0; i < length; i++) {
        lrc += data[i];
    }

    // Two's complement
    lrc = (uint8_t)(-((int8_t)lrc));

    return lrc;
}

bool mb_lrc_verify(const uint8_t *frame, size_t length) {
    if (frame == NULL || length < 2) {
        return false;
    }

    // Calculate LRC for all bytes except last one (which contains the LRC)
    uint8_t calculated_lrc = mb_lrc(frame, length - 1);

    // Extract LRC from frame
    uint8_t frame_lrc = frame[length - 1];

    return calculated_lrc == frame_lrc;
}
