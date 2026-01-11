# Smart Modbus

Smart Modbus 是一個高效能、可重用的 Modbus 封包優化程式庫。它採用獨創的「以字元為基礎 (Char-based)」成本模型與「單次間隙 (Gap-once)」演算法，旨在透過智能合併不連續的資料區塊，極小化通訊的 Round-trip 次數，從而最大化工業網路的吞吐量。

![GenAI Driven](https://img.shields.io/badge/GenAI-Driven-blueviolet?logo=google-gemini&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

## 1. 核心設計理念 (Design Philosophy)

在工業通訊中，通訊延遲（Latency）通常遠大於資料傳輸時間。本程式庫的核心邏輯是：「寧可多讀一點沒用的資料，也絕對要避免多跑一次 Round-trip。」

- 唯一成本單位：統一使用 char (byte) 計算通訊開銷，不依賴特定鮑率或硬體定時器。
- 傳輸層解耦合：通訊邏輯與傳播媒介（UART/Socket）完全分離。
- 零動態記憶體 (Optional)：設計時考慮 MCU 兼容性，支援靜態記憶體配置。

## 2. 智能分段與合併流程

本程式庫會自動將多個零散的資料請求優化為最精簡的 Modbus 請求計畫（Request Plan）。

- Gap-aware Merge：根據成本模型判斷兩個相鄰區塊是否值得合併。
- FFD Packing：使用 First-Fit Decreasing 演算法將區塊填入最大 PDU (Protocol Data Unit)。
- Frame Building：生成對應 RTU/ASCII/TCP 的封裝。

## 3. 成本模型 (Char-based Cost Model)

### 3.1 固定開銷 (Round Overhead)

每次請求的固定成本 計算如下：

| 項目	 | 說明	                      | 預設值 (RTU)    |
|-------|----------------------------|---------------:|
|	| 地址 + 功能碼 + CRC         |	4 chars        |
|	| 地址 + 功能碼 + 長度 + CRC   | 5 chars        |
|	| Modbus 3.5 字元間隙	     | 4 chars        |
|	| 系統處理與往返延遲	       | 依環境設定       |

### 3.2 合併判定準則

當兩個資料區塊之間存在 gap_units 的間距時，系統會執行以下判定：

Merge if:

    - Register-based (FC03/04): EXTRA_UNIT_CHAR = 2 bytes.
    - Bit-based (FC01/02): EXTRA_UNIT_CHAR = 1/8 byte.

## 4. 功能碼支援矩陣

| 功能碼 (FC) | 類型 | 智能分段 |  備註       |
|-----------:|-----|--------:|-------------|
| 01 / 02 | Read Coils / Discrete | ✔ | Bit-based 合併 |
| 03 / 04 | Read Registers | ✔ | Word-based 合併 |
| 05 / 06 | Write Single | ✖ | 單獨執行 |
| 15 / 16 | Write Multiple | △ | 僅合併物理連續區塊 |
| 23 | Read/Write Multiple | △ | 依分區策略處理 |

## 5. 快速上手 (Quick Start)

...

---

## LICENSE

...

---

## CONTRIBUTE

...

---

## 🤖 GenAI Driven Development

本專案的核心邏輯、架構設計及規格文件均由 **Generative AI** 驅動開發。

* **演算法優化**：AI 協助處理 FFD (First-Fit Decreasing) 演算法與 Modbus 封包邊界的邊際案例。
* **開發效率**：透過 AI 協作，大幅縮短了從通訊理論到 C 語言抽象介面實作的轉換週期。
* **透明度聲明**：雖然核心逻辑經由 AI 生成，但所有通訊時序與安全性邏輯均經過人工審閱與工業標準驗證。
