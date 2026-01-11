/**
 * @file mb_config.h
 * @brief Configuration structures for Smart Modbus library
 *
 * This file defines configuration structures used to initialize and
 * configure the Smart Modbus master.
 */

#ifndef SMARTMODBUS_MB_CONFIG_H
#define SMARTMODBUS_MB_CONFIG_H

#include "mb_transport.h"
#include "mb_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Smart Modbus configuration
 *
 * Configuration parameters for initializing a Modbus master instance.
 */
typedef struct {
    mb_mode_t mode;             /**< Protocol mode (RTU/ASCII/TCP) */
    uint16_t max_pdu_chars;     /**< Maximum PDU size (default: 253) */
    uint8_t gap_chars;          /**< Inter-frame gap (RTU/ASCII: 4, TCP: 0) */
    uint8_t latency_chars;      /**< Network/processing latency equivalent */
    mb_transport_t transport;   /**< Transport layer callbacks */
    uint32_t timeout_ms;        /**< Response timeout in milliseconds */
} mb_config_t;

/**
 * @brief Smart Modbus master context
 *
 * Master instance that maintains state and statistics.
 */
typedef struct {
    mb_config_t config;         /**< Configuration */
    uint16_t transaction_id;    /**< Transaction ID for TCP/IP */
    mb_stats_t stats;           /**< Statistics */

#ifdef MB_USE_STATIC_MEMORY
    // Static memory pools
    mb_block_t block_pool[MB_MAX_BLOCKS];
    mb_pdu_t pdu_pool[MB_MAX_PDUS];
    mb_request_plan_t plan_pool[MB_MAX_PLANS];
    uint8_t frame_buffers[MB_MAX_PLANS][MB_MAX_PDU_CHARS + 20];  // Extra for headers
    uint16_t block_pool_used;
    uint16_t pdu_pool_used;
    uint16_t plan_pool_used;
#endif
} mb_master_t;

/**
 * @brief Create default configuration
 * @param mode Protocol mode (RTU/ASCII/TCP)
 * @return Default configuration for specified mode
 */
mb_config_t mb_config_default(mb_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_MB_CONFIG_H
