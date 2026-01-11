/**
 * @file main.c
 * @brief Basic TCP example demonstrating Smart Modbus with TCP/IP
 *
 * This example shows how to use the Smart Modbus library with TCP mode.
 */

#include <smartmodbus/smartmodbus.h>
#include <stdio.h>
#include <stdlib.h>

// Mock TCP transport
static int mock_tcp_send(void *ctx, const uint8_t *data, size_t len) {
    (void)ctx;
    printf("  [TCP SEND] %zu bytes\n", len);
    return (int)len;
}

static int mock_tcp_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    (void)ctx;
    (void)buffer;
    (void)max_len;
    *received = 0;
    printf("  [TCP RECV] Mock response\n");
    return 0;
}

int main(void) {
    printf("=== Smart Modbus TCP Example ===\n\n");

    // Initialize master with TCP configuration
    mb_master_t master;
    mb_config_t config = mb_config_default(MB_MODE_TCP);

    // Setup mock TCP transport
    config.transport.send        = mock_tcp_send;
    config.transport.recv        = mock_tcp_recv;
    config.transport.delay_chars = NULL; // No delay needed for TCP
    config.transport.context     = NULL;

    int result = mb_master_init(&master, &config);
    if (result != MB_SUCCESS) {
        printf("ERROR: Failed to initialize master: %s\n", mb_error_to_string(result));
        return 1;
    }

    printf("Master initialized successfully\n");
    printf("  Mode: TCP/IP\n");
    printf("  Max PDU: %u chars\n", config.max_pdu_chars);
    printf("  Gap: %u chars (TCP has no inter-frame gap)\n", config.gap_chars);
    printf("  Latency: %u chars\n\n", config.latency_chars);

    // Example: Read holding registers
    printf("Example: Reading holding registers via TCP\n");
    printf("  Slave/Unit ID: 1\n");
    printf("  Registers: 1000-1009 (10 registers)\n\n");

    uint16_t data[10];
    printf("Executing read...\n");
    result = mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS, 1000, 10, data);

    if (result == MB_ERROR_NOT_SUPPORTED) {
        printf("  Note: Full implementation not yet complete (stub)\n");
        printf("  This example demonstrates the API structure\n\n");
    } else if (result != MB_SUCCESS) {
        printf("ERROR: Read failed: %s\n", mb_error_to_string(result));
    }

    printf("\nTCP/IP Advantages:\n");
    printf("  - No inter-frame gap (gap_chars = 0)\n");
    printf("  - Lower latency than serial\n");
    printf("  - Full-duplex communication\n");
    printf("  - MBAP header provides transaction tracking\n\n");

    // Cleanup
    mb_master_cleanup(&master);
    printf("Master cleaned up\n");
    printf("\n=== Example Complete ===\n");

    return 0;
}
