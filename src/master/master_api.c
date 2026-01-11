/**
 * @file master_api.c
 * @brief Master API implementation
 *
 * This file provides the main user-facing API for the Smart Modbus library.
 */

#include "smartmodbus/smartmodbus.h"
#include "smartmodbus/mb_error.h"
#include "request_optimizer.h"
#include "response_parser.h"
#include "../protocol/frame_builder.h"

#include <string.h>
#include <stdlib.h>

const char *mb_error_to_string(mb_error_t error) {
    switch (error) {
    case MB_SUCCESS:
        return "Success";
    case MB_ERROR_INVALID_PARAM:
        return "Invalid parameter";
    case MB_ERROR_BUFFER_TOO_SMALL:
        return "Buffer too small";
    case MB_ERROR_TIMEOUT:
        return "Timeout";
    case MB_ERROR_CRC_MISMATCH:
        return "CRC mismatch";
    case MB_ERROR_LRC_MISMATCH:
        return "LRC mismatch";
    case MB_ERROR_INVALID_FRAME:
        return "Invalid frame";
    case MB_ERROR_EXCEPTION_RESPONSE:
        return "Exception response";
    case MB_ERROR_TRANSPORT:
        return "Transport error";
    case MB_ERROR_OUT_OF_MEMORY:
        return "Out of memory";
    case MB_ERROR_NOT_SUPPORTED:
        return "Not supported";
    case MB_ERROR_INVALID_FC:
        return "Invalid function code";
    case MB_ERROR_INVALID_ADDRESS:
        return "Invalid address";
    case MB_ERROR_INVALID_QUANTITY:
        return "Invalid quantity";
    case MB_ERROR_NO_BLOCKS:
        return "No blocks";
    case MB_ERROR_TOO_MANY_BLOCKS:
        return "Too many blocks";
    case MB_ERROR_PDU_TOO_LARGE:
        return "PDU too large";
    case MB_ERROR_TOO_MANY_PLANS:
        return "Too many plans";
    case MB_ERROR_NO_MEMORY:
        return "No memory";
    default:
        return "Unknown error";
    }
}

mb_config_t mb_config_default(mb_mode_t mode) {
    mb_config_t config;
    memset(&config, 0, sizeof(config));

    config.mode          = mode;
    config.max_pdu_chars = 253; // Standard Modbus PDU limit
    config.timeout_ms    = 1000;

    // Set gap and latency based on mode
    if (mode == MB_MODE_RTU || mode == MB_MODE_ASCII) {
        config.gap_chars     = 4; // 3.5 chars rounded up
        config.latency_chars = 2; // Default latency
    } else {
        config.gap_chars     = 0; // TCP has no gap
        config.latency_chars = 1; // Lower latency for TCP
    }

    return config;
}

int mb_master_init(mb_master_t *master, const mb_config_t *config) {
    if (master == NULL || config == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Copy configuration
    memcpy(&master->config, config, sizeof(mb_config_t));

    // Initialize transaction ID for TCP
    master->transaction_id = 0;

    // Reset statistics
    memset(&master->stats, 0, sizeof(mb_stats_t));

#ifdef MB_USE_STATIC_MEMORY
    // Initialize static memory pools
    master->block_pool_used = 0;
    master->pdu_pool_used   = 0;
    master->plan_pool_used  = 0;
#endif

    return MB_SUCCESS;
}

int mb_master_read_optimized(mb_master_t *master,
                              const mb_read_request_t *request,
                              uint16_t *data_buffer,
                              uint16_t buffer_size) {
    if (master == NULL || request == NULL || data_buffer == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    if (buffer_size < request->address_count) {
        return MB_ERROR_BUFFER_TOO_SMALL;
    }

    // Step 1: Optimize request into plans
    mb_request_plan_t plans[16];  // Max 16 plans
    uint16_t plan_count = 0;

    int result = mb_optimize_request(request, &master->config, plans, 16, &plan_count);

    if (result != MB_SUCCESS) {
        return result;
    }

    // Step 2: Execute each plan
    uint16_t temp_buffer[256];  // Temporary buffer for merged reads
    uint16_t data_index = 0;

    for (uint16_t i = 0; i < plan_count; i++) {
        mb_request_plan_t *plan = &plans[i];

        // Build PDU
        uint8_t pdu_data[4];
        pdu_data[0] = (uint8_t)((plan->start_address >> 8) & 0xFF);
        pdu_data[1] = (uint8_t)(plan->start_address & 0xFF);
        pdu_data[2] = (uint8_t)((plan->quantity >> 8) & 0xFF);
        pdu_data[3] = (uint8_t)(plan->quantity & 0xFF);

        // Build frame
        uint8_t frame_buffer[260];
        uint16_t frame_length = 0;
        result = mb_build_frame(plan->slave_id, plan->function_code, pdu_data, 4, master->config.mode,
                                frame_buffer, sizeof(frame_buffer), &frame_length);

        if (result != MB_SUCCESS) {
            master->stats.total_requests++;
            return result;
        }

        // Send request
        if (master->config.transport.send == NULL) {
            return MB_ERROR_TRANSPORT;
        }

        int sent = master->config.transport.send(master->config.transport.context, frame_buffer, frame_length);
        if (sent < 0) {
            master->stats.total_requests++;
            return MB_ERROR_TRANSPORT;
        }

        // Receive response
        if (master->config.transport.recv == NULL) {
            return MB_ERROR_TRANSPORT;
        }

        uint8_t response_buffer[260];
        size_t received = 0;
        int recv_result = master->config.transport.recv(master->config.transport.context, response_buffer,
                                                         sizeof(response_buffer), &received);

        if (recv_result < 0 || received == 0) {
            master->stats.total_requests++;
            return MB_ERROR_TIMEOUT;
        }

        // Parse frame
        uint8_t resp_slave_id, resp_fc;
        uint8_t pdu_response[256];
        uint16_t pdu_length = 0;

        result = mb_parse_frame(response_buffer, (uint16_t)received, master->config.mode, &resp_slave_id,
                                &resp_fc, pdu_response, &pdu_length);

        if (result != MB_SUCCESS) {
            master->stats.total_requests++;
            return result;
        }

        // Validate response
        if (resp_slave_id != plan->slave_id) {
            master->stats.total_requests++;
            return MB_ERROR_INVALID_FRAME;
        }

        // Parse response data into temp buffer
        result = mb_parse_read_response(resp_fc, pdu_response, pdu_length, plan->quantity, temp_buffer);

        if (result != MB_SUCCESS) {
            master->stats.total_requests++;
            return result;
        }

        // Extract requested data from merged response
        // For now, copy all data (TODO: map back to original addresses)
        for (uint16_t j = 0; j < plan->quantity && data_index < buffer_size; j++) {
            data_buffer[data_index++] = temp_buffer[j];
        }

        // Update statistics
        master->stats.total_requests++;
        master->stats.total_requests++;
        master->stats.total_chars_sent += frame_length;
        master->stats.total_chars_recv += received;
    }

    // Update optimization statistics
    master->stats.optimized_requests++;
    master->stats.blocks_merged += (request->address_count - plan_count);

    return MB_SUCCESS;
}

int mb_master_read_single(mb_master_t *master,
                          uint8_t slave_id,
                          uint8_t fc,
                          uint16_t start_addr,
                          uint16_t quantity,
                          uint16_t *data_buffer) {
    if (master == NULL || data_buffer == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    // Validate function code
    if (fc != MB_FC_READ_COILS && fc != MB_FC_READ_DISCRETE_INPUTS &&
        fc != MB_FC_READ_HOLDING_REGISTERS && fc != MB_FC_READ_INPUT_REGISTERS) {
        return MB_ERROR_INVALID_FC;
    }

    // Build PDU (address + quantity)
    uint8_t pdu_data[4];
    pdu_data[0] = (uint8_t)((start_addr >> 8) & 0xFF);
    pdu_data[1] = (uint8_t)(start_addr & 0xFF);
    pdu_data[2] = (uint8_t)((quantity >> 8) & 0xFF);
    pdu_data[3] = (uint8_t)(quantity & 0xFF);

    // Build frame
    uint8_t frame_buffer[260];  // Max frame size
    uint16_t frame_length = 0;
    int result = mb_build_frame(slave_id, fc, pdu_data, 4, master->config.mode, frame_buffer,
                                sizeof(frame_buffer), &frame_length);

    if (result != MB_SUCCESS) {
        return result;
    }

    // Send request
    if (master->config.transport.send == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    int sent = master->config.transport.send(master->config.transport.context, frame_buffer, frame_length);
    if (sent < 0) {
        master->stats.total_requests++;
        return MB_ERROR_TRANSPORT;
    }

    // Receive response
    if (master->config.transport.recv == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    uint8_t response_buffer[260];
    size_t received = 0;
    int recv_result = master->config.transport.recv(master->config.transport.context, response_buffer,
                                                     sizeof(response_buffer), &received);

    if (recv_result < 0 || received == 0) {
        master->stats.total_requests++;
        return MB_ERROR_TIMEOUT;
    }

    // Parse frame
    uint8_t resp_slave_id, resp_fc;
    uint8_t pdu_response[256];
    uint16_t pdu_length = 0;

    result = mb_parse_frame(response_buffer, (uint16_t)received, master->config.mode, &resp_slave_id,
                            &resp_fc, pdu_response, &pdu_length);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Validate response
    if (resp_slave_id != slave_id) {
        master->stats.total_requests++;
        return MB_ERROR_INVALID_FRAME;
    }

    // Parse response data
    result = mb_parse_read_response(resp_fc, pdu_response, pdu_length, quantity, data_buffer);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Update statistics
    master->stats.total_requests++;
    master->stats.total_requests++;
    master->stats.total_chars_sent += frame_length;
    master->stats.total_chars_recv += received;

    return MB_SUCCESS;
}

int mb_master_write_single_coil(mb_master_t *master, uint8_t slave_id, uint16_t addr, bool value) {
    if (master == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    uint8_t fc = MB_FC_WRITE_SINGLE_COIL;

    // Build PDU (address + value)
    uint8_t pdu_data[4];
    pdu_data[0] = (uint8_t)((addr >> 8) & 0xFF);
    pdu_data[1] = (uint8_t)(addr & 0xFF);
    pdu_data[2] = value ? 0xFF : 0x00;  // 0xFF00 for ON, 0x0000 for OFF
    pdu_data[3] = 0x00;

    // Build frame
    uint8_t frame_buffer[260];
    uint16_t frame_length = 0;
    int result = mb_build_frame(slave_id, fc, pdu_data, 4, master->config.mode, frame_buffer,
                                sizeof(frame_buffer), &frame_length);

    if (result != MB_SUCCESS) {
        return result;
    }

    // Send request
    if (master->config.transport.send == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    int sent = master->config.transport.send(master->config.transport.context, frame_buffer, frame_length);
    if (sent < 0) {
        master->stats.total_requests++;
        return MB_ERROR_TRANSPORT;
    }

    // Receive response
    if (master->config.transport.recv == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    uint8_t response_buffer[260];
    size_t received = 0;
    int recv_result = master->config.transport.recv(master->config.transport.context, response_buffer,
                                                     sizeof(response_buffer), &received);

    if (recv_result < 0 || received == 0) {
        master->stats.total_requests++;
        return MB_ERROR_TIMEOUT;
    }

    // Parse frame
    uint8_t resp_slave_id, resp_fc;
    uint8_t pdu_response[256];
    uint16_t pdu_length = 0;

    result = mb_parse_frame(response_buffer, (uint16_t)received, master->config.mode, &resp_slave_id,
                            &resp_fc, pdu_response, &pdu_length);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Validate response
    if (resp_slave_id != slave_id) {
        master->stats.total_requests++;
        return MB_ERROR_INVALID_FRAME;
    }

    // Parse write response
    result = mb_parse_write_response(resp_fc, pdu_response, pdu_length, addr, 1, &value);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Update statistics
    master->stats.total_requests++;
    master->stats.total_requests++;
    master->stats.total_chars_sent += frame_length;
    master->stats.total_chars_recv += received;

    return MB_SUCCESS;
}

int mb_master_write_single_register(mb_master_t *master,
                                     uint8_t slave_id,
                                     uint16_t addr,
                                     uint16_t value) {
    if (master == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    uint8_t fc = MB_FC_WRITE_SINGLE_REGISTER;

    // Build PDU (address + value)
    uint8_t pdu_data[4];
    pdu_data[0] = (uint8_t)((addr >> 8) & 0xFF);
    pdu_data[1] = (uint8_t)(addr & 0xFF);
    pdu_data[2] = (uint8_t)((value >> 8) & 0xFF);
    pdu_data[3] = (uint8_t)(value & 0xFF);

    // Build frame
    uint8_t frame_buffer[260];
    uint16_t frame_length = 0;
    int result = mb_build_frame(slave_id, fc, pdu_data, 4, master->config.mode, frame_buffer,
                                sizeof(frame_buffer), &frame_length);

    if (result != MB_SUCCESS) {
        return result;
    }

    // Send request
    if (master->config.transport.send == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    int sent = master->config.transport.send(master->config.transport.context, frame_buffer, frame_length);
    if (sent < 0) {
        master->stats.total_requests++;
        return MB_ERROR_TRANSPORT;
    }

    // Receive response
    if (master->config.transport.recv == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    uint8_t response_buffer[260];
    size_t received = 0;
    int recv_result = master->config.transport.recv(master->config.transport.context, response_buffer,
                                                     sizeof(response_buffer), &received);

    if (recv_result < 0 || received == 0) {
        master->stats.total_requests++;
        return MB_ERROR_TIMEOUT;
    }

    // Parse frame
    uint8_t resp_slave_id, resp_fc;
    uint8_t pdu_response[256];
    uint16_t pdu_length = 0;

    result = mb_parse_frame(response_buffer, (uint16_t)received, master->config.mode, &resp_slave_id,
                            &resp_fc, pdu_response, &pdu_length);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Validate response
    if (resp_slave_id != slave_id) {
        master->stats.total_requests++;
        return MB_ERROR_INVALID_FRAME;
    }

    // Parse write response
    result = mb_parse_write_response(resp_fc, pdu_response, pdu_length, addr, 1, &value);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Update statistics
    master->stats.total_requests++;
    master->stats.total_requests++;
    master->stats.total_chars_sent += frame_length;
    master->stats.total_chars_recv += received;

    return MB_SUCCESS;
}

int mb_master_write_multiple_registers(mb_master_t *master,
                                        uint8_t slave_id,
                                        uint16_t start_addr,
                                        uint16_t quantity,
                                        const uint16_t *values) {
    if (master == NULL || values == NULL) {
        return MB_ERROR_INVALID_PARAM;
    }

    if (quantity == 0 || quantity > 123) {  // Max 123 registers for FC16
        return MB_ERROR_INVALID_QUANTITY;
    }

    uint8_t fc = MB_FC_WRITE_MULTIPLE_REGISTERS;

    // Build PDU (address + quantity + byte_count + values)
    uint8_t pdu_data[256];
    uint16_t pdu_length = 0;

    pdu_data[pdu_length++] = (uint8_t)((start_addr >> 8) & 0xFF);
    pdu_data[pdu_length++] = (uint8_t)(start_addr & 0xFF);
    pdu_data[pdu_length++] = (uint8_t)((quantity >> 8) & 0xFF);
    pdu_data[pdu_length++] = (uint8_t)(quantity & 0xFF);
    pdu_data[pdu_length++] = (uint8_t)(quantity * 2);  // Byte count

    // Add register values (big-endian)
    for (uint16_t i = 0; i < quantity; i++) {
        pdu_data[pdu_length++] = (uint8_t)((values[i] >> 8) & 0xFF);
        pdu_data[pdu_length++] = (uint8_t)(values[i] & 0xFF);
    }

    // Build frame
    uint8_t frame_buffer[260];
    uint16_t frame_length = 0;
    int result = mb_build_frame(slave_id, fc, pdu_data, pdu_length, master->config.mode, frame_buffer,
                                sizeof(frame_buffer), &frame_length);

    if (result != MB_SUCCESS) {
        return result;
    }

    // Send request
    if (master->config.transport.send == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    int sent = master->config.transport.send(master->config.transport.context, frame_buffer, frame_length);
    if (sent < 0) {
        master->stats.total_requests++;
        return MB_ERROR_TRANSPORT;
    }

    // Receive response
    if (master->config.transport.recv == NULL) {
        return MB_ERROR_TRANSPORT;
    }

    uint8_t response_buffer[260];
    size_t received = 0;
    int recv_result = master->config.transport.recv(master->config.transport.context, response_buffer,
                                                     sizeof(response_buffer), &received);

    if (recv_result < 0 || received == 0) {
        master->stats.total_requests++;
        return MB_ERROR_TIMEOUT;
    }

    // Parse frame
    uint8_t resp_slave_id, resp_fc;
    uint8_t pdu_response[256];
    uint16_t pdu_resp_length = 0;

    result = mb_parse_frame(response_buffer, (uint16_t)received, master->config.mode, &resp_slave_id,
                            &resp_fc, pdu_response, &pdu_resp_length);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Validate response
    if (resp_slave_id != slave_id) {
        master->stats.total_requests++;
        return MB_ERROR_INVALID_FRAME;
    }

    // Parse write response
    result = mb_parse_write_response(resp_fc, pdu_response, pdu_resp_length, start_addr, quantity, NULL);

    if (result != MB_SUCCESS) {
        master->stats.total_requests++;
        return result;
    }

    // Update statistics
    master->stats.total_requests++;
    master->stats.total_requests++;
    master->stats.total_chars_sent += frame_length;
    master->stats.total_chars_recv += received;

    return MB_SUCCESS;
}

void mb_master_get_stats(const mb_master_t *master, mb_stats_t *stats) {
    if (master == NULL || stats == NULL) {
        return;
    }

    memcpy(stats, &master->stats, sizeof(mb_stats_t));
}

void mb_master_reset_stats(mb_master_t *master) {
    if (master == NULL) {
        return;
    }

    memset(&master->stats, 0, sizeof(mb_stats_t));
}

void mb_master_cleanup(mb_master_t *master) {
    if (master == NULL) {
        return;
    }

    // TODO: Free any dynamically allocated resources
    // In static mode, just reset pool counters
#ifdef MB_USE_STATIC_MEMORY
    master->block_pool_used = 0;
    master->pdu_pool_used   = 0;
    master->plan_pool_used  = 0;
#endif
}

const char *mb_get_version(void) {
    return "1.0.0";
}
