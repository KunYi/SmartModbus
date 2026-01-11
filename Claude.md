# Smart Modbus Segmentation & Packing Specification

**Version:** v1.0
**Date:** 2026-01-11
**Purpose:** Universal, reusable Smart Modbus segmentation and packing mechanism supporting RTU/ASCII/TCP/IP protocols

---

## Overview

This specification defines a **char-based, gap-aware** Modbus communication optimization system that:

- Supports Modbus RTU, ASCII, and TCP/IP variants
- Works across RS-232, RS-485, UART, and TCP socket transports
- Minimizes round-trip overhead
- Maximizes data payload per response
- Supports all Modbus function codes
- Provides configurable PDU size limits
- Suitable for MCU, industrial gateways, and PC platforms

---

## Core Design Principles

### 1. Character-Based Cost Model
- **Single unit of cost:** character (char), not bits or time
- Protocol-agnostic approach works for RTU, ASCII, and TCP/IP
- Eliminates baudrate and timing dependencies

### 2. Round-Trip Optimization
- One request + one response = one round-trip
- Primary optimization target: minimize total rounds

### 3. Gap Counted Once
- RTU/ASCII: 3.5 chars inter-frame gap (typically implemented as 4)
- TCP/IP: no gap, but uses equivalent cost model
- Gap cost amortized across merged blocks

### 4. User Intent Focus
- Users specify: "What data do I need?"
- System generates: Optimal request plan automatically

### 5. Protocol-Transport Separation
- **Protocol layer:** Block merging, FFD packing, PDU construction
- **Transport layer:** RTU/ASCII/TCP/IP frame generation

### 6. Memory Efficient
- No dynamic memory allocation required
- Static arrays or MCU-friendly allocators supported

---

## Communication Model

### RTU/ASCII Flow
```
[Request Frame] → (3.5 char gap) → Slave Processing → [Response Frame]
```

### TCP/IP Flow
```
[MBAP Header + PDU] → TCP Socket → Slave → [MBAP Header + PDU]
```

### Key Assumptions
- RTU/ASCII gap marks round boundary
- TCP/IP uses MBAP header + PDU for cost calculation
- Network latency expressed in equivalent char count

---

## Character-Based Cost Model

### Fixed Overhead Per Round

#### Request Fixed Characters
| Field          | Chars |
|----------------|-------|
| Address        | 1     |
| Function Code  | 1     |
| Payload        | FC-dependent |
| CRC/LRC        | 2     |

#### Response Fixed Characters (excluding data)
| Field          | Chars |
|----------------|-------|
| Address        | 1     |
| Function Code  | 1     |
| Byte Count     | 1     |
| CRC/LRC        | 2     |
| **Total**      | **5** |

#### Round Overhead Formula
```
OVERHEAD_CHAR = REQ_FIXED_CHAR(fc) + RESP_FIXED_CHAR(fc) + GAP_CHAR + LATENCY_CHAR
```

Where:
- `GAP_CHAR = 4` (3.5 rounded up for RTU/ASCII)
- `GAP_CHAR = 0` (for TCP/IP)
- `LATENCY_CHAR`: User-configured or measured
- `REQ_FIXED_CHAR(fc)`: Function code dependent
- `RESP_FIXED_CHAR(fc)`: Typically 5 for read operations

### Variable Cost (Extra Data)

**Cost of reading unwanted data to merge non-contiguous blocks**

| Data Type | EXTRA_UNIT_CHAR |
|-----------|-----------------|
| Registers (FC03/04) | 2 bytes per register |
| Coils/Discrete (FC01/02) | 1 bit → 1/8 char (rounded up) |
| Write operations (FC05/06/15/16) | Not merged |

---

## Smart Merge Decision Algorithm

For two blocks with same function code and slave ID:

**Block A:** `[startAddr, lengthA]`
**Block B:** `[startAddrB, lengthB]`
**Gap:** `startAddrB - (startAddr + lengthA)`

### Merge Condition
```
IF (gap_units × EXTRA_UNIT_CHAR) < OVERHEAD_CHAR:
    MERGE blocks A and B
ELSE:
    Keep separate requests
```

### Rationale
- **Trade-off:** Read extra unwanted data vs. additional round-trip
- If gap cost < round-trip overhead → merge is beneficial
- Minimizes total communication cost

### Example Calculation (FC03 - Holding Registers)

**Scenario:**
- Block A: registers 100-109 (10 registers)
- Block B: registers 115-124 (10 registers)
- Gap: 5 registers

**Cost Analysis:**
```
OVERHEAD_CHAR = 4 (req) + 5 (resp) + 4 (gap) + 2 (latency) = 15 chars

Gap cost = 5 registers × 2 chars/register = 10 chars

Decision: 10 < 15 → MERGE (saves 5 chars per round)
```

---

## Function Code Support Matrix

| FC  | Type | Smart Segmentation | Notes |
|-----|------|-------------------|-------|
| 01  | Read Coils | ✔ Full Support | Bit-based merging |
| 02  | Read Discrete Inputs | ✔ Full Support | Bit-based merging |
| 03  | Read Holding Registers | ✔ Full Support | Register-based |
| 04  | Read Input Registers | ✔ Full Support | Register-based |
| 05  | Write Single Coil | ✖ No Merge | Individual requests only |
| 06  | Write Single Register | ✖ No Merge | Individual requests only |
| 15  | Write Multiple Coils | △ Limited | Contiguous only |
| 16  | Write Multiple Registers | △ Limited | Contiguous only |
| 22  | Mask Write Register | ✖ No Merge | Individual requests only |
| 23  | Read/Write Multiple | △ Limited | Separate read/write handling |

**Legend:**
- ✔ = Full gap-aware merge support
- △ = Partial support (contiguous blocks only)
- ✖ = No merging (safety/atomicity concerns)

---

## Packing Strategy (First-Fit Decreasing)

After gap-aware merging, pack blocks into PDU frames:

### Algorithm Steps

1. **Sort merged blocks** by data length (descending)
2. **Apply First-Fit Decreasing (FFD):**
   - For each block, find first PDU with sufficient space
   - If no PDU fits, create new PDU
3. **Constraints:**
   - Respect `MAX_PDU_CHAR` limit
   - Single function code per PDU
   - Single slave ID per PDU

### Objective
- Minimize total round-trips
- Maximize PDU utilization
- Stay within protocol limits

---

## Master API Design

### User Input (Semantic Layer)

Users provide:
```c
typedef struct {
    uint8_t slave_id;
    uint8_t function_code;
    uint16_t *addresses;      // Array of addresses (can be non-contiguous)
    uint16_t address_count;
} mb_read_request_t;
```

### System Configuration
```c
typedef struct {
    uint16_t max_pdu_chars;
    uint8_t latency_chars;
    mb_mode_t mode;           // RTU, ASCII, or TCP_IP
    mb_transport_t transport; // Send/receive callbacks
} mb_config_t;
```

### System Output
```c
typedef struct {
    uint8_t *frame_data;
    uint16_t frame_length;
    uint8_t function_code;
    uint16_t start_address;
    uint16_t quantity;
} mb_request_plan_t;
```

**Output:** Array of `mb_request_plan_t` (optimized request sequence)

---

## Transport Layer Abstraction

### Generic Transport Interface
```c
typedef struct {
    int (*send)(uint8_t *data, size_t len);
    int (*recv)(uint8_t *buf, size_t max_len, size_t *received);
    void (*delay_chars)(uint16_t chars); // Optional for gap timing
} mb_transport_t;
```

### Implementation Examples

**RTU/ASCII:**
```c
mb_transport_t uart_transport = {
    .send = uart_send,
    .recv = uart_recv,
    .delay_chars = uart_delay
};
```

**TCP/IP:**
```c
mb_transport_t tcp_transport = {
    .send = socket_send,
    .recv = socket_recv,
    .delay_chars = NULL  // No gap needed
};
```

---

## Modular Architecture

### System Components

| Module | Responsibility |
|--------|---------------|
| **char_model** | Cost calculation, gap thresholds |
| **gap_merge** | Block merging decision engine |
| **ffd_pack** | PDU packing optimization |
| **fc_policy** | Function code characteristics |
| **frame_builder** | RTU/ASCII/TCP frame generation |
| **transport** | Serial/TCP/IP callback handling |
| **master_api** | Unified user interface |

### Data Flow
```
User Request
    ↓
Gap-Aware Merge
    ↓
FFD Packing
    ↓
Frame Builder (RTU/ASCII/TCP)
    ↓
Transport Layer
    ↓
Physical Medium
```

---

## Configuration Parameters

### Runtime Configurable
- `MAX_PDU_CHAR`: Protocol limit (default: 253 for Modbus)
- `LATENCY_CHAR`: Network/slave response delay equivalent
- `MB_MODE`: RTU, ASCII, or TCP_IP
- `GAP_CHAR`: Inter-frame gap (RTU/ASCII only)

### Function Code Policies
```c
typedef struct {
    uint8_t fc;
    bool supports_merge;
    uint8_t req_fixed_chars;
    uint8_t resp_fixed_chars;
    uint16_t extra_unit_chars;  // For gap cost calculation
} fc_policy_t;
```

---

## Explicitly Out of Scope

This specification does **NOT** handle:

- ❌ Automatic retransmission on timeout
- ❌ Exception response retry logic
- ❌ Multi-slave scheduling/arbitration
- ❌ Timing jitter compensation
- ❌ Baudrate auto-detection
- ❌ Cable diagnostics

**Rationale:** Keep protocol optimization separate from error handling and physical layer concerns.

---

## Implementation Checklist

### Core Features
- [ ] Character-based cost model
- [ ] Gap-aware merge algorithm
- [ ] FFD packing engine
- [ ] RTU frame builder
- [ ] ASCII frame builder
- [ ] TCP/IP frame builder
- [ ] Transport abstraction layer
- [ ] Master API interface

### Configuration
- [ ] Dynamic MAX_PDU_CHAR
- [ ] Dynamic LATENCY_CHAR
- [ ] Mode switching (RTU/ASCII/TCP)
- [ ] Function code policy table

### Testing
- [ ] Unit tests for merge decisions
- [ ] FFD packing validation
- [ ] Cross-protocol compatibility
- [ ] MCU memory footprint verification

---

## Design Benefits Summary

1. **Protocol Agnostic:** Single codebase for RTU/ASCII/TCP/IP
2. **Automatic Optimization:** Users specify intent, system optimizes execution
3. **Minimal Round-Trips:** Gap-aware merging reduces communication overhead
4. **All Function Codes:** Comprehensive FC01-FC23 support with safety policies
5. **Configurable:** Runtime adjustment of PDU limits and latency
6. **Platform Independent:** MCU, gateway, and PC compatible
7. **Maintainable:** Clear separation between protocol logic and transport

---

## Usage Example

```c
// Configuration
mb_config_t config = {
    .max_pdu_chars = 253,
    .latency_chars = 2,
    .mode = MB_MODE_RTU,
    .transport = uart_transport
};

// User request (non-contiguous registers)
uint16_t addresses[] = {100, 101, 102, 115, 116, 117};
mb_read_request_t request = {
    .slave_id = 1,
    .function_code = 0x03,  // Read Holding Registers
    .addresses = addresses,
    .address_count = 6
};

// System generates optimal plan
mb_request_plan_t *plan;
uint8_t plan_count;
mb_optimize_request(&config, &request, &plan, &plan_count);

// Execute optimized requests
for (uint8_t i = 0; i < plan_count; i++) {
    mb_execute_request(&config, &plan[i]);
}
```

**Result:** System automatically merges registers 100-102 and 115-117 into minimal round-trips based on gap cost analysis.

---

## References

- Modbus Application Protocol Specification V1.1b3
- Modbus over Serial Line Specification V1.02
- Modbus Messaging on TCP/IP Implementation Guide V1.0b

---

**End of Specification**
