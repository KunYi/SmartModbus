/**
 * @file char_model.c
 * @brief Character-based cost model implementation
 *
 * Implements the core cost calculation logic for the Smart Modbus optimization.
 * The cost model uses characters as the universal unit of cost, making it
 * protocol-agnostic and independent of baudrate or timing.
 */

#include "char_model.h"

#include "fc_policy.h"

uint16_t mb_calc_overhead_chars(mb_mode_t mode, uint8_t fc, uint8_t gap_chars, uint8_t latency_chars) {
    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    if (policy == NULL) {
        return 0;
    }

    uint16_t overhead = 0;

    // Request fixed overhead
    overhead += policy->req_fixed_chars;

    // Response fixed overhead
    overhead += policy->resp_fixed_chars;

    // Inter-frame gap (RTU/ASCII only, TCP has no gap)
    if (mode == MB_MODE_RTU || mode == MB_MODE_ASCII) {
        overhead += gap_chars;
    }

    // Network/processing latency
    overhead += latency_chars;

    return overhead;
}

uint16_t mb_calc_gap_cost(uint8_t fc, uint16_t gap_units) {
    if (gap_units == 0) {
        return 0;
    }

    uint16_t extra_unit_chars = mb_fc_get_extra_unit_chars(fc);

    // extra_unit_chars is ×100 for precision
    // For registers: 200 (2.00 bytes)
    // For coils: 12 (0.12 bytes, rounded from 0.125)

    // Calculate: gap_units × extra_unit_chars / 100
    uint32_t cost = ((uint32_t)gap_units * extra_unit_chars) / 100;

    // For bit-based operations, round up
    if (fc == MB_FC_READ_COILS || fc == MB_FC_READ_DISCRETE_INPUTS) {
        // Convert bits to bytes, round up
        cost = (gap_units + 7) / 8;
    } else {
        // Register-based: 2 bytes per register
        cost = gap_units * 2;
    }

    return (uint16_t)cost;
}

uint16_t mb_get_extra_unit_chars(uint8_t fc) {
    return mb_fc_get_extra_unit_chars(fc);
}

uint16_t mb_calc_request_cost(const mb_block_t *block,
                              mb_mode_t mode,
                              uint8_t gap_chars,
                              uint8_t latency_chars) {
    if (block == NULL) {
        return 0;
    }

    // Calculate overhead
    uint16_t overhead = mb_calc_overhead_chars(mode, block->function_code, gap_chars, latency_chars);

    // Calculate data cost
    uint16_t data_cost = 0;
    uint8_t unit_size  = mb_fc_get_unit_size(block->function_code);

    if (unit_size == 1) {
        // Bit-based: convert bits to bytes
        data_cost = (uint16_t)((block->quantity + 7) / 8);
    } else if (unit_size == 2) {
        // Register-based: 2 bytes per register
        data_cost = (uint16_t)(block->quantity * 2);
    }

    return overhead + data_cost;
}

void mb_init_cost_params(mb_mode_t mode,
                         uint8_t fc,
                         uint8_t latency_chars,
                         mb_cost_params_t *params) {
    if (params == NULL) {
        return;
    }

    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    if (policy == NULL) {
        return;
    }

    params->req_fixed_chars  = policy->req_fixed_chars;
    params->resp_fixed_chars = policy->resp_fixed_chars;
    params->latency_chars    = latency_chars;

    // Set gap based on mode
    if (mode == MB_MODE_RTU || mode == MB_MODE_ASCII) {
        params->gap_chars = 4; // 3.5 chars rounded up
    } else {
        params->gap_chars = 0; // TCP has no inter-frame gap
    }
}

int16_t mb_calc_merge_savings(uint16_t gap_units, uint8_t fc, const mb_cost_params_t *cost_params) {
    if (cost_params == NULL) {
        return 0;
    }

    // Calculate overhead cost (cost of additional round-trip)
    uint16_t overhead_cost = cost_params->req_fixed_chars + cost_params->resp_fixed_chars +
                             cost_params->gap_chars + cost_params->latency_chars;

    // Calculate gap cost (cost of reading extra data)
    uint16_t gap_cost = mb_calc_gap_cost(fc, gap_units);

    // Savings = overhead - gap_cost
    // Positive = merging saves characters
    // Negative = merging wastes characters
    return (int16_t)(overhead_cost - gap_cost);
}
