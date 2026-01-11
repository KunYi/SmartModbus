# Smart Modbus Examples

This directory contains example applications demonstrating the Smart Modbus library.

## Building Examples

```bash
mkdir build && cd build
cmake -DMB_BUILD_EXAMPLES=ON ..
make
```

## Available Examples

### 1. basic_rtu
**File:** `basic_rtu/main.c`

Demonstrates basic RTU mode usage with gap-aware merge analysis.

**Run:**
```bash
./basic_rtu
```

**Features:**
- RTU mode initialization
- Mock transport layer
- Gap cost analysis demonstration
- Non-contiguous register reading

### 2. basic_tcp
**File:** `basic_tcp/main.c`

Demonstrates TCP/IP mode usage.

**Run:**
```bash
./basic_tcp
```

**Features:**
- TCP mode initialization
- MBAP header handling
- No inter-frame gap (TCP advantage)

### 3. advanced_merge
**File:** `advanced_merge/main.c`

In-depth demonstration of the gap-aware merge algorithm.

**Run:**
```bash
./advanced_merge
```

**Features:**
- Multiple merge scenarios
- Cost-benefit analysis
- Real-world PLC example
- Optimization savings calculation

### 4. embedded_static
**File:** `embedded_static/main.c`

Demonstrates static memory mode for embedded systems.

**Build with static memory:**
```bash
cmake -DMB_USE_STATIC_MEMORY=ON -DMB_MAX_BLOCKS=32 ..
make
```

**Run:**
```bash
./embedded_static
```

**Features:**
- Static memory allocation
- Memory footprint analysis
- MCU-friendly design
- MISRA-C compliance notes

## Example Output

### basic_rtu
```
=== Smart Modbus RTU Example ===

Master initialized successfully
  Mode: RTU
  Max PDU: 253 chars
  Gap: 4 chars
  Latency: 2 chars

Example: Reading non-contiguous registers
  Registers: 100-102 and 115-117
  Gap: 12 registers

Optimization Analysis:
  Without optimization: 2 separate requests
  With gap-aware merge:
    Gap cost: 24 bytes
    Overhead cost: 15 bytes
    Decision: DON'T merge (keep separate)
```

### advanced_merge
```
=== Smart Modbus Gap-Aware Merge Demonstration ===

--- Scenario 1: Small Gap (3 registers) ---
  Gap: 3 registers
  Gap cost: 6 bytes
  Overhead cost: 17 bytes
  Decision: MERGE (saves 11 bytes per round-trip)

Real-World Example:
  Final result: 1 request instead of 4
  Net savings: 29 bytes (36% reduction)
```

## Integration Guide

To integrate Smart Modbus into your project:

1. **Include the header:**
```c
#include <smartmodbus/smartmodbus.h>
```

2. **Initialize master:**
```c
mb_master_t master;
mb_config_t config = mb_config_default(MB_MODE_RTU);
config.transport = your_transport;
mb_master_init(&master, &config);
```

3. **Read with optimization:**
```c
uint16_t addresses[] = {100, 101, 102, 115, 116};
mb_read_request_t request = {
    .slave_id = 1,
    .function_code = MB_FC_READ_HOLDING_REGISTERS,
    .addresses = addresses,
    .address_count = 5
};

uint16_t data[5];
mb_master_read_optimized(&master, &request, data, 5);
```

4. **Cleanup:**
```c
mb_master_cleanup(&master);
```

## Transport Layer Implementation

Each example uses a mock transport. In real applications, implement:

```c
int uart_send(void *ctx, const uint8_t *data, size_t len) {
    // Send data via UART
    return uart_write(ctx, data, len);
}

int uart_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    // Receive data from UART with timeout
    return uart_read(ctx, buffer, max_len, received);
}

void uart_delay(void *ctx, uint16_t chars) {
    // Delay for character times (RTU gap)
    uint32_t us = chars * char_time_us;
    delay_microseconds(us);
}
```

## Notes

- Examples use mock transport for demonstration
- Full implementation requires completing protocol layer
- See main README.md for library architecture details
- Refer to specification in docs/ for algorithm details
