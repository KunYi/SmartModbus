/**
 * @file block_utils.c
 * @brief Utility functions for block manipulation implementation
 */

#include "block_utils.h"

#include "../core/fc_policy.h"
#include "smartmodbus/mb_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Comparison function for sorting blocks by address
 */
static int compare_blocks_by_address(const void *a, const void *b) {
    const mb_block_t *block_a = (const mb_block_t *)a;
    const mb_block_t *block_b = (const mb_block_t *)b;

    if (block_a->start_address < block_b->start_address) {
        return -1;
    } else if (block_a->start_address > block_b->start_address) {
        return 1;
    }
    return 0;
}

/**
 * @brief Comparison function for sorting blocks by quantity (descending)
 */
static int compare_blocks_by_quantity_desc(const void *a, const void *b) {
    const mb_block_t *block_a = (const mb_block_t *)a;
    const mb_block_t *block_b = (const mb_block_t *)b;

    if (block_a->quantity > block_b->quantity) {
        return -1;
    } else if (block_a->quantity < block_b->quantity) {
        return 1;
    }
    return 0;
}

void mb_block_sort_by_address(mb_block_t *blocks, uint16_t count) {
    if (blocks == NULL || count == 0) {
        return;
    }
    qsort(blocks, count, sizeof(mb_block_t), compare_blocks_by_address);
}

void mb_block_sort_by_quantity_desc(mb_block_t *blocks, uint16_t count) {
    if (blocks == NULL || count == 0) {
        return;
    }
    qsort(blocks, count, sizeof(mb_block_t), compare_blocks_by_quantity_desc);
}

bool mb_block_are_adjacent(const mb_block_t *a, const mb_block_t *b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    // Must be compatible first
    if (!mb_block_are_compatible(a, b)) {
        return false;
    }

    // Check if b starts right after a ends
    uint32_t a_end = (uint32_t)a->start_address + a->quantity;
    return (a_end == b->start_address);
}

uint16_t mb_block_calc_gap(const mb_block_t *a, const mb_block_t *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    // Ensure a comes before b
    if (a->start_address > b->start_address) {
        const mb_block_t *temp = a;
        a                      = b;
        b                      = temp;
    }

    uint32_t a_end = (uint32_t)a->start_address + a->quantity;

    // If overlapping or adjacent, no gap
    if (a_end >= b->start_address) {
        return 0;
    }

    return (uint16_t)(b->start_address - a_end);
}

bool mb_block_are_compatible(const mb_block_t *a, const mb_block_t *b) {
    if (a == NULL || b == NULL) {
        return false;
    }

    // Must have same slave ID and function code
    return (a->slave_id == b->slave_id) && (a->function_code == b->function_code);
}

int mb_block_merge(const mb_block_t *a, const mb_block_t *b, mb_block_t *result) {
    if (a == NULL || b == NULL || result == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Check compatibility
    if (!mb_block_are_compatible(a, b)) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Ensure a comes before b
    if (a->start_address > b->start_address) {
        const mb_block_t *temp = a;
        a                      = b;
        b                      = temp;
    }

    // Calculate merged block
    result->slave_id      = a->slave_id;
    result->function_code = a->function_code;
    result->start_address = a->start_address;

    uint32_t a_end = (uint32_t)a->start_address + a->quantity;
    uint32_t b_end = (uint32_t)b->start_address + b->quantity;
    uint32_t end   = (a_end > b_end) ? a_end : b_end;

    result->quantity  = (uint16_t)(end - result->start_address);
    result->is_merged = true;

    return MB_SUCCESS;
}

uint16_t mb_block_calc_data_size(const mb_block_t *block) {
    if (block == NULL) {
        return 0;
    }

    uint8_t unit_size = mb_fc_get_unit_size(block->function_code);

    if (unit_size == 1) {
        // Bit-based (coils): convert bits to bytes, round up
        return (uint16_t)((block->quantity + 7) / 8);
    } else if (unit_size == 2) {
        // Register-based: 2 bytes per register
        return (uint16_t)(block->quantity * 2);
    }

    return 0;
}

int mb_block_validate(const mb_block_t *block) {
    if (block == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Validate slave ID (1-247 for RTU/ASCII, 0-255 for TCP)
    if (block->slave_id == 0 || block->slave_id > 247) {
        return MB_ERROR_INVALID_ADDRESS;
    }

    // Validate function code
    if (!mb_fc_is_valid(block->function_code)) {
        return MB_ERROR_INVALID_FC;
    }

    // Validate quantity
    if (block->quantity == 0) {
        return MB_ERROR_INVALID_QUANTITY;
    }

    uint16_t max_quantity = mb_fc_get_max_quantity(block->function_code);
    if (block->quantity > max_quantity) {
        return MB_ERROR_INVALID_QUANTITY;
    }

    // Check for address overflow
    uint32_t end_address = (uint32_t)block->start_address + block->quantity;
    if (end_address > 0x10000) {
        return MB_ERROR_INVALID_ADDRESS;
    }

    return MB_SUCCESS;
}

int mb_addresses_to_blocks(const uint16_t *addresses,
                           uint16_t count,
                           uint8_t slave_id,
                           uint8_t fc,
                           mb_block_t *blocks,
                           uint16_t max_blocks,
                           uint16_t *block_count) {
    if (addresses == NULL || blocks == NULL || block_count == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    if (count == 0) {
        *block_count = 0;
        return MB_SUCCESS;
    }

    if (!mb_fc_is_valid(fc)) {
        return MB_ERROR_INVALID_FC;
    }

    // Create a sorted copy of addresses
    uint16_t *sorted_addresses = (uint16_t *)malloc(count * sizeof(uint16_t));
    if (sorted_addresses == NULL) {
        return MB_ERROR_OUT_OF_MEMORY;
    }

    memcpy(sorted_addresses, addresses, count * sizeof(uint16_t));

    // Simple bubble sort for addresses
    for (uint16_t i = 0; i < count - 1; i++) {
        for (uint16_t j = 0; j < count - i - 1; j++) {
            if (sorted_addresses[j] > sorted_addresses[j + 1]) {
                uint16_t temp          = sorted_addresses[j];
                sorted_addresses[j]     = sorted_addresses[j + 1];
                sorted_addresses[j + 1] = temp;
            }
        }
    }

    // Group consecutive addresses into blocks
    uint16_t num_blocks     = 0;
    uint16_t block_start    = sorted_addresses[0];
    uint16_t block_quantity = 1;

    for (uint16_t i = 1; i < count; i++) {
        // Check if current address is consecutive
        if (sorted_addresses[i] == sorted_addresses[i - 1] + 1) {
            block_quantity++;
        } else {
            // Save current block
            if (num_blocks >= max_blocks) {
                free(sorted_addresses);
                return MB_ERROR_TOO_MANY_BLOCKS;
            }

            blocks[num_blocks].slave_id      = slave_id;
            blocks[num_blocks].function_code = fc;
            blocks[num_blocks].start_address = block_start;
            blocks[num_blocks].quantity      = block_quantity;
            blocks[num_blocks].is_merged     = false;
            num_blocks++;

            // Start new block
            block_start    = sorted_addresses[i];
            block_quantity = 1;
        }
    }

    // Save last block
    if (num_blocks >= max_blocks) {
        free(sorted_addresses);
        return MB_ERROR_TOO_MANY_BLOCKS;
    }

    blocks[num_blocks].slave_id      = slave_id;
    blocks[num_blocks].function_code = fc;
    blocks[num_blocks].start_address = block_start;
    blocks[num_blocks].quantity      = block_quantity;
    blocks[num_blocks].is_merged     = false;
    num_blocks++;

    *block_count = num_blocks;

    free(sorted_addresses);
    return MB_SUCCESS;
}
