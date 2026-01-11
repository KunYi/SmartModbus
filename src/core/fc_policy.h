/**
 * @file fc_policy.h
 * @brief Function code policy definitions
 *
 * This module defines characteristics and constraints for each Modbus
 * function code, including merge support and cost parameters.
 */

#ifndef SMARTMODBUS_FC_POLICY_H
#define SMARTMODBUS_FC_POLICY_H

#include "smartmodbus/mb_types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get policy for a function code
 * @param fc Function code
 * @return Pointer to policy structure, or NULL if not supported
 */
const mb_fc_policy_t *mb_fc_get_policy(uint8_t fc);

/**
 * @brief Check if function code supports gap-aware merging
 * @param fc Function code
 * @return true if merging is supported, false otherwise
 */
bool mb_fc_supports_merge(uint8_t fc);

/**
 * @brief Check if function code is a read operation
 * @param fc Function code
 * @return true if read operation, false otherwise
 */
bool mb_fc_is_read(uint8_t fc);

/**
 * @brief Check if function code is a write operation
 * @param fc Function code
 * @return true if write operation, false otherwise
 */
bool mb_fc_is_write(uint8_t fc);

/**
 * @brief Get data unit size for function code
 * @param fc Function code
 * @return Unit size: 1 for bits (coils), 2 for registers, 0 if invalid
 */
uint8_t mb_fc_get_unit_size(uint8_t fc);

/**
 * @brief Get extra unit cost for gap calculation
 * @param fc Function code
 * @return Extra unit cost in chars (×100 for precision)
 *
 * For FC03/04 (registers): returns 200 (2.00 bytes per register)
 * For FC01/02 (coils): returns 12 (0.12 bytes per bit, rounded up)
 */
uint16_t mb_fc_get_extra_unit_chars(uint8_t fc);

/**
 * @brief Get maximum quantity for function code
 * @param fc Function code
 * @return Maximum quantity per request
 */
uint16_t mb_fc_get_max_quantity(uint8_t fc);

/**
 * @brief Validate function code
 * @param fc Function code
 * @return true if valid, false otherwise
 */
bool mb_fc_is_valid(uint8_t fc);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_FC_POLICY_H
