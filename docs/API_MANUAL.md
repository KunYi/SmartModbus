# Smart Modbus Library - API Usage Manual

**Version:** 1.0.0
**Date:** 2026-01-11
**Target:** Embedded Systems, Industrial Automation, IoT Gateways

---

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Core Concepts](#core-concepts)
4. [API Reference](#api-reference)
5. [Usage Examples](#usage-examples)
6. [Transport Layer Implementation](#transport-layer-implementation)
7. [Configuration Options](#configuration-options)
8. [Error Handling](#error-handling)
9. [Best Practices](#best-practices)
10. [Troubleshooting](#troubleshooting)

---

## Introduction

Smart Modbus is a high-performance Modbus optimization library that automatically reduces communication overhead by intelligently merging non-contiguous data blocks. The library uses a character-based cost model to make optimal decisions about when to merge requests.

### Key Features

- **Automatic Optimization**: No manual configuration needed
- **Protocol Support**: RTU, ASCII, and TCP/IP
- **Memory Modes**: Static (MCU-friendly) or dynamic allocation
- **Transport Agnostic**: Works with any physical layer
- **All Function Codes**: FC01-FC23 supported
- **Performance**: 30-40% overhead reduction in typical scenarios

---

## Getting Started

### Installation

1. **Include the lib in your project:**

```cmake
# In your CMakeLists.txt
add_subdirectory(path/to/SmartModbus)
target_link_libraries(your_target PRIVATE smartmodbus)
```

2. **Include the header:**

```c
#include <smartmodbus/smartmodbus.h>
```

### Minimal Example

```c
#include <smartmodbus/smartmodbus.h>

int main(void) {
    // 1. Initialize master
    mb_master_t master;
    mb_config_t config = mb_config_default(MB_MODE_RTU);

    // 2. Setup transport (see Transport Layer section)
    config.transport.send = my_uart_send;
    config.transport.recv = my_uart_recv;
    config.transport.delay_chars = my_uart_delay;

    mb_master_init(&master, &config);

    // 3. Read data (library handles optimization automatically)
    uint16_t data[10];
    mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS, 100, 10, data);

    // 4. Cleanup
    mb_master_cleanup(&master);

    return 0;
}
```

---

## Core Concepts

### 1. Character-Based Cost Model

The library uses **characters (bytes)** as the universal cost unit:

```
OVERHEAD_CHAR = REQ_FIXED + RESP_FIXED + GAP_CHAR + LATENCY_CHAR
```

- **REQ_FIXED**: Request frame overhead (address, FC, CRC)
- **RESP_FIXED**: Response frame overhead
- **GAP_CHAR**: Inter-frame gap (RTU/ASCII: 4, TCP: 0)
- **LATENCY_CHAR**: Network/processing delay

### 2. Gap-Aware Merge

The library automatically merges non-contiguous blocks when beneficial:

```c
if (gap_cost < overhead_cost) {
    MERGE();  // Reading extra data is cheaper
} else {
    KEEP_SEPARATE();  // Additional round-trip is cheaper
}
```

**Example:**
- Gap: 5 registers = 10 bytes
- Overhead: 17 bytes
- Decision: **MERGE** (saves 7 bytes)

### 3. Data Flow

```
User Request (addresses)
    ↓
Convert to Blocks
    ↓
Gap-Aware Merge
    ↓
FFD Packing
    ↓
Frame Building
    ↓
Transport Layer
    ↓
Response Parsing
    ↓
Extract User Data
```

---

## API Reference

### Initialization

#### `mb_master_init()`

Initialize a Modbus master instance.

```c
int mb_master_init(mb_master_t *master, const mb_config_t *config);
```

**Parameters:**
- `master`: Pointer to master context structure
- `config`: Configuration parameters

**Returns:**
- `MB_SUCCESS` (0) on success
- Negative error code on failure

**Example:**
```c
mb_master_t master;
mb_config_t config = mb_config_default(MB_MODE_RTU);
config.transport = my_transport;

int result = mb_master_init(&master, &config);
if (result != MB_SUCCESS) {
    printf("Init failed: %s\n", mb_error_to_string(result));
}
```

---

#### `mb_config_default()`

Create default configuration for a protocol mode.

```c
mb_config_t mb_config_default(mb_mode_t mode);
```

**Parameters:**
- `mode`: Protocol mode (`MB_MODE_RTU`, `MB_MODE_ASCII`, `MB_MODE_TCP`)

**Returns:**
- Configuration structure with default values

**Default Values:**
- `max_pdu_chars`: 253 (Modbus standard)
- `gap_chars`: 4 for RTU/ASCII, 0 for TCP
- `latency_chars`: 2 for RTU/ASCII, 1 for TCP
- `timeout_ms`: 1000

---

### Reading Data

#### `mb_master_read_optimized()`

Read data with automatic optimization (recommended).

```c
int mb_master_read_optimized(mb_master_t *master,
                              const mb_read_request_t *request,
                              uint16_t *data_buffer,
                              uint16_t buffer_size);
```

**Parameters:**
- `master`: Master context
- `request`: Read request with potentially non-contiguous addresses
- `data_buffer`: Buffer to store read data
- `buffer_size`: Buffer size in words

**Returns:**
- `MB_SUCCESS` on success
- Negative error code on failure

**Example:**
```c
// Read non-contiguous registers: 100-102 and 115-117
uint16_t addresses[] = {100, 101, 102, 115, 116, 117};
mb_read_request_t request = {
    .slave_id = 1,
    .function_code = MB_FC_READ_HOLDING_REGISTERS,
    .addresses = addresses,
    .address_count = 6
};

uint16_t data[6];
int result = mb_master_read_optimized(&master, &request, data, 6);

// Library automatically decides whether to merge based on gap cost
```

---

#### `mb_master_read_single()`

Read contiguous data without optimization.

```c
int mb_master_read_single(mb_master_t *master,
                          uint8_t slave_id,
                          uint8_t fc,
                          uint16_t start_addr,
                          uint16_t quantity,
                          uint16_t *data_buffer);
```

**Parameters:**
- `master`: Master context
- `slave_id`: Slave device ID (1-247)
- `fc`: Function code (01, 02, 03, or 04)
- `start_addr`: Starting address
- `quantity`: Number of coils/registers
- `data_buffer`: Buffer to store data

**Returns:**
- `MB_SUCCESS` on success
- Negative error code on failure

**Example:**
```c
// Read 10 holding registers starting at address 100
uint16_t data[10];
int result = mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS,
                                   100, 10, data);
```

---

### Writing Data

#### `mb_master_write_single_coil()`

Write a single coil (FC05).

```c
int mb_master_write_single_coil(mb_master_t *master,
                                 uint8_t slave_id,
                                 uint16_t addr,
                                 bool value);
```

**Example:**
```c
// Turn ON coil at address 50
mb_master_write_single_coil(&master, 1, 50, true);
```

---

#### `mb_master_write_single_register()`

Write a single register (FC06).

```c
int mb_master_write_single_register(mb_master_t *master,
                                     uint8_t slave_id,
                                     uint16_t addr,
                                     uint16_t value);
```

**Example:**
```c
// Write value 1234 to register 100
mb_master_write_single_register(&master, 1, 100, 1234);
```

---

#### `mb_master_write_multiple_registers()`

Write multiple registers (FC16).

```c
int mb_master_write_multiple_registers(mb_master_t *master,
                                        uint8_t slave_id,
                                        uint16_t start_addr,
                                        uint16_t quantity,
                                        const uint16_t *values);
```

**Example:**
```c
// Write 5 registers starting at address 100
uint16_t values[] = {100, 200, 300, 400, 500};
mb_master_write_multiple_registers(&master, 1, 100, 5, values);
```

---

### Statistics and Cleanup

#### `mb_master_get_stats()`

Get optimization statistics.

```c
void mb_master_get_stats(const mb_master_t *master, mb_stats_t *stats);
```

**Example:**
```c
mb_stats_t stats;
mb_master_get_stats(&master, &stats);

printf("Total requests: %u\n", stats.total_requests);
printf("Optimized requests: %u\n", stats.optimized_requests);
printf("Rounds saved: %u\n", stats.rounds_saved);
```

---

#### `mb_master_reset_stats()`

Reset statistics counters.

```c
void mb_master_reset_stats(mb_master_t *master);
```

---

#### `mb_master_cleanup()`

Cleanup master context and free resources.

```c
void mb_master_cleanup(mb_master_t *master);
```

**Example:**
```c
mb_master_cleanup(&master);
```

---

## Usage Examples

### Example 1: Basic RTU Read

```c
#include <smartmodbus/smartmodbus.h>

// Transport callbacks (implement these for your hardware)
int uart_send(void *ctx, const uint8_t *data, size_t len) {
    // Send data via UART
    return uart_write(ctx, data, len);
}

int uart_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    // Receive data from UART with timeout
    return uart_read_timeout(ctx, buffer, max_len, received, 1000);
}

void uart_delay(void *ctx, uint16_t chars) {
    // Delay for character times (3.5 char gap for RTU)
    uint32_t us = chars * 1000;  // Assuming 9600 baud
    delay_microseconds(us);
}

int main(void) {
    // Initialize UART hardware
    uart_handle_t uart = uart_init(9600, UART_8N1);

    // Configure Smart Modbus
    mb_master_t master;
    mb_config_t config = mb_config_default(MB_MODE_RTU);

    config.transport.send = uart_send;
    config.transport.recv = uart_recv;
    config.transport.delay_chars = uart_delay;
    config.transport.context = &uart;

    mb_master_init(&master, &config);

    // Read 10 holding registers from slave 1
    uint16_t data[10];
    int result = mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS,
                                       100, 10, data);

    if (result == MB_SUCCESS) {
        for (int i = 0; i < 10; i++) {
            printf("Register %d: %u\n", 100 + i, data[i]);
        }
    } else {
        printf("Read failed: %s\n", mb_error_to_string(result));
    }

    mb_master_cleanup(&master);
    return 0;
}
```

---

### Example 2: Optimized Read with Non-Contiguous Addresses

```c
#include <smartmodbus/smartmodbus.h>

int main(void) {
    mb_master_t master;
    mb_config_t config = mb_config_default(MB_MODE_RTU);
    config.transport = my_transport;
    mb_master_init(&master, &config);

    // Read sensor data from PLC
    // Temperature: 100-101, Pressure: 105-106, Flow: 110-111, Status: 115
    uint16_t addresses[] = {100, 101, 105, 106, 110, 111, 115};

    mb_read_request_t request = {
        .slave_id = 1,
        .function_code = MB_FC_READ_HOLDING_REGISTERS,
        .addresses = addresses,
        .address_count = 7
    };

    uint16_t data[7];
    int result = mb_master_read_optimized(&master, &request, data, 7);

    if (result == MB_SUCCESS) {
        uint16_t temp = (data[0] << 16) | data[1];
        uint16_t pressure = (data[2] << 16) | data[3];
        uint16_t flow = (data[4] << 16) | data[5];
        uint16_t status = data[6];

        printf("Temperature: %u\n", temp);
        printf("Pressure: %u\n", pressure);
        printf("Flow: %u\n", flow);
        printf("Status: 0x%04X\n", status);
    }

    // Check optimization statistics
    mb_stats_t stats;
    mb_master_get_stats(&master, &stats);
    printf("Rounds saved: %u\n", stats.rounds_saved);

    mb_master_cleanup(&master);
    return 0;
}
```

---

### Example 3: TCP/IP Mode

```c
#include <smartmodbus/smartmodbus.h>
#include <sys/socket.h>

int tcp_send(void *ctx, const uint8_t *data, size_t len) {
    int sock = *(int *)ctx;
    return send(sock, data, len, 0);
}

int tcp_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    int sock = *(int *)ctx;
    ssize_t n = recv(sock, buffer, max_len, 0);
    if (n > 0) {
        *received = n;
        return 0;
    }
    return -1;
}

int main(void) {
    // Connect to Modbus TCP server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(502),
        .sin_addr.s_addr = inet_addr("192.168.1.100")
    };
    connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    // Configure Smart Modbus for TCP
    mb_master_t master;
    mb_config_t config = mb_config_default(MB_MODE_TCP);

    config.transport.send = tcp_send;
    config.transport.recv = tcp_recv;
    config.transport.delay_chars = NULL;  // No delay needed for TCP
    config.transport.context = &sock;

    mb_master_init(&master, &config);

    // Read data
    uint16_t data[10];
    mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS, 0, 10, data);

    mb_master_cleanup(&master);
    close(sock);
    return 0;
}
```

---

### Example 4: Static Memory Mode (MCU)

```c
// Compile with: -DMB_USE_STATIC_MEMORY -DMB_MAX_BLOCKS=32

#include <smartmodbus/smartmodbus.h>

int main(void) {
    // In static mode, all memory is pre-allocated
    // No malloc/free calls - suitable for safety-critical systems

    mb_master_t master;  // Contains pre-allocated pools
    mb_config_t config = mb_config_default(MB_MODE_RTU);
    config.transport = my_transport;

    mb_master_init(&master, &config);

    // Use normally - library uses static pools internally
    uint16_t data[10];
    mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS, 100, 10, data);

    mb_master_cleanup(&master);
    return 0;
}
```

---

## Transport Layer Implementation

The transport layer provides the interface between Smart Modbus and your hardware.

### Transport Structure

```c
typedef struct {
    int (*send)(void *ctx, const uint8_t *data, size_t len);
    int (*recv)(void *ctx, uint8_t *buffer, size_t max_len, size_t *received);
    void (*delay_chars)(void *ctx, uint16_t chars);
    void *context;
} mb_transport_t;
```

### UART/RS485 Implementation

```c
typedef struct {
    int fd;              // File descriptor
    uint32_t baudrate;
    uint32_t char_time_us;
} uart_context_t;

int uart_send(void *ctx, const uint8_t *data, size_t len) {
    uart_context_t *uart = (uart_context_t *)ctx;
    ssize_t written = write(uart->fd, data, len);
    return (written == (ssize_t)len) ? (int)len : -1;
}

int uart_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    uart_context_t *uart = (uart_context_t *)ctx;

    // Set timeout
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(uart->fd, &readfds);

    int ret = select(uart->fd + 1, &readfds, NULL, NULL, &tv);
    if (ret > 0) {
        ssize_t n = read(uart->fd, buffer, max_len);
        if (n > 0) {
            *received = n;
            return 0;
        }
    }
    return -1;
}

void uart_delay(void *ctx, uint16_t chars) {
    uart_context_t *uart = (uart_context_t *)ctx;
    uint32_t delay_us = chars * uart->char_time_us;
    usleep(delay_us);
}

// Usage
uart_context_t uart_ctx = {
    .fd = open("/dev/ttyUSB0", O_RDWR),
    .baudrate = 9600,
    .char_time_us = 1000000 / (9600 / 11)  // 11 bits per char (8N1 + start/stop)
};

mb_config_t config = mb_config_default(MB_MODE_RTU);
config.transport.send = uart_send;
config.transport.recv = uart_recv;
config.transport.delay_chars = uart_delay;
config.transport.context = &uart_ctx;
```

### TCP Socket Implementation

```c
typedef struct {
    int socket_fd;
} tcp_context_t;

int tcp_send(void *ctx, const uint8_t *data, size_t len) {
    tcp_context_t *tcp = (tcp_context_t *)ctx;
    ssize_t sent = send(tcp->socket_fd, data, len, 0);
    return (sent == (ssize_t)len) ? (int)len : -1;
}

int tcp_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    tcp_context_t *tcp = (tcp_context_t *)ctx;

    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    setsockopt(tcp->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    ssize_t n = recv(tcp->socket_fd, buffer, max_len, 0);
    if (n > 0) {
        *received = n;
        return 0;
    }
    return -1;
}

// Usage
tcp_context_t tcp_ctx = {.socket_fd = my_socket};

mb_config_t config = mb_config_default(MB_MODE_TCP);
config.transport.send = tcp_send;
config.transport.recv = tcp_recv;
config.transport.delay_chars = NULL;  // No delay for TCP
config.transport.context = &tcp_ctx;
```

---

## Configuration Options

### Runtime Configuration

```c
mb_config_t config = mb_config_default(MB_MODE_RTU);

// Adjust PDU size (default: 253)
config.max_pdu_chars = 250;

// Adjust latency (affects merge decisions)
config.latency_chars = 5;  // Higher latency = more aggressive merging

// Adjust gap (RTU/ASCII only)
config.gap_chars = 4;  // 3.5 chars rounded up

// Adjust timeout
config.timeout_ms = 2000;  // 2 seconds
```

### Compile-Time Configuration

```cmake
# CMakeLists.txt or command line

# Enable static memory mode
set(MB_USE_STATIC_MEMORY ON)
set(MB_MAX_BLOCKS 32)
set(MB_MAX_PDUS 16)
set(MB_MAX_PLANS 16)

# Disable specific protocols
set(MB_ENABLE_ASCII OFF)

# Adjust PDU size
set(MB_MAX_PDU_CHARS 253)
```

---

## Error Handling

### Error Codes

```c
typedef enum {
    MB_SUCCESS = 0,
    MB_ERROR_INVALID_PARAM = -1,
    MB_ERROR_BUFFER_TOO_SMALL = -2,
    MB_ERROR_TIMEOUT = -3,
    MB_ERROR_CRC_MISMATCH = -4,
    MB_ERROR_LRC_MISMATCH = -5,
    MB_ERROR_INVALID_FRAME = -6,
    MB_ERROR_EXCEPTION_RESPONSE = -7,
    MB_ERROR_TRANSPORT = -8,
    MB_ERROR_OUT_OF_MEMORY = -9,
    MB_ERROR_NOT_SUPPORTED = -10,
    // ... more error codes
} mb_error_t;
```

### Error Handling Pattern

```c
int result = mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS,
                                   100, 10, data);

if (result != MB_SUCCESS) {
    // Get human-readable error message
    const char *error_msg = mb_error_to_string(result);
    printf("Error: %s\n", error_msg);

    // Handle specific errors
    switch (result) {
    case MB_ERROR_TIMEOUT:
        // Retry or check connection
        break;
    case MB_ERROR_CRC_MISMATCH:
        // Communication error, retry
        break;
    case MB_ERROR_EXCEPTION_RESPONSE:
        // Slave returned exception
        break;
    default:
        // Other errors
        break;
    }
}
```

---

## Best Practices

### 1. Initialization

```c
// ✅ Good: Check initialization result
int result = mb_master_init(&master, &config);
if (result != MB_SUCCESS) {
    // Handle error
}

// ❌ Bad: Ignore errors
mb_master_init(&master, &config);
```

### 2. Buffer Sizing

```c
// ✅ Good: Allocate sufficient buffer
uint16_t data[100];  // Enough for all requested data

// ❌ Bad: Buffer too small
uint16_t data[5];  // Requesting 10 registers - buffer overflow!
```

### 3. Transport Implementation

```c
// ✅ Good: Implement timeout in recv
int uart_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    // Use select() or poll() with timeout
    // Return error if timeout occurs
}

// ❌ Bad: Blocking recv without timeout
int uart_recv(void *ctx, uint8_t *buffer, size_t max_len, size_t *received) {
    read(fd, buffer, max_len);  // Blocks forever if no data!
}
```

### 4. Resource Cleanup

```c
// ✅ Good: Always cleanup
mb_master_init(&master, &config);
// ... use master ...
mb_master_cleanup(&master);

// ❌ Bad: Forget cleanup (memory leak in dynamic mode)
mb_master_init(&master, &config);
// ... use master ...
// No cleanup!
```

### 5. Error Checking

```c
// ✅ Good: Check all return values
int result = mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS,
                                   100, 10, data);
if (result != MB_SUCCESS) {
    printf("Error: %s\n", mb_error_to_string(result));
    return -1;
}

// ❌ Bad: Ignore return values
mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS, 100, 10, data);
// Did it succeed? Who knows!
```

### 6. Optimization Usage

```c
// ✅ Good: Use optimized read for non-contiguous addresses
uint16_t addresses[] = {100, 101, 102, 115, 116, 117};
mb_read_request_t request = {
    .slave_id = 1,
    .function_code = MB_FC_READ_HOLDING_REGISTERS,
    .addresses = addresses,
    .address_count = 6
};
mb_master_read_optimized(&master, &request, data, 6);

// ✅ Also good: Use single read for contiguous addresses
mb_master_read_single(&master, 1, MB_FC_READ_HOLDING_REGISTERS, 100, 10, data);
```

---

## Troubleshooting

### Problem: Timeout Errors

**Symptoms:** `MB_ERROR_TIMEOUT` returned frequently

**Solutions:**
1. Check physical connection
2. Verify baudrate matches slave device
3. Increase timeout: `config.timeout_ms = 2000;`
4. Check slave ID is correct
5. Verify slave device is powered and responding

### Problem: CRC/LRC Mismatch

**Symptoms:** `MB_ERROR_CRC_MISMATCH` or `MB_ERROR_LRC_MISMATCH`

**Solutions:**
1. Check for electrical noise on communication line
2. Verify baudrate settings
3. Check cable quality and length
4. Add termination resistors (RS485)
5. Reduce baudrate if errors persist

### Problem: Invalid Frame

**Symptoms:** `MB_ERROR_INVALID_FRAME`

**Solutions:**
1. Verify protocol mode matches slave (RTU vs ASCII)
2. Check frame timing (3.5 char gap for RTU)
3. Implement proper delay_chars callback
4. Check for buffer overruns

### Problem: Out of Memory

**Symptoms:** `MB_ERROR_OUT_OF_MEMORY` in dynamic mode

**Solutions:**
1. Switch to static memory mode: `-DMB_USE_STATIC_MEMORY=ON`
2. Reduce number of simultaneous requests
3. Check for memory leaks (always call `mb_master_cleanup()`)

### Problem: Poor Optimization

**Symptoms:** Library not merging blocks as expected

**Solutions:**
1. Check gap size - large gaps won't be merged
2. Adjust latency: `config.latency_chars = 5;` (higher = more merging)
3. Verify addresses are for same slave and function code
4. Check statistics: `mb_master_get_stats()`

### Problem: Compilation Errors

**Symptoms:** Build fails with missing symbols

**Solutions:**
1. Ensure all source files are included in CMakeLists.txt
2. Check include paths are correct
3. Verify C11 standard is enabled: `set(CMAKE_C_STANDARD 11)`
4. Link against smartmodbus library: `target_link_libraries(... smartmodbus)`

---

## Function Code Reference

| FC | Name | Merge Support | Usage |
|----|------|---------------|-------|
| 01 | Read Coils | ✅ Full | `MB_FC_READ_COILS` |
| 02 | Read Discrete Inputs | ✅ Full | `MB_FC_READ_DISCRETE_INPUTS` |
| 03 | Read Holding Registers | ✅ Full | `MB_FC_READ_HOLDING_REGISTERS` |
| 04 | Read Input Registers | ✅ Full | `MB_FC_READ_INPUT_REGISTERS` |
| 05 | Write Single Coil | ❌ No | `MB_FC_WRITE_SINGLE_COIL` |
| 06 | Write Single Register | ❌ No | `MB_FC_WRITE_SINGLE_REGISTER` |
| 15 | Write Multiple Coils | ⚠️ Contiguous | `MB_FC_WRITE_MULTIPLE_COILS` |
| 16 | Write Multiple Registers | ⚠️ Contiguous | `MB_FC_WRITE_MULTIPLE_REGISTERS` |

---

## Performance Tips

1. **Use Optimized Read**: For non-contiguous addresses, always use `mb_master_read_optimized()`
2. **Batch Requests**: Group related reads together
3. **Adjust Latency**: Higher latency values enable more aggressive merging
4. **Monitor Statistics**: Use `mb_master_get_stats()` to verify optimization
5. **Static Memory**: Use static mode for deterministic performance on MCU

---

## Additional Resources

- **Specification**: See `docs/smartmodbus spec.md` for algorithm details
- **Examples**: See `examples/` directory for working code
- **Source Code**: See `src/` for implementation details
- **Build Guide**: See `IMPLEMENTATION_STATUS.md` for build instructions

---

## Support

For issues, questions, or contributions:
- Check `TROUBLESHOOTING` section above
- Review example applications in `examples/`
- Consult specification documents in `docs/`

---

**Document Version:** 1.0.0
**Last Updated:** 2026-01-11
**Library Version:** 1.0.0
