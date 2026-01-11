/**
 * @file memory_pool.c
 * @brief Static memory pool allocator implementation
 *
 * Provides simple fixed-size memory pools for embedded systems
 * that cannot use dynamic memory allocation.
 */

#include "memory_pool.h"

#include <string.h>

#ifdef MB_USE_STATIC_MEMORY

// Block pool functions
void mb_block_pool_init(mb_block_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

    memset(pool->used, 0, sizeof(pool->used));
    pool->count = 0;
}

mb_block_t *mb_block_pool_alloc(mb_block_pool_t *pool) {
    if (pool == NULL) {
        return NULL;
    }

    // Find first free slot
    for (uint16_t i = 0; i < MB_MAX_BLOCKS; i++) {
        if (!pool->used[i]) {
            pool->used[i] = true;
            pool->count++;
            memset(&pool->blocks[i], 0, sizeof(mb_block_t));
            return &pool->blocks[i];
        }
    }

    return NULL;  // Pool is full
}

void mb_block_pool_free(mb_block_pool_t *pool, mb_block_t *block) {
    if (pool == NULL || block == NULL) {
        return;
    }

    // Find block in pool
    for (uint16_t i = 0; i < MB_MAX_BLOCKS; i++) {
        if (&pool->blocks[i] == block) {
            pool->used[i] = false;
            if (pool->count > 0) {
                pool->count--;
            }
            return;
        }
    }
}

uint16_t mb_block_pool_available(const mb_block_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }

    return MB_MAX_BLOCKS - pool->count;
}

// PDU pool functions
void mb_pdu_pool_init(mb_pdu_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

    memset(pool->used, 0, sizeof(pool->used));
    pool->count = 0;
}

mb_pdu_t *mb_pdu_pool_alloc(mb_pdu_pool_t *pool) {
    if (pool == NULL) {
        return NULL;
    }

    // Find first free slot
    for (uint16_t i = 0; i < MB_MAX_PDUS; i++) {
        if (!pool->used[i]) {
            pool->used[i] = true;
            pool->count++;
            memset(&pool->pdus[i], 0, sizeof(mb_pdu_t));
            return &pool->pdus[i];
        }
    }

    return NULL;  // Pool is full
}

void mb_pdu_pool_free(mb_pdu_pool_t *pool, mb_pdu_t *pdu) {
    if (pool == NULL || pdu == NULL) {
        return;
    }

    // Find PDU in pool
    for (uint16_t i = 0; i < MB_MAX_PDUS; i++) {
        if (&pool->pdus[i] == pdu) {
            pool->used[i] = false;
            if (pool->count > 0) {
                pool->count--;
            }
            return;
        }
    }
}

uint16_t mb_pdu_pool_available(const mb_pdu_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }

    return MB_MAX_PDUS - pool->count;
}

// Plan pool functions
void mb_plan_pool_init(mb_plan_pool_t *pool) {
    if (pool == NULL) {
        return;
    }

    memset(pool->used, 0, sizeof(pool->used));
    pool->count = 0;
}

mb_request_plan_t *mb_plan_pool_alloc(mb_plan_pool_t *pool) {
    if (pool == NULL) {
        return NULL;
    }

    // Find first free slot
    for (uint16_t i = 0; i < MB_MAX_PLANS; i++) {
        if (!pool->used[i]) {
            pool->used[i] = true;
            pool->count++;
            memset(&pool->plans[i], 0, sizeof(mb_request_plan_t));
            return &pool->plans[i];
        }
    }

    return NULL;  // Pool is full
}

void mb_plan_pool_free(mb_plan_pool_t *pool, mb_request_plan_t *plan) {
    if (pool == NULL || plan == NULL) {
        return;
    }

    // Find plan in pool
    for (uint16_t i = 0; i < MB_MAX_PLANS; i++) {
        if (&pool->plans[i] == plan) {
            pool->used[i] = false;
            if (pool->count > 0) {
                pool->count--;
            }
            return;
        }
    }
}

uint16_t mb_plan_pool_available(const mb_plan_pool_t *pool) {
    if (pool == NULL) {
        return 0;
    }

    return MB_MAX_PLANS - pool->count;
}

#endif  // MB_USE_STATIC_MEMORY
