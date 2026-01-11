/**
 * @file memory_pool.h
 * @brief Static memory pool allocator for embedded systems
 *
 * Provides fixed-size memory pools for blocks, PDUs, and plans
 * when MB_USE_STATIC_MEMORY is defined.
 */

#ifndef SMARTMODBUS_MEMORY_POOL_H
#define SMARTMODBUS_MEMORY_POOL_H

#include "smartmodbus/mb_types.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MB_USE_STATIC_MEMORY

/**
 * @brief Memory pool for blocks
 */
typedef struct {
    mb_block_t blocks[MB_MAX_BLOCKS];
    bool used[MB_MAX_BLOCKS];
    uint16_t count;
} mb_block_pool_t;

/**
 * @brief Memory pool for PDUs
 */
typedef struct {
    mb_pdu_t pdus[MB_MAX_PDUS];
    bool used[MB_MAX_PDUS];
    uint16_t count;
} mb_pdu_pool_t;

/**
 * @brief Memory pool for request plans
 */
typedef struct {
    mb_request_plan_t plans[MB_MAX_PLANS];
    bool used[MB_MAX_PLANS];
    uint16_t count;
} mb_plan_pool_t;

/**
 * @brief Initialize block pool
 * @param pool Block pool to initialize
 */
void mb_block_pool_init(mb_block_pool_t *pool);

/**
 * @brief Allocate a block from pool
 * @param pool Block pool
 * @return Pointer to allocated block, or NULL if pool is full
 */
mb_block_t *mb_block_pool_alloc(mb_block_pool_t *pool);

/**
 * @brief Free a block back to pool
 * @param pool Block pool
 * @param block Block to free
 */
void mb_block_pool_free(mb_block_pool_t *pool, mb_block_t *block);

/**
 * @brief Get number of free blocks
 * @param pool Block pool
 * @return Number of free blocks
 */
uint16_t mb_block_pool_available(const mb_block_pool_t *pool);

/**
 * @brief Initialize PDU pool
 * @param pool PDU pool to initialize
 */
void mb_pdu_pool_init(mb_pdu_pool_t *pool);

/**
 * @brief Allocate a PDU from pool
 * @param pool PDU pool
 * @return Pointer to allocated PDU, or NULL if pool is full
 */
mb_pdu_t *mb_pdu_pool_alloc(mb_pdu_pool_t *pool);

/**
 * @brief Free a PDU back to pool
 * @param pool PDU pool
 * @param pdu PDU to free
 */
void mb_pdu_pool_free(mb_pdu_pool_t *pool, mb_pdu_t *pdu);

/**
 * @brief Get number of free PDUs
 * @param pool PDU pool
 * @return Number of free PDUs
 */
uint16_t mb_pdu_pool_available(const mb_pdu_pool_t *pool);

/**
 * @brief Initialize plan pool
 * @param pool Plan pool to initialize
 */
void mb_plan_pool_init(mb_plan_pool_t *pool);

/**
 * @brief Allocate a plan from pool
 * @param pool Plan pool
 * @return Pointer to allocated plan, or NULL if pool is full
 */
mb_request_plan_t *mb_plan_pool_alloc(mb_plan_pool_t *pool);

/**
 * @brief Free a plan back to poolam pool Plan pool
 * @param plan Plan to free
 */
void mb_plan_pool_free(mb_plan_pool_t *pool, mb_request_plan_t *plan);

/**
 * @brief Get number of free plans
 * @param pool Plan pool
 * @return Number of free plans
 */
uint16_t mb_plan_pool_available(const mb_plan_pool_t *pool);

#endif  // MB_USE_STATIC_MEMORY

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_MEMORY_POOL_H
