/**
 * @file lrc.h
 * @brief LRC (Longitudinal Redundancy Check) calculation for Modbus ASCII
 *
 * Implements the LRC algorithm used in Modbus ASCII frames.
 */

#ifndef SMARTMODBUS_LRC_H
#define SMARTMODBUS_LRC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate LRC for Modbus ASCII
 * @param data Data buffer
 * @param length Data length in bytes
 * @return LRC value (8-bit)
 */
uint8_t mb_lrc(const uint8_t *data, size_t length);

/**
 * @brief Verify LRC in ASCII frame
 * @param frame Complete ASCII frame including LRC
 * @param length Frame length in bytes
 * @return true if LRC is valid, false otherwise
 */
bool mb_lrc_verify(const uint8_t *frame, size_t length);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_LRC_H
