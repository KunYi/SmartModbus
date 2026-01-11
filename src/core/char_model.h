/**
 * @file char_model.h
 * @brief Character-based cost model for Modbus optimization
 *
 * This module implements the core cost calculation engine that determines
 * whether merging non-contiguous blocks is beneficial based on the
 * character-based cost model.
 */

#ifndef SMARTMODBUS_CHAR_MODEL_H
#define SMARTMODBUS_CHAR_MODEL_H

#include "smartmodbus/mb_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate overhead cost for a single round-trip
 * @param mode Protocol mode (RTU/ASCII/TCP)
 * @param fc Function code
 * @param gap_chars Inter-frame gap in characters
 * @param latency_chars Network/processing latency in characters
 * @return Total overhead in characters
 *
 * Formula: OVERHEAD_CHAR = REQ_FIXED + RESP_FIXED + GAP_CHAR + LATENCY_CHAR
 */
uint16_t mb_calc_overhead_chars(mb_mode_t mode, uint8_t fc, uint8_t gap_chars, uint8_t latency_chars);

/**
 * @brief Calculate cost of reading extra data in a gap
 * @param fc Function code
 * @param gap_units Gap size in units (registers or coils)
 * @return Gap cost in characters (×100 for precision)
 *
 * For FC03/04: gap_units × 2 bytes per register
 * For FC01/02: gap_units × 1/8 byte per bit (rounded up)
 */
uint16_t mb_calc_gap_cost(uint8_t fc, uint16_t gap_units);

/**
 * @brief Get extra unit cost for function code
 * @param fc Function code
 * @return Extra unit cost in chars (×100 for precision)
 */
uint16_t mb_get_extra_unit_chars(uint8_t fc);

/**
 * @brief Calculate total request cost for a block
 * @param block Block to calculate cost for
 * @param mode Protocol mode
 * @param gap_chars Inter-frame gap
 * @param latency_chars Network latency
 * @return Total cost in characters
 */
uint16_t mb_calc_request_cost(const mb_block_t *block,
                              mb_mode_t mode,
                              uint8_t gap_chars,
                              uint8_t latency_chars);

/**
 * @brief Initialize cost parameters for a given mode
 * @param mode Protocol mode
 * @param fc Function code
 * @param latency_chars User-configured latency
 * @param params Output cost parameters
 */
void mb_init_cost_params(mb_mode_t mode,
                         uint8_t fc,
                         uint8_t latency_chars,
                         mb_cost_params_t *params);

/**
 * @brief Calculate savings from merging two blocks
 * @param gap_units Gap between blocks in units
 * @param fc Function code
 * @param cost_params Cost parameters
 * @return Savings in characters (positive = beneficial, negative = wasteful)
 *
 * Savings = OVERHEAD_CHAR - gap_cost
 * If positive, merging saves characters
 */
int16_t mb_calc_merge_savings(uint16_t gap_units, uint8_t fc, const mb_cost_params_t *cost_params);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_CHAR_MODEL_H
