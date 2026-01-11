/**
 * @file ffd_pack.c
 * @brief First-Fit Decreasing packing algorithm implementation
 *
 * Implements FFD bin packing to maximize PDU utilization while respecting
 * protocol constraints.
 */

#include "ffd_pack.h"

#include "../utils/block_utils.h"
#include "fc_policy.h"
#include "smartmodbus/mb_error.h"

#include <stdlib.h>
#include <string.h>

void mb_init_pdu(mb_pdu_t *pdu, uint8_t slave_id, uint8_t fc) {
    if (pdu == NULL) {
        return;
    }

    pdu->slave_id      = slave_id;
    pdu->function_code = fc;
    pdu->start_address = 0;
    pdu->quantity      = 0;
    pdu->total_chars   = 0;
}

uint16_t mb_calc_pdu_data_size(const mb_pdu_t *pdu) {
    if (pdu == NULL || pdu->quantity == 0) {
        return 0;
    }

    uint8_t unit_size = mb_fc_get_unit_size(pdu->function_code);

    if (unit_size == 1) {
        // Bit-based: convert bits to bytes
        return (uint16_t)((pdu->quantity + 7) / 8);
    } else if (unit_size == 2) {
        // Register-based: 2 bytes per register
        return (uint16_t)(pdu->quantity * 2);
    }

    return 0;
}

bool mb_block_fits_pdu(const mb_block_t *block, const mb_pdu_t *pdu, uint16_t max_pdu_chars) {
    if (block == NULL || pdu == NULL) {
        return false;
    }

    // Empty PDU always fits
    if (pdu->quantity == 0) {
        uint16_t block_size = mb_block_calc_data_size(block);
        return block_size <= max_pdu_chars;
    }

    // Must have same slave and function code
    if (block->slave_id != pdu->slave_id || block->function_code != pdu->function_code) {
        return false;
    }

    // Calculate merged range
    uint16_t min_addr = (block->start_address < pdu->start_address) ? block->start_address
                                                                      : pdu->start_address;
    uint32_t block_end = (uint32_t)block->start_address + block->quantity;
    uint32_t pdu_end   = (uint32_t)pdu->start_address + pdu->quantity;
    uint32_t max_end   = (block_end > pdu_end) ? block_end : pdu_end;

    uint16_t merged_quantity = (uint16_t)(max_end - min_addr);

    // Check against function code max quantity
    uint16_t max_quantity = mb_fc_get_max_quantity(block->function_code);
    if (merged_quantity > max_quantity) {
        return false;
    }

    // Calculate merged data size
    uint8_t unit_size = mb_fc_get_unit_size(block->function_code);
    uint16_t data_size;

    if (unit_size == 1) {
        data_size = (uint16_t)((merged_quantity + 7) / 8);
    } else if (unit_size == 2) {
        data_size = (uint16_t)(merged_quantity * 2);
    } else {
        return false;
    }

    // Check if it fits in PDU
    return data_size <= max_pdu_chars;
}

int mb_add_block_to_pdu(const mb_block_t *block, mb_pdu_t *pdu) {
    if (block == NULL || pdu == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // If PDU is empty, initialize with block
    if (pdu->quantity == 0) {
        pdu->slave_id      = block->slave_id;
        pdu->function_code = block->function_code;
        pdu->start_address = block->start_address;
        pdu->quantity      = block->quantity;
        pdu->total_chars   = mb_block_calc_data_size(block);
        return MB_SUCCESS;
    }

    // Merge block into PDU
    uint16_t min_addr = (block->start_address < pdu->start_address) ? block->start_address
                                                                      : pdu->start_address;
    uint32_t block_end = (uint32_t)block->start_address + block->quantity;
    uint32_t pdu_end   = (uint32_t)pdu->start_address + pdu->quantity;
    uint32_t max_end   = (block_end > pdu_end) ? block_end : pdu_end;

    pdu->start_address = min_addr;
    pdu->quantity      = (uint16_t)(max_end - min_addr);
    pdu->total_chars   = mb_calc_pdu_data_size(pdu);

    return MB_SUCCESS;
}

float mb_calc_pdu_utilization(const mb_pdu_t *pdu, uint16_t max_pdu_chars) {
    if (pdu == NULL || max_pdu_chars == 0) {
        return 0.0f;
    }

    return (float)pdu->total_chars / (float)max_pdu_chars * 100.0f;
}

int mb_ffd_pack(const mb_block_t *blocks,
                uint16_t block_count,
                uint16_t max_pdu_chars,
                mb_pdu_t *pdus,
                uint16_t max_pdus,
                uint16_t *pdu_count) {
    if (blocks == NULL || pdus == NULL || pdu_count == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    if (block_count == 0) {
        *pdu_count = 0;
        return MB_SUCCESS;
    }

    // Create a working copy of blocks for sorting
    mb_block_t *sorted_blocks = (mb_block_t *)malloc(block_count * sizeof(mb_block_t));
    if (sorted_blocks == NULL) {
        return MB_ERROR_OUT_OF_MEMORY;
    }

    memcpy(sorted_blocks, blocks, block_count * sizeof(mb_block_t));

    // Sort blocks by quantity (descending) for FFD
    mb_block_sort_by_quantity_desc(sorted_blocks, block_count);

    // Initialize PDU count
    uint16_t num_pdus = 0;

    // FFD algorithm: for each block, find first PDU that fits
    for (uint16_t i = 0; i < block_count; i++) {
        const mb_block_t *block = &sorted_blocks[i];
        bool placed             = false;

        // Try to fit in existing PDUs (first-fit)
        for (uint16_t j = 0; j < num_pdus; j++) {
            if (mb_block_fits_pdu(block, &pdus[j], max_pdu_chars)) {
                // Add block to this PDU
                int result = mb_add_block_to_pdu(block, &pdus[j]);
                if (result != MB_SUCCESS) {
                    free(sorted_blocks);
                    return result;
                }
                placed = true;
                break;
            }
        }

        // If not placed, create new PDU
        if (!placed) {
            if (num_pdus >= max_pdus) {
                free(sorted_blocks);
                return MB_ERROR_TOO_MANY_BLOCKS;
            }

            // Initialize new PDU with this block
            mb_init_pdu(&pdus[num_pdus], block->slave_id, block->function_code);
            int result = mb_add_block_to_pdu(block, &pdus[num_pdus]);
            if (result != MB_SUCCESS) {
                free(sorted_blocks);
                return result;
            }
            num_pdus++;
        }
    }

    *pdu_count = num_pdus;

    free(sorted_blocks);
    return MB_SUCCESS;
}
