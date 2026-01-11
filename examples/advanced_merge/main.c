/**
 * @file main.c
 * @brief Advanced merge demonstration
 *
 * This example demonstrates the gap-aware merge algorithm with various
 * scenarios to show when merging is beneficial vs. wasteful.
 */

#include <smartmodbus/smartmodbus.h>
#include <stdio.h>
#include <stdlib.h>

void demonstrate_merge_decision(const char *scenario,
                                 uint16_t gap_registers,
                                 uint8_t overhead_chars) {
    printf("\n--- %s ---\n", scenario);
    printf("  Gap: %u registers\n", gap_registers);
    printf("  Gap cost: %u registers × 2 bytes = %u bytes\n", gap_registers, gap_registers * 2);
    printf("  Overhead cost: %u bytes\n", overhead_chars);

    uint16_t gap_cost = gap_registers * 2;
    if (gap_cost < overhead_chars) {
        printf("  Decision: MERGE (gap cost %u < overhead %u)\n", gap_cost, overhead_chars);
        printf("  Savings: %u bytes per round-trip\n", overhead_chars - gap_cost);
    } else {
        printf("  Decision: DON'T MERGE (gap cost %u >= overhead %u)\n", gap_cost, overhead_chars);
        printf("  Waste: %u bytes if merged\n", gap_cost - overhead_chars);
    }
}

int main(void) {
    printf("=== Smart Modbus Gap-Aware Merge Demonstration ===\n");
    printf("\nThis example shows the cost-benefit analysis of merging\n");
    printf("non-contiguous blocks based on the character-based cost model.\n");

    // Typical RTU overhead calculation
    printf("\n=== Cost Model Parameters (RTU) ===\n");
    printf("  Request fixed: 6 bytes (addr + FC + start + quantity + CRC)\n");
    printf("  Response fixed: 5 bytes (addr + FC + count + CRC)\n");
    printf("  Gap: 4 bytes (3.5 char inter-frame gap)\n");
    printf("  Latency: 2 bytes (network/processing delay)\n");
    printf("  Total overhead: 6 + 5 + 4 + 2 = 17 bytes\n");

    uint8_t overhead = 17;

    // Scenario 1: Small gap - merge is beneficial
    demonstrate_merge_decision("Scenario 1: Small Gap (3 registers)", 3, overhead);

    // Scenario 2: Medium gap - borderline
    demonstrate_merge_decision("Scenario 2: Medium Gap (8 registers)", 8, overhead);

    // Scenario 3: Large gap - don't merge
    demonstrate_merge_decision("Scenario 3: Large Gap (15 registers)", 15, overhead);

    // Scenario 4: Very small gap - definitely merge
    demonstrate_merge_decision("Scenario 4: Tiny Gap (1 register)", 1, overhead);

    // Scenario 5: Huge gap - definitely don't merge
    demonstrate_merge_decision("Scenario 5: Huge Gap (50 registers)", 50, overhead);

    // Real-world example
    printf("\n=== Real-World Example ===\n");
    printf("Reading sensor data from PLC:\n");
    printf("  Temperature: registers 100-101 (2 registers)\n");
    printf("  Pressure: registers 105-106 (2 registers)\n");
    printf("  Flow rate: registers 110-111 (2 registers)\n");
    printf("  Status: register 115 (1 register)\n\n");

    printf("Analysis:\n");
    printf("  Block 1: 100-101 (2 regs)\n");
    printf("  Block 2: 105-106 (2 regs) - Gap: 3 regs\n");
    printf("  Block 3: 110-111 (2 regs) - Gap: 4 regs\n");
    printf("  Block 4: 115 (1 reg) - Gap: 4 regs\n\n");

    printf("Optimization steps:\n");
    printf("  1. Merge 100-101 with 105-106? Gap=3, Cost=6 < 17 → YES\n");
    printf("     Result: 100-106 (7 regs, includes 3 unused)\n");
    printf("  2. Merge 100-106 with 110-111? Gap=4, Cost=8 < 17 → YES\n");
    printf("     Result: 100-111 (12 regs, includes 7 unused)\n");
    printf("  3. Merge 100-111 with 115? Gap=4, Cost=8 < 17 → YES\n");
    printf("     Result: 100-115 (16 regs, includes 11 unused)\n\n");

    printf("Final result:\n");
    printf("  Without optimization: 4 requests, 4 round-trips\n");
    printf("  With optimization: 1 request, 1 round-trip\n");
    printf("  Savings: 3 round-trips = 3 × 17 = 51 bytes overhead saved\n");
    printf("  Cost: 11 unused registers = 22 bytes extra data\n");
    printf("  Net savings: 51 - 22 = 29 bytes (36%% reduction)\n");

    printf("\n=== Key Takeaway ===\n");
    printf("The Smart Modbus library automatically makes these decisions\n");
    printf("based on the cost model, maximizing efficiency without user\n");
    printf("intervention. Just specify what data you need, and the library\n");
    printf("handles the optimization!\n");

    printf("\n=== Demonstration Complete ===\n");
    return 0;
}
