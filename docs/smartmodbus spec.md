# Smart Modbus RTU/ASCII/TCP/IP 智能分段與封包規格（Char-based, Gap-once）

版本：v1.0
作者：工程團隊
日期：2026-01-11

---

## 1. 設計目標 (Scope)

本規格定義一個 **通用、可重用的 Smart Modbus 分段與封包機制**，支援：

- Modbus RTU / ASCII / TCP/IP
- RS-232 / RS-485 / UART / TCP socket
- 半雙工或全雙工 master–slave 通訊
- 最小化 **Round Trip 次數**
- 最大化 **單次回應有效資料量**
- 適用 **所有 Modbus 功能碼（Function Code）**
- 可參數化 **最大 PDU bytes**
- 適合 MCU / 工業網關 / PC 平台
---

## 2. 基本設計原則 (Design Principles)

1. **以 char 為唯一成本單位**
   - 不依賴 bit / baudrate / time
   - 對 RTU / ASCII / TCP/IP 均適用

2. **Round-trip 為最佳化單位**
   - 一次 request + 一次 response = 一個 round

3. **Gap 只計一次**
   - RTU / ASCII 為 3.5 chars（實作可取 4）
   - TCP/IP 無 gap，但可沿用同一成本模型

4. **使用者只描述「我要哪些資料」**
   - 系統自動生成最佳 Request Plan

5. **Protocol 與 Transport 層分離**
   - Protocol 層：Block merge + FFD Pack + PDU
   - Transport 層：RTU / ASCII / TCP/IP

6. **不依賴動態記憶體**
   - 可選用靜態陣列或 MCU-friendly allocator

---

## 3. 通訊抽象模型 (Communication Model)

### 3.1 RTU / ASCII

一個 Modbus RTU/ASCII round-trip 抽象為：

[ Request Frame ] → (RTU Gap) → Slave Processing / Turnaround → [ Response Frame ]

### 3.2 TCP/IP

[ MBAP Header + PDU ] → TCP Socket → Slave → [ MBAP Header + PDU ]


### 關鍵假設

- RTU / ASCII gap = round boundary
- TCP/IP 以 MBAP header + PDU 計算成本
- latency 用等價 char 數表示

---

## 4. 成本模型 (Char-based Cost Model)

### 4.1 固定成本（Round Overhead）

#### Request 固定 char 數

| 欄位          | chars |
|---------------|-------|
| Address       | 1     |
| Function Code | 1     |
| Payload       | 視 FC 而定 |
| CRC / LRC     | 2     |

#### Response 固定 char 數（不含資料）

| 欄位         | chars |
|--------------|-------|
| Address      | 1     |
| Function Code| 1     |
| Byte Count   | 1     |
| CRC / LRC    | 2     |
| **合計**    | 5     |

#### Round Overhead 定義

OVERHEAD_CHAR = REQ_FIXED_CHAR(fc) + RESP_FIXED_CHAR(fc) + GAP_CHAR + LATENCY_CHAR

- `GAP_CHAR = 3.5` (取整 4)
- `LATENCY_CHAR`：由實作或配置提供
- `REQ_FIXED_CHAR(fc)`：依功能碼決定
- TCP/IP 可將 `GAP_CHAR = 0`

---

### 4.2 變動成本（Extra Data Cost）

**為了合併不連續區間而「多讀但不需要」的資料量**

| 功能類型               | EXTRA_UNIT_CHAR |
|------------------------|----------------|
| Register-based (FC03/04)| 2 bytes       |
| Coil / Discrete (FC01/02)| 1 bit → 1/8 char (取整) |
| Write 類 (FC05/06/15/16)| 不合併，單獨 request |

---

## 5. 智能 Merge 判斷規則（核心）

對於同一功能碼、同一 slave 的兩個 block：

---

## 5. 智能 Merge 判斷規則（核心）

對於同一功能碼、同一 slave 的兩個 block：

- block A: `[addr, lenA]`
- block B: `[addrB, lenB]`
- gap = `addrB - (addr + lenA)`

### Merge 條件：

merge if:
gap_units * EXTRA_UNIT_CHAR < OVERHEAD_CHAR

- **寧可多讀一點資料，也避免額外 round-trip**
- 只要比「再來一次 round-trip」便宜，就值得合併
- TCP/IP 可直接忽略 gap

---

## 6. 功能碼支援矩陣（All Function Codes）

| FC | 類型 | 支援智能分段 | 備註 |
|----|------|--------------|------|
| 01 | Read Coils | ✔ | bit-based |
| 02 | Read Discrete Inputs | ✔ | bit-based |
| 03 | Read Holding Registers | ✔ | register |
| 04 | Read Input Registers | ✔ | register |
| 05 | Write Single Coil | ✖ | 不合併 |
| 06 | Write Single Register | ✖ | 不合併 |
| 15 | Write Multiple Coils | △ | 僅連續 |
| 16 | Write Multiple Registers | △ | 僅連續 |
| 22 | Mask Write Register | ✖ | 單獨 |
| 23 | Read/Write Multiple Registers | △ | 分區處理 |

---

## 7. Packing 策略（FFD）

在完成 **Gap-aware merge** 後，對所有 block：

1. Gap-aware merge 完成後，依資料長度遞減排序
2. 使用 **First-Fit Decreasing (FFD)** 將 block 填入 PDU
3. 目標：
   - 不超過 protocol max PDU
   - 最少 round 數

---

## 8. Master API 抽象（語義層）

使用者只需提供：

- Slave ID
- Function Code
- Address list（可不連續）
- 系統參數：
  - `LATENCY_CHAR`
  - `MAX_PDU_CHAR`
  - `MB_MODE` (RTU / ASCII / TCP/IP)
- Transport Callback（send / recv）

系統輸出：

- 一組 **Optimized Request Plan**
- 每個 plan = 一個 Frame (RTU / ASCII / TCP/IP)

---

## 9. Transport 層抽象

```c
typedef struct {
    int (*send)(uint8_t *data, size_t len);
    int (*recv)(uint8_t *data, size_t max_len, size_t *recv_len);
} mb_transport_t;
```

- RTU / ASCII → UART callback
- TCP/IP → socket callback
- Protocol 層保持共用

--
## 10. 非目標（Explicitly Out of Scope）

- 不處理 retransmission
- 不處理 exception retry
- 不處理 multi-slave scheduling
- 不處理 timing jitter

---

## 11. 實作落地

模組化設計：

| 模組	          | 功能                        |
|-----------------|-----------------------------|
| char-model      |	計算 cost, gap 等級          |
| gap-aware merge | 	Block 合併決策         |
| FFD pack	      | 最優化 PDU 封包               |
| FC policy	      | 功能碼特性                    |
| Frame builder   |	RTU / ASCII / TCP Frame 生成 |
| Transport	      | Serial / TCP/IP callback    |

- Master API 統一接口
- 可動態設定 MAX_PDU_CHAR、MB_MODE、LATENCY_CHAR
- MCU / Gateway / PC 均可使用

---

## 12. 設計特性總結

1. **完全 char-based**，符合 RTU / ASCII / TCPIP 標準
2. **自動 gap-aware merge**，只算一次 gap
3. **支援 RTU / ASCII / TCP/IP 三種模式**
4. **支援所有功能碼**，寫入操作可選擇保護
5. **可參數化 latency / max PDU**
6. **MCU / Gateway / PC 都可用**
7. **Protocol** 與 **Transport** 層分離，Master API 保持一致

---


