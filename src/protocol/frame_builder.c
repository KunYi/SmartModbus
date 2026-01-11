/**
 * @file frame_builder.c
 * @brief Frame builder orchestrator implementation
 *
 * Dispatches frame building to appropriate protocol-specific builder.
 */

#include "frame_builder.h"

#include "ascii_frame.h"
#include "rtu_frame.h"
#include "tcp_frame.h"
#include "smartmodbus/mb_error.h"

int mb_build_frame(uint8_t slave_id,
                   uint8_t fc,
                   const uint8_t *pdu_data,
                   uint16_t pdu_length,
                   mb_mode_t mode,
                   uint8_t *frame_buffer,
                   uint16_t buffer_size,
                   uint16_t *frame_length) {
    if (frame_buffer == NULL || frame_length == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    int result;

    switch (mode) {
#ifdef MB_ENABLE_RTU
    case MB_MODE_RTU:
        result = mb_rtu_build_frame(slave_id, fc, pdu_data, pdu_length, frame_buffer, buffer_size);
        break;
#endif

#ifdef MB_ENABLE_ASCII
    case MB_MODE_ASCII:
        result = mb_ascii_build_frame(slave_id, fc, pdu_data, pdu_length, frame_buffer, buffer_size);
        break;
#endif

#ifdef MB_ENABLE_TCP
    case MB_MODE_TCP:
        // For TCP, use transaction_id = 0 (will be set by master)
        result = mb_tcp_build_frame(0, slave_id, fc, pdu_data, pdu_length, frame_buffer, buffer_size);
        break;
#endif

    default:
        return MB_ERROR_NOT_SUPPORTED;
    }

    if (result > 0) {
        *frame_length = (uint16_t)result;
        return MB_SUCCESS;
    }

    return result;
}

int mb_parse_frame(const uint8_t *frame_data,
                   uint16_t frame_length,
                   mb_mode_t mode,
                   uint8_t *slave_id,
                   uint8_t *fc,
                   uint8_t *pdu_data,
                   uint16_t *pdu_length) {
    if (frame_data == NULL || slave_id == NULL || fc == NULL || pdu_length == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    switch (mode) {
#ifdef MB_ENABLE_RTU
    case MB_MODE_RTU:
        return mb_rtu_parse_frame(frame_data, frame_length, slave_id, fc, pdu_data, pdu_length);
#endif

#ifdef MB_ENABLE_ASCII
    case MB_MODE_ASCII:
        return mb_ascii_parse_frame(frame_data, frame_length, slave_id, fc, pdu_data, pdu_length);
#endif

#ifdef MB_ENABLE_TCP
    case MB_MODE_TCP: {
        uint16_t transaction_id;
        return mb_tcp_parse_frame(frame_data, frame_length, &transaction_id, slave_id, fc, pdu_data,
                                  pdu_length);
    }
#endif

    default:
        return MB_ERROR_NOT_SUPPORTED;
    }
}

uint16_t mb_calc_frame_length(uint16_t pdu_length, mb_mode_t mode) {
    switch (mode) {
#ifdef MB_ENABLE_RTU
    case MB_MODE_RTU:
        return mb_rtu_calc_frame_length(pdu_length);
#endif

#ifdef MB_ENABLE_ASCII
    case MB_MODE_ASCII:
        return mb_ascii_calc_frame_length(pdu_length);
#endif

#ifdef MB_ENABLE_TCP
    case MB_MODE_TCP:
        return mb_tcp_calc_frame_length(pdu_length);
#endif

    default:
        return 0;
    }
}
