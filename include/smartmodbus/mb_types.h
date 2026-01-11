/**
 * @file mb_types.h
 * @brief Core data types for Smart Modbus library
 *
 * This file defines all fundamental data structures used throughout the library.
 */

#ifndef SMARTMODBUS_MB_TYPES_H
#define SMARTMODBUS_MB_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Modbus protocol mode
 */
typedef enum {
    MB_MODE_RTU,   /**< Modbus RTU (binary) */
    MB_MODE_ASCII, /**< Modbus ASCII (hex encoded) */
    MB_MODE_TCP    /**< Modbus TCP/IP */
} mb_mode_t;

/**
 * @brief Modbus function codes
 */
typedef enum {
    MB_FC_READ_COILS = 0x01,                  /**< Read Coils */
    MB_FC_READ_DISCRETE_INPUTS = 0x02,        /**< Read Discrete Inputs */
    MB_FC_READ_HOLDING_REGISTERS = 0x03,      /**< Read Holding Registers */
    MB_FC_READ_INPUT_REGISTERS = 0x04,        /**< Read Input Registers */
    MB_FC_WRITE_SINGLE_COIL = 0x05,           /**< Write Single Coil */
    MB_FC_WRITE_SINGLE_REGISTER = 0x06,       /**< Write Single Register */
    MB_FC_WRITE_MULTIPLE_COILS = 0x0F,        /**< Write Multiple Coils */
    MB_FC_WRITE_MULTIPLE_REGISTERS = 0x10,    /**< Write Multiple Registers */
    MB_FC_MASK_WRITE_REGISTER = 0x16,         /**< Mask Write Register */
    MB_FC_READ_WRITE_MULTIPLE_REGISTERS = 0x17 /**< Read/Write Multiple Registers */
} mb_function_code_t;

/**
 * @brief Modbus exception codes
 */
typedef enum {
    MB_EX_ILLEGAL_FUNCTION = 0x01,        /**< Function code not supported */
    MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,    /**< Address not allowed */
    MB_EX_ILLEGAL_DATA_VALUE = 0x03,      /**< Value not allowed */
    MB_EX_SLAVE_DEVICE_FAILURE = 0x04,    /**< Unrecoverable error */
    MB_EX_ACKNOWLEDGE = 0x05,             /**< Long duration command accepted */
    MB_EX_SLAVE_DEVICE_BUSY = 0x06,       /**< Slave is busy */
    MB_EX_MEMORY_PARITY_ERROR = 0x08,     /**< Memory parity error */
    MB_EX_GATEWAY_PATH_UNAVAILABLE = 0x0A, /**< Gateway path unavailable */
    MB_EX_GATEWAY_TARGET_FAILED = 0x0B    /**< Gateway target device failed */
} mb_exception_code_t;

/**
 * @brief Data block representing a contiguous address range
 *
 * A block represents a contiguous range of Modbus addresses to read or write.
 * Blocks can be merged during optimization if the gap cost is less than
 * the overhead of an additional round-trip.
 */
typedef struct {
    uint8_t slave_id;       /**< Slave device ID (1-247) */
    uint8_t function_code;  /**< Modbus function code */
    uint16_t start_address; /**< Starting address */
    uint16_t quantity;      /**< Number of coils/registers */
    bool is_merged;         /**< Flag indicating if block was created by merging */
} mb_block_t;

/**
 * @brief PDU (Protocol Data Unit) container
 *
 * A PDU contains one or more blocks that will be sent in a single request.
 * The FFD packing algorithm fills PDUs to maximize utilization while
 * respecting the MAX_PDU_CHAR limit.
 */
typedef struct {
    uint8_t slave_id;       /**< Slave device ID */
    uint8_t function_code;  /**< Modbus function code */
    uint16_t start_address; /**< Starting address of merged blocks */
    uint16_t quantity;      /**< Total quantity across all blocks */
    uint16_t total_chars;   /**< Total character count for this PDU */
} mb_pdu_t;

/**
 * @brief User read request (semantic layer)
 *
 * Users specify what data they need using potentially non-contiguous addresses.
 * The library automatically optimizes this into minimal round-trips.
 */
typedef struct {
    uint8_t slave_id;         /**< Slave device ID */
    uint8_t function_code;    /**< Modbus function code (01-04) */
    uint16_t *addresses;      /**< Array of addresses (can be non-contiguous) */
    uint16_t address_count;   /**< Number of addresses */
} mb_read_request_t;

/**
 * @brief Optimized request plan (output)
 *
 * Represents a single optimized request that will be executed.
 * The optimization pipeline generates an array of these plans.
 */
typedef struct {
    uint8_t slave_id;                  /**< Slave device ID */
    uint8_t function_code;             /**< Modbus function code */
    uint16_t start_address;            /**< Starting address */
    uint16_t quantity;                 /**< Number of coils/registers */
    uint8_t *frame_data;               /**< Pre-built frame (optional) */
    uint16_t frame_length;             /**< Frame length in bytes */
    uint16_t expected_response_length; /**< Expected response length */
} mb_request_plan_t;

/**
 * @brief Response structure
 *
 * Parsed response from a Modbus slave device.
 */
typedef struct {
    uint8_t slave_id;        /**< Slave device ID */
    uint8_t function_code;   /**< Modbus function code */
    uint8_t byte_count;      /**< Number of data bytes */
    uint8_t *data;           /**< Response data */
    uint16_t data_length;    /**< Data length in bytes */
    bool is_exception;       /**< True if exception response */
    uint8_t exception_code;  /**< Exception code (if is_exception) */
} mb_response_t;

/**
 * @brief Cost calculation parameters
 *
 * Parameters used for calculating communication costs in the character-based
 * cost model.
 */
typedef struct {
    uint8_t req_fixed_chars;  /**< Request fixed overhead (chars) */
    uint8_t resp_fixed_chars; /**< Response fixed overhead (chars) */
    uint8_t gap_chars;        /**< Inter-frame gap (RTU/ASCII: 4, TCP: 0) */
    uint8_t latency_chars;    /**< Network/processing latency (chars) */
} mb_cost_params_t;

/**
 * @brief Function code policy
 *
 * Defines characteristics and constraints for each Modbus function code.
 */
typedef struct {
    uint8_t fc;                 /**< Function code */
    bool supports_merge;        /**< Can merge non-contiguous blocks */
    bool is_read;               /**< Read vs write operation */
    uint8_t req_fixed_chars;    /**< Request overhead (chars) */
    uint8_t resp_fixed_chars;   /**< Response overhead (chars) */
    uint16_t extra_unit_chars;  /**< Cost per extra unit (×100 for precision) */
    uint16_t max_quantity;      /**< Max quantity per request */
} mb_fc_policy_t;

/**
 * @brief Statistics structure
 *
 * Tracks optimization statistics for performance analysis.
 */
typedef struct {
    uint32_t total_requests;     /**< Total requests made */
    uint32_t optimized_requests; /**< Requests that were optimized */
    uint32_t rounds_saved;       /**< Round-trips saved by optimization */
    uint32_t blocks_merged;      /**< Number of blocks merged */
    uint32_t total_chars_sent;   /**< Total characters sent */
    uint32_t total_chars_recv;   /**< Total characters received */
} mb_stats_t;

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_MB_TYPES_H
