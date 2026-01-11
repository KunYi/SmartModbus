/**
 * @file gap_merge.h
 * @brief Gap-aware merge algorithm for Smart Modbus
 *
 * This module implements the core optimization algorithm that decides whether
 * to merge non-contiguous blocks based on gap cost analysis.
 *
 * Merge condition: gap_units × EXTRA_UNIT_CHAR < OVERHEAD_CHAR
 */

#ifndef SMARTMODBUS_GAP_MERGE_H
#define SMARTMODBUS_GAP_MERGE_H

#include "smartmodbus/mb_types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check if two blocks should be merged based on cost analysis
 * @param block_a First block (must come before block_b)
 * @param block_b Second block
 * @param cost_params Cost calculation parameters
 * @return true if merging is beneficial, false otherwise
 *
 * Merges if: gap_cost < overhead_cost
 * This means reading extra data is cheaper than an additional round-trip.
 */
bool mb_should_merge_blocks(const mb_block_t *block_a,
                            const mb_block_t *block_b,
                            const mb_cost_params_t *cost_params);

/**
 * @brief Merge two blocks into one
 * @param block_a First block
 * @param block_b Second block
 * @param result Output merged block
 * @return 0 on success, negative error code on failure
 */
int mb_merge_two_blocks(const mb_block_t *block_a, const mb_block_t *block_b, mb_block_t *result);

/**
 * @brief Process array of blocks and merge where beneficial
 * @param blocks Array of blocks (will be modified in-place)
 * @param block_count Pointer to block count (updated after merging)
 * @param cost_params Cost calculation parameters
 * @return 0 on success, negative error code on failure
 *
 * Algorithm:
 * 1. Sort blocks by address
 * 2. For each adjacent pair, check if merging is beneficial
 * 3. If yes, merge and continue
 * 4. Return compacted array of merged blocks
 */
int mb_merge_block_array(mb_block_t *blocks, uint16_t *block_count, const mb_cost_params_t *cost_params);

/**
 * @brief Merge all compatible blocks in array (greedy approach)
 * @param blocks Array of blocks
 * @param block_count Pointer to block count
 * @param mode Protocol mode
 * @param latency_chars Latency in characters
 * @return Number of merges performed
 *
 * This is a convenience function that initializes cost params and performs merging.
 */
uint16_t mb_merge_blocks_greedy(mb_block_t *blocks,
                                uint16_t *block_count,
                                mb_mode_t mode,
                                uint8_t latency_chars);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_GAP_MERGE_H
