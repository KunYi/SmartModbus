/**
 * @file mb_error.h
 * @brief Error codes for Smart Modbus library
 *
 * This file defines all error codes used throughout the Smart Modbus library.
 * All functions return MB_SUCCESS (0) on success, or negative error codes on failure.
 */

#ifndef SMARTMODBUS_MB_ERROR_H
#define SMARTMODBUS_MB_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes for Smart Modbus operations
 */
typedef enum {
    MB_SUCCESS = 0,                    /**< Operation completed successfully */
    MB_ERROR_INVALID_PARAM = -1,       /**< Invalid parameter provided */
    MB_ERROR_BUFFER_TOO_SMALL = -2,    /**< Buffer size insufficient */
    MB_ERROR_TIMEOUT = -3,             /**< Operation timed out */
    MB_ERROR_CRC_MISMATCH = -4,        /**< CRC check failed (RTU) */
    MB_ERROR_LRC_MISMATCH = -5,        /**< LRC check failed (ASCII) */
    MB_ERROR_INVALID_FRAME = -6,       /**< Frame format invalid */
    MB_ERROR_EXCEPTION_RESPONSE = -7,  /**< Modbus exception received */
    MB_ERROR_TRANSPORT = -8,           /**< Transport layer error */
    MB_ERROR_OUT_OF_MEMORY = -9,       /**< Memory allocation failed */
    MB_ERROR_NOT_SUPPORTED = -10,      /**< Operation not supported */
    MB_ERROR_INVALID_FC = -11,         /**< Invalid function code */
    MB_ERROR_INVALID_ADDRESS = -12,    /**< Invalid Modbus address */
    MB_ERROR_INVALID_QUANTITY = -13,   /**< Invalid quantity value */
    MB_ERROR_NO_BLOCKS = -14,          /**< No blocks to process */
    MB_ERROR_TOO_MANY_BLOCKS = -15,    /**< Too many blocks (static mode) */
    MB_ERROR_PDU_TOO_LARGE = -16,      /**< PDU exceeds maximum size */
    MB_ERROR_TOO_MANY_PLANS = -17,     /**< Too many plans (exceeds max_plans) */
    MB_ERROR_NO_MEMORY = -18           /**< Memory allocation failed */
} mb_error_t;

/**
 * @brief Get error message string
 * @param error Error code
 * @return Human-readable error message
 */
const char *mb_error_to_string(mb_error_t error);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_MB_ERROR_H
