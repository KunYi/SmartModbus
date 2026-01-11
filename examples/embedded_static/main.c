/**
 * @file main.c
 * @brief Embedded static memory example
 *
 * This example demonstrates using Smart Modbus in static memory mode,
 * suitable for MCU environments without dynamic allocation.
 */

#include <smartmodbus/smartmodbus.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Smart Modbus Embedded Static Memory Example ===\n\n");

#ifdef MB_USE_STATIC_MEMORY
    printf("Static memory mode: ENABLED\n");
    printf("  Max blocks: %d\n", MB_MAX_BLOCKS);
    printf("  Max PDUs: %d\n", MB_MAX_PDUS);
    printf("  Max plans: %d\n\n", MB_MAX_PLANS);

    printf("Memory footprint:\n");
    printf("  mb_master_t size: %zu bytes\n", sizeof(mb_master_t));
    printf("  Block pool: %zu bytes\n", sizeof(mb_block_t) * MB_MAX_BLOCKS);
    printf("  PDU pool: %zu bytes\n", sizeof(mb_pdu_t) * MB_MAX_PDUS);
    printf("  Plan pool: %zu bytes\n\n", sizeof(mb_request_plan_t) * MB_MAX_PLANS);

    printf("Advantages of static memory mode:\n");
    printf("  ✓ Deterministic memory usage\n");
    printf("  ✓ No heap fragmentation\n");
    printf("  ✓ No malloc/free overhead\n");
    printf("  ✓ Suitable for safety-critical systems\n");
    printf("  ✓ MISRA-C compliant (no dynamic allocation)\n\n");

    printf("Usage in embedded systems:\n");
    printf("  1. Configure MB_MAX_BLOCKS based on your needs\n");
    printf("  2. Compile with -DMB_USE_STATIC_MEMORY\n");
    printf("  3. All memory is pre-allocated at compile time\n");
    printf("  4. No runtime allocation failures\n\n");

#else
    printf("Static memory mode: DISABLED\n");
    printf("  This example requires MB_USE_STATIC_MEMORY to be enabled\n");
    printf("  Rebuild with: cmake -DMB_USE_STATIC_MEMORY=ON ..\n\n");
#endif

    printf("=== Example Complete ===\n");
    return 0;
}
