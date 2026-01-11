/**
 * @file fc_policy.c
 * @brief Function code policy implementation
 *
 * Implements the policy table for all Modbus function codes, defining
 * their characteristics, merge support, and cost parameters.
 */

#include "fc_policy.h"

#include <stddef.h>

/**
 * @brief Function code policy table
 *
 * This table defines the characteristics of each supported Modbus function code.
 * The extra_unit_chars values are multiplied by 100 for precision (e.g., 200 = 2.00 bytes).
 */
static const mb_fc_policy_t fc_policy_table[] = {
    // FC01: Read Coils - bit-based, full merge support
    {.fc                = 0x01,
     .supports_merge    = true,
     .is_read           = true,
     .req_fixed_chars   = 6,  // Addr(1) + FC(1) + StartAddr(2) + Quantity(2)
     .resp_fixed_chars  = 5,  // Addr(1) + FC(1) + ByteCount(1) + CRC(2)
     .extra_unit_chars  = 12, // 1 bit = 1/8 byte = 0.125, rounded up (×100 = 12)
     .max_quantity      = 2000},

    // FC02: Read Discrete Inputs - bit-based, full merge support
    {.fc                = 0x02,
     .supports_merge    = true,
     .is_read           = true,
     .req_fixed_chars   = 6,
     .resp_fixed_chars  = 5,
     .extra_unit_chars  = 12, // 1 bit = 1/8 byte
     .max_quantity      = 2000},

    // FC03: Read Holding Registers - register-based, full merge support
    {.fc                = 0x03,
     .supports_merge    = true,
     .is_read           = true,
     .req_fixed_chars   = 6,
     .resp_fixed_chars  = 5,
     .extra_unit_chars  = 200, // 2 bytes per register (×100 = 200)
     .max_quantity      = 125},

    // FC04: Read Input Registers - register-based, full merge support
    {.fc                = 0x04,
     .supports_merge    = true,
     .is_read           = true,
     .req_fixed_chars   = 6,
     .resp_fixed_chars  = 5,
     .extra_unit_chars  = 200, // 2 bytes per register
     .max_quantity      = 125},

    // FC05: Write Single Coil - no merge support
    {.fc                = 0x05,
     .supports_merge    = false,
     .is_read           = false,
     .req_fixed_chars   = 6,
     .resp_fixed_chars  = 6,
     .extra_unit_chars  = 0,
     .max_quantity      = 1},

    // FC06: Write Single Register - no merge support
    {.fc                = 0x06,
     .supports_merge    = false,
     .is_read           = false,
     .req_fixed_chars   = 6,
     .resp_fixed_chars  = 6,
     .extra_unit_chars  = 0,
     .max_quantity      = 1},

    // FC15: Write Multiple Coils - contiguous only
    {.fc                = 0x0F,
     .supports_merge    = false, // Only contiguous blocks
     .is_read           = false,
     .req_fixed_chars   = 7, // Addr(1) + FC(1) + StartAddr(2) + Quantity(2) + ByteCount(1)
     .resp_fixed_chars  = 6,
     .extra_unit_chars  = 0,
     .max_quantity      = 1968},

    // FC16: Write Multiple Registers - contiguous only
    {.fc                = 0x10,
     .supports_merge    = false, // Only contiguous blocks
     .is_read           = false,
     .req_fixed_chars   = 7,
     .resp_fixed_chars  = 6,
     .extra_unit_chars  = 0,
     .max_quantity      = 123},

    // FC22: Mask Write Register - no merge support
    {.fc                = 0x16,
     .supports_merge    = false,
     .is_read           = false,
     .req_fixed_chars   = 8,
     .resp_fixed_chars  = 8,
     .extra_unit_chars  = 0,
     .max_quantity      = 1},

    // FC23: Read/Write Multiple Registers - special handling
    {.fc                = 0x17,
     .supports_merge    = false,
     .is_read           = true, // Primarily a read operation
     .req_fixed_chars   = 11,
     .resp_fixed_chars  = 5,
     .extra_unit_chars  = 0,
     .max_quantity      = 121},
};

#define FC_POLICY_TABLE_SIZE (sizeof(fc_policy_table) / sizeof(fc_policy_table[0]))

const mb_fc_policy_t *mb_fc_get_policy(uint8_t fc) {
    for (size_t i = 0; i < FC_POLICY_TABLE_SIZE; i++) {
        if (fc_policy_table[i].fc == fc) {
            return &fc_policy_table[i];
        }
    }
    return NULL;
}

bool mb_fc_supports_merge(uint8_t fc) {
    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    return policy != NULL && policy->supports_merge;
}

bool mb_fc_is_read(uint8_t fc) {
    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    return policy != NULL && policy->is_read;
}

bool mb_fc_is_write(uint8_t fc) {
    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    return policy != NULL && !policy->is_read;
}

uint8_t mb_fc_get_unit_size(uint8_t fc) {
    switch (fc) {
    case 0x01: // Read Coils
    case 0x02: // Read Discrete Inputs
    case 0x05: // Write Single Coil
    case 0x0F: // Write Multiple Coils
        return 1; // Bit-based (coils)

    case 0x03: // Read Holding Registers
    case 0x04: // Read Input Registers
    case 0x06: // Write Single Register
    case 0x10: // Write Multiple Registers
    case 0x16: // Mask Write Register
    case 0x17: // Read/Write Multiple Registers
        return 2; // Register-based (16-bit words)

    default:
        return 0; // Invalid or unsupported
    }
}

uint16_t mb_fc_get_extra_unit_chars(uint8_t fc) {
    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    return policy != NULL ? policy->extra_unit_chars : 0;
}

uint16_t mb_fc_get_max_quantity(uint8_t fc) {
    const mb_fc_policy_t *policy = mb_fc_get_policy(fc);
    return policy != NULL ? policy->max_quantity : 0;
}

bool mb_fc_is_valid(uint8_t fc) {
    return mb_fc_get_policy(fc) != NULL;
}
