/**
 * @file main.c
 * @brief Basic RTU example demonstrating Smart Modbus optimization
 *
 * This example shows how to use the Smart Modbus library with RTU mode
 * to read non-contiguous registers with automatic optimization.
 */

#include <smartmodbus/smartmodbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock transport layer for demonstration
static int mock_send(void *ctx, const uint8_t *data, size_t len) {
    (void)ctx;
    printf("  [SEND] %zu bytes\n", len);
    return (int)len;
}

static int mock_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    (void)ctx;
    (void)buffer;
    (void)max_len;
    *received = 0;
    printf("  [RECV] Mock response\n");
    return 0;
}

static void mock_delay(void *ctx, uint16_t chars) {
    (void)ctx;
    printf("  [DELAY] %u character times\n", chars);
}

int main(void) {
    printf("=== Smart Modbus RTU Example ===\n\n");

    // Initialize master with RTU configuration
    mb_master_t master;
    mb_config_t config = mb_config_default(MB_MODE_RTU);

    // Setup mock transport
    config.transport.send        = mock_send;
    config.transport.recv        = mock_recv;
    config.transport.delay_chars = mock_delay;
    config.transport.context     = NULL;

    int result = mb_master_init(&master, &config);
    if (result != MB_SUCCESS) {
        printf("ERROR: Failed to initialize master: %s\n", mb_error_to_string(result));
        return 1;
    }

    printf("Master initialized successfully\n");
    printf("  Mode: RTU\n");
    printf("  Max PDU: %u chars\n", config.max_pdu_chars);
    printf("  Gap: %u chars\n", config.gap_chars);
    printf("  Latency: %u chars\n\n", config.latency_chars);

    // Example: Read non-contiguous holding registers
    // Addresses: 100-102 (3 registers) and 115-117 (3 registers)
    // Gap: 12 registers between them
    printf("Example: Reading non-contiguous registers\n");
    printf("  Registers: 100-102 and 115-117\n");
    printf("  Gap: 12 registers\n\n");

    uint16_t addresses[] = {100, 101, 102, 115, 116, 117};
    mb_read_request_t request = {
        .slave_id      = 1,
        .function_code = MB_FC_READ_HOLDING_REGISTERS,
        .addresses     = addresses,
        .address_count = 6
    };

    uint16_t data[6];
    printf("Executing optimized read...\n");
    result = mb_master_read_optimized(&master, &request, data, 6);

    if (result == MB_ERROR_NOT_SUPPORTED) {
        printf("  Note: Full optimization not yet implemented (stub)\n");
        printf("  This example demonstrates the API structure\n\n");
    } else if (result != MB_SUCCESS) {
        printf("ERROR: Read failed: %s\n", mb_error_to_string(result));
    }

    // Show what the optimization would do
    printf("\nOptimization Analysis:\n");
    printf("  Without optimization: 2 separate requests\n");
    printf("    Request 1: Read 100-102 (3 registers, 6 bytes)\n");
    printf("    Request 2: Read 115-117 (3 registers, 6 bytes)\n");
    printf("    Total: 2 round-trips\n\n");

    printf("  With gap-aware merge:\n");
    printf("    Gap cost: 12 registers × 2 bytes = 24 bytes\n");
    printf("    Overhead cost: ~15 bytes (req + resp + gap + latency)\n");
    printf("    Decision: 24 > 15, DON'T merge (keep separate)\n");
    printf("    Result: 2 round-trips (no benefit from merging)\n\n");

    printf("  Alternative scenario (smaller gap):\n");
    printf("    If gap was 5 registers: 5 × 2 = 10 bytes\n");
    printf("    10 < 15, MERGE! (saves 5 bytes per round)\n");
    printf("    Result: 1 round-trip instead of 2\n\n");

    // Get statistics
    mb_stats_t stats;
    mb_master_get_stats(&master, &stats);
    printf("Statistics:\n");
    printf("  Total requests: %u\n", stats.total_requests);
    printf("  Optimized requests: %u\n", stats.optimized_requests);
    printf("  Rounds saved: %u\n\n", stats.rounds_saved);

    // Cleanup
    mb_master_cleanup(&master);
    printf("Master cleaned up\n");
    printf("\n=== Example Complete ===\n");

    return 0;
}
