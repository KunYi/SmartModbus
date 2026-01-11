/**
 * @file gap_merge.c
 * @brief Gap-aware merge algorithm implementation
 *
 * Implements the core Smart Modbus optimization: merging non-contiguous
 * blocks when the gap cost is less than the overhead of an additional round-trip.
 */

#include "gap_merge.h"

#include "../utils/block_utils.h"
#include "char_model.h"
#include "fc_policy.h"
#include "smartmodbus/mb_error.h"

#include <string.h>

bool mb_should_merge_blocks(const mb_block_t *block_a,
                            const mb_block_t *block_b,
                            const mb_cost_params_t *cost_params) {
    if (block_a == NULL || block_b == NULL || cost_params == NULL) {
        return false;
    }

    // Must be compatible (same slave, same FC)
    if (!mb_block_are_compatible(block_a, block_b)) {
        return false;
    }

    // Function code must support merging
    if (!mb_fc_supports_merge(block_a->function_code)) {
        return false;
    }

    // If blocks are adjacent, always merge (no gap cost)
    if (mb_block_are_adjacent(block_a, block_b)) {
        return true;
    }

    // Calculate gap between blocks
    uint16_t gap_units = mb_block_calc_gap(block_a, block_b);
    if (gap_units == 0) {
        return true; // Adjacent or overlapping
    }

    // Calculate merge savings
    int16_t savings = mb_calc_merge_savings(gap_units, block_a->function_code, cost_params);

    // Merge if savings are positive (gap cost < overhead cost)
    return savings > 0;
}

int mb_merge_two_blocks(const mb_block_t *block_a, const mb_block_t *block_b, mb_block_t *result) {
    return mb_block_merge(block_a, block_b, result);
}

int mb_merge_block_array(mb_block_t *blocks, uint16_t *block_count, const mb_cost_params_t *cost_params) {
    if (blocks == NULL || block_count == NULL || cost_params == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    if (*block_count == 0) {
        return MB_SUCCESS;
    }

    // Sort blocks by address first
    mb_block_sort_by_address(blocks, *block_count);

    // Merge pass: iterate through blocks and merge where beneficial
    uint16_t write_idx = 0;
    uint16_t read_idx  = 0;

    while (read_idx < *block_count) {
        // Start with current block
        mb_block_t current = blocks[read_idx];
        read_idx++;

        // Try to merge with subsequent blocks
        while (read_idx < *block_count) {
            mb_block_t *next = &blocks[read_idx];

            // Check if we should merge current with next
            if (mb_should_merge_blocks(&current, next, cost_params)) {
                // Merge next into current
                mb_block_t merged;
                int result = mb_merge_two_blocks(&current, next, &merged);
                if (result != MB_SUCCESS) {
                    return result;
                }
                current = merged;
                read_idx++;
            } else {
                // Can't merge, stop trying
                break;
            }
        }

        // Write merged block
        blocks[write_idx] = current;
        write_idx++;
    }

    // Update block count
    *block_count = write_idx;

    return MB_SUCCESS;
}

uint16_t mb_merge_blocks_greedy(mb_block_t *blocks,
                                uint16_t *block_count,
                                mb_mode_t mode,
                                uint8_t latency_chars) {
    if (blocks == NULL || block_count == NULL || *block_count == 0) {
        return 0;
    }

    uint16_t original_count = *block_count;

    // Get function code from first block (assume all same FC for now)
    uint8_t fc = blocks[0].function_code;

    // Initialize cost parameters
    mb_cost_params_t cost_params;
    mb_init_cost_params(mode, fc, latency_chars, &cost_params);

    // Perform merging
    int result = mb_merge_block_array(blocks, block_count, &cost_params);
    if (result != MB_SUCCESS) {
        return 0;
    }

    // Return number of merges performed
    return original_count - *block_count;
}
