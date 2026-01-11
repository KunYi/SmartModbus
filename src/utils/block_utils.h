/**
 * @file block_utils.h
 * @brief Utility functions for block manipulation
 *
 * This module provides helper functions for working with Modbus data blocks.
 */

#ifndef SMARTMODBUS_BLOCK_UTILS_H
#define SMARTMODBUS_BLOCK_UTILS_H

#include "smartmodbus/mb_types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sort blocks by start address (ascending)
 * @param blocks Array of blocks
 * @param count Number of blocks
 */
void mb_block_sort_by_address(mb_block_t *blocks, uint16_t count);

/**
 * @brief Sort blocks by quantity (descending) for FFD packing
 * @param blocks Array of blocks
 * @param count Number of blocks
 */
void mb_block_sort_by_quantity_desc(mb_block_t *blocks, uint16_t count);

/**
 * @brief Check if two blocks are adjacent (no gap)
 * @param a First block
 * @param b Second block
 * @return true if blocks are adjacent, false otherwise
 */
bool mb_block_are_adjacent(const mb_block_t *a, const mb_block_t *b);

/**
 * @brief Calculate gap between two blocks
 * @param a First block
 * @param b Second block
 * @return Gap size in units (registers or coils), 0 if overlapping/adjacent
 */
uint16_t mb_block_calc_gap(const mb_block_t *a, const mb_block_t *b);

/**
 * @brief Check if two blocks can be merged (same slave, same FC)
 * @param a First block
 * @param b Second block
 * @return true if blocks are compatible for merging, false otherwise
 */
bool mb_block_are_compatible(const mb_block_t *a, const mb_block_t *b);

/**
 * @brief Merge two blocks into one
 * @param a First block
 * @param b Second block
 * @param result Merged block output
 * @return 0 on success, negative error code on failure
 */
int mb_block_merge(const mb_block_t *a, const mb_block_t *b, mb_block_t *result);

/**
 * @brief Calculate data size for a block in bytes
 * @param block Block to calculate size for
 * @return Data size in bytes
 */
uint16_t mb_block_calc_data_size(const mb_block_t *block);

/**
 * @brief Validate block parameters
 * @param block Block to validate
 * @return 0 if valid, negative error code otherwise
 */
int mb_block_validate(const mb_block_t *block);

/**
 * @brief Convert array of addresses to blocks
 * @param addresses Array of addresses
 * @param count Number of addresses
 * @param slave_id Slave device ID
 * @param fc Function code
 * @param blocks Output array of blocks
 * @param max_blocks Maximum number of blocks
 * @param block_count Output: actual number of blocks created
 * @return 0 on success, negative error code on failure
 */
int mb_addresses_to_blocks(const uint16_t *addresses,
                           uint16_t count,
                           uint8_t slave_id,
                           uint8_t fc,
                           mb_block_t *blocks,
                           uint16_t max_blocks,
                           uint16_t *block_count);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_BLOCK_UTILS_H
