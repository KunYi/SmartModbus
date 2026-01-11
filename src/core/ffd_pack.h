/**
 * @file ffd_pack.h
 * @brief First-Fit Decreasing (FFD) packing algorithm
 *
 * This module implements the FFD bin packing algorithm to pack merged blocks
 * into PDU frames, maximizing utilization while respecting the MAX_PDU_CHAR limit.
 */

#ifndef SMARTMODBUS_FFD_PACK_H
#define SMARTMODBUS_FFD_PACK_H

#include "smartmodbus/mb_types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pack blocks into PDUs using First-Fit Decreasing algorithm
 * @param blocks Array of blocks to pack
 * @param block_count Number of blocks
 * @param max_pdu_chars Maximum PDU size in characters
 * @param pdus Output array of PDUs
 * @param max_pdus Maximum number of PDUs
 * @param pdu_count Output: actual number of PDUs created
 * @return 0 on success, negative error code on failure
 *
 * Algorithm:
 * 1. Sort blocks by data length (descending)
 * 2. For each block:
 *    - Try to fit in existing PDU (first-fit)
 *    - If no fit, create new PDU
 * 3. Constraints: same FC, same slave, within MAX_PDU_CHAR
 */
int mb_ffd_pack(const mb_block_t *blocks,
                uint16_t block_count,
                uint16_t max_pdu_chars,
                mb_pdu_t *pdus,
                uint16_t max_pdus,
                uint16_t *pdu_count);

/**
 * @brief Check if block fits in PDU
 * @param block Block to check
 * @param pdu PDU to check against
 * @param max_pdu_chars Maximum PDU size
 * @return true if block fits, false otherwise
 */
bool mb_block_fits_pdu(const mb_block_t *block, const mb_pdu_t *pdu, uint16_t max_pdu_chars);

/**
 * @brief Calculate PDU utilization percentage
 * @param pdu PDU to calculate utilization for
 * @param max_pdu_chars Maximum PDU size
 * @return Utilization percentage (0-100)
 */
float mb_calc_pdu_utilization(const mb_pdu_t *pdu, uint16_t max_pdu_chars);

/**
 * @brief Add block to PDU
 * @param block Block to add
 * @param pdu PDU to add to
 * @return 0 on success, negative error code on failure
 */
int mb_add_block_to_pdu(const mb_block_t *block, mb_pdu_t *pdu);

/**
 * @brief Initialize empty PDU
 * @param pdu PDU to initialize
 * @param slave_id Slave device ID
 * @param fc Function code
 */
void mb_init_pdu(mb_pdu_t *pdu, uint8_t slave_id, uint8_t fc);

/**
 * @brief Calculate PDU data size in characters
 * @param pdu PDU to calculate size for
 * @return Data size in characters
 */
uint16_t mb_calc_pdu_data_size(const mb_pdu_t *pdu);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_FFD_PACK_H
