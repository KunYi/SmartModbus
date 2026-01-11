/**
 * @file smartmodbus.h
 * @brief Smart Modbus Master API - Main public interface
 *
 * This is the primary header file for the Smart Modbus library.
 * Include this file to access all public API functions.
 *
 * @example Basic usage:
 * @code
 * #include <smartmodbus/smartmodbus.h>
 *
 * // Initialize master
 * mb_master_t master;
 * mb_config_t config = mb_config_default(MB_MODE_RTU);
 * config.transport = my_transport;
 * mb_master_init(&master, &config);
 *
 * // Read with optimization
 * uint16_t addresses[] = {100, 101, 102, 115, 116, 117};
 * mb_read_request_t request = {
 *     .slave_id = 1,
 *     .function_code = MB_FC_READ_HOLDING_REGISTERS,
 *     .addresses = addresses,
 *     .address_count = 6
 * };
 *
 * uint16_t data[6];
 * mb_master_read_optimized(&master, &request, data, 6);
 *
 * // Cleanup
 * mb_master_cleanup(&master);
 * @endcode
 */

#ifndef SMARTMODBUS_H
#define SMARTMODBUS_H

#include "smartmodbus/mb_config.h"
#include "smartmodbus/mb_error.h"
#include "smartmodbus/mb_transport.h"
#include "smartmodbus/mb_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Modbus master context
 * @param master Pointer to master context
 * @param config Configuration parameters
 * @return MB_SUCCESS on success, error code otherwise
 */
int mb_master_init(mb_master_t *master, const mb_config_t *config);

/**
 * @brief Read data with automatic optimization
 * @param master Master context
 * @param request Read request with potentially non-contiguous addresses
 * @param data_buffer Buffer to store read data
 * @param buffer_size Size of data buffer in words (registers or coils)
 * @return MB_SUCCESS on success, error code otherwise
 *
 * This function automatically:
 * - Converts addresses to blocks
 * - Merges non-contiguous blocks based on gap cost analysis
 * - Packs blocks into optimal PDU frames using FFD
 * - Executes minimal round-trips
 * - Extracts only requested data from responses
 */
int mb_master_read_optimized(mb_master_t *master,
                              const mb_read_request_t *request,
                              uint16_t *d_buffer,
                              uint16_t buffer_size);

/**
 * @brief Execute single read request (no optimization)
 * @param master Master context
 * @param slave_id Slave device ID (1-247)
 * @param fc Function code (01, 02, 03, or 04)
 * @param start_addr Starting address
 * @param quantity Number of coils/registers to read
 * @param data_buffer Buffer to store read data
 * @return MB_SUCCESS on success, error code otherwise
 */
int mb_master_read_single(mb_master_t *master,
                          uint8_t slave_id,
                          uint8_t fc,
                          uint16_t start_addr,
                          uint16_t quantity,
                          uint16_t *data_buffer);

/**
 * @brief Write single coil (FC05)
 * @param master Master context
 * @param slave_id Slave device ID
 * @param addr Coil address
 * @param value Coil value (true = ON, false = OFF)
 * @return MB_SUCCESS on success, error code otherwise
 */
int mb_master_write_single_coil(mb_master_t *master, uint8_t slave_id, uint16_t addr, bool value);

/**
 * @brief Write single register (FC06)
 * @param master Master context
 * @param slave_id Slave device ID
 * @param addr Register address
 * @param value Register value
 * @return MB_SUCCESS on success, error code otherwise
 */
int mb_master_write_single_register(mb_master_t *master,
                                     uint8_t slave_id,
                                     uint16_t addr,
                                     uint16_t value);

/**
 * @brief Write multiple registers (FC16)
 * @param master Master context
 * @param slave_id Slave device ID
 * @param start_addr Starting address
 * @param quantity Number of registers
 * @param values Array of register values
 * @return MB_SUCCESS on success, error code otherwise
 */
int mb_master_write_multiple_registers(mb_master_t *master,
                                        uint8_t slave_id,
                                        uint16_t start_addr,
                                        uint16_t quantity,
                                        const uint16_t *values);

/**
 * @brief Get optimization statistics
 * @param master Master context
 * @param stats Output statistics structure
 */
void mb_master_get_stats(const mb_master_t *master, mb_stats_t *stats);

/**
 * @brief Reset statistics counters
 * @param master Master context
 */
void mb_master_reset_stats(mb_master_t *master);

/**
 * @brief Cleanup master context and free resources
 * @param master Master context
 */
void mb_master_cleanup(mb_master_t *master);

/**
 * @brief Get library version string
 * @return Version string (e.g., "1.0.0")
 */
const char *mb_get_version(void);

#ifdef __cplusplus
}
#endif

#endif  // SMARTMODBUS_H
