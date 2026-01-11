/**
 * @file request_optimizer.c
 * @brief Request optimization pipeline implementation (stub)
 */

#include "request_optimizer.h"

#include "../core/char_model.h"
#include "../core/ffd_pack.h"
#include "../core/gap_merge.h"
#include "../utils/block_utils.h"
#include "smartmodbus/mb_config.h"
#include "smartmodbus/mb_error.h"

#include <stdlib.h>

int mb_optimize_request(const mb_read_request_t *request,
                        const mb_config_t *config,
                        mb_request_plan_t *plans,
                        uint16_t max_plans,
                        uint16_t *plan_count) {
    if (request == NULL || config == NULL || plans == NULL || plan_count == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    if (request->address_count == 0) {
        *plan_count = 0;
        return MB_SUCCESS;
    }

    if (max_plans == 0) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Step 1: Convert addresses to blocks
    mb_block_t *blocks = NULL;
    uint16_t block_count = 0;

#ifdef MB_USE_STATIC_MEMORY
    mb_block_t static_blocks[MB_MAX_BLOCKS];
    blocks = static_blocks;
    uint16_t max_blocks = MB_MAX_BLOCKS;
#else
    uint16_t max_blocks = request->address_count;  // Worst case: one block per address
    blocks = (mb_block_t *)malloc(max_blocks * sizeof(mb_block_t));
    if (blocks == NULL) {
        return MB_ERROR_NO_MEMORY;
    }
#endif

    int result = mb_addresses_to_blocks(request->slave_id, request->function_code, request->addresses,
                                        request->address_count, blocks, max_blocks, &block_count);

    if (result != MB_SUCCESS) {
#ifndef MB_USE_STATIC_MEMORY
        free(blocks);
#endif
        return result;
    }

    // Step 2: Sort blocks by address (already done by mb_addresses_to_blocks)

    // Step 3: Apply gap-aware merge
    mb_cost_params_t cost_params;
    mb_init_cost_params(config->mode, request->function_code, config->latency_chars, &cost_params);

    result = mb_merge_block_array(blocks, &block_count, &cost_params);

    if (result != MB_SUCCESS) {
#ifndef MB_USE_STATIC_MEMORY
        free(blocks);
#endif
        return result;
    }

    // Step 4: Apply FFD packing
    mb_pdu_t *pdus = NULL;
    uint16_t pdu_count = 0;

#ifdef MB_USE_STATIC_MEMORY
    mb_pdu_t static_pdus[MB_MAX_PDUS];
    pdus = static_pdus;
    uint16_t max_pdus = MB_MAX_PDUS;
#else
    uint16_t max_pdus = block_count;  // Worst case: one PDU per block
    pdus = (mb_pdu_t *)malloc(max_pdus * sizeof(mb_pdu_t));
    if (pdus == NULL) {
#ifndef MB_USE_STATIC_MEMORY
        free(blocks);
#endif
        return MB_ERROR_NO_MEMORY;
    }
#endif

    result = mb_ffd_pack(blocks, block_count, config->max_pdu_chars, pdus, max_pdus, &pdu_count);

    if (result != MB_SUCCESS) {
#ifndef MB_USE_STATIC_MEMORY
        free(blocks);
        free(pdus);
#endif
        return result;
    }

    // Step 5: Generate request plans from PDUs
    if (pdu_count > max_plans) {
#ifndef MB_USE_STATIC_MEMORY
        free(blocks);
        free(pdus);
#endif
        return MB_ERROR_TOO_MANY_PLANS;
    }

    for (uint16_t i = 0; i < pdu_count; i++) {
        plans[i].slave_id = pdus[i].slave_id;
        plans[i].function_code = pdus[i].function_code;
        plans[i].start_address = pdus[i].start_address;
        plans[i].quantity = pdus[i].quantity;
        plans[i].frame_data = NULL;  // Will be built by master API
        plans[i].frame_length = 0;
        plans[i].expected_response_length = 0;  // Will be calculated by master API
    }

    *plan_count = pdu_count;

#ifndef MB_USE_STATIC_MEMORY
    free(blocks);
    free(pdus);
#endif

    return MB_SUCCESS;
}
