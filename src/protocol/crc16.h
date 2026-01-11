/**
 * @file crc16.h
 * @brief CRC16 calculation for Modbus RTU
 *
 * Implements the CRC16-MODBUS algorithm used in Modbus RTU frames.
 */

#ifndef SMARTMODBUS_CRC16_H
#define SMARTMODBUS_CRC16_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate CRC16 for Modbus RTU
 * @param data Data buffer
 * @param length Data length in bytes
 * @return CRC16 value (little-endian)
 */
uint16_t mb_crc16(const uint8_t *data, size_t length);

/**
 * @brief Verify CRC16 in RTU frame
 * @param frame Complete RTU frame including CRC
 * @param length Frame length in bytes
 * @return true if CRC is valid, false otherwise
 */
bool mb_crc16_verify(const uint8_t *frame, size_t length);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_CRC16_H
