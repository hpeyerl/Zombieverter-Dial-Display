# ZombieVerter CAN Protocol Reference

## Overview

This document describes the CAN bus communication between the M5Stack Dial and ZombieVerter motor controller.

---

## Control Parameters (M5Dial → ZombieVerter)

These parameters are **sent by M5Dial** and **received by ZombieVerter** (configured as **Rx** in ZombieVerter CAN table).

### Configurable Control Parameters

| CAN ID | Parameter | Param ID | Data Format | Values | Description |
|--------|-----------|----------|-------------|--------|-------------|
| 0x300 | Gear | 27 | Byte 0: 0-3 | 0=LOW, 1=HIGH, 2=AUTO, 3=HI/LO | Transmission gear selection |
| 0x301 | Motor Active | 129 | Byte 0: 0-3 | 0=MG1 only, 1=MG2 only, 2=MG1+MG2, 3=Blended | Motor operation mode |
| 0x302 | Regen Max | 61 | Bytes 0-1: int16 | -35 to 0 | Maximum regenerative braking percentage |

**Note:** These must be configured as **Receive (Rx)** in ZombieVerter's CAN mapping table.

**M5Dial Behavior:**
- Uses **optimistic updates** - display changes immediately when encoder rotates
- Sends CAN command to ZombieVerter
- Does not wait for confirmation (since ZombieVerter Rx-only doesn't send responses)

---

## Telemetry Parameters (ZombieVerter → M5Dial)

These parameters are **broadcast by ZombieVerter** and **received by M5Dial** (configured as **Tx** in ZombieVerter CAN table).

### Required Telemetry Parameters

Configure these as **Transmit (Tx)** in ZombieVerter CAN table:

| CAN ID | Parameter | Param ID | Data Format | Units | Description |
|--------|-----------|----------|-------------|-------|-------------|
| 0x126 | Inverter Temp | 6 | Bytes 4-5: int16 | °C | Heatsink temperature |
| 0x210 | 12V Supply | 8 | Bytes 4-5: int16 | V | Auxiliary 12V voltage |
| 0x257 | Motor Speed | 1 | Bytes 0-1: int16 | RPM | Motor shaft speed |
| 0x355 | Battery SOC | 7 | Bytes 0-1: int16 | % | State of charge |
| 0x356 | Motor Temp | 5 | Bytes 4-5: int16 | °C | Motor winding temperature |
| 0x300 | Gear (echo) | 27 | Byte 0: 0-3 | - | Current gear (if Tx enabled) |

**Note:** If ZombieVerter is configured with 0x300/0x301/0x302 as **Rx only**, those values will NOT be echoed back. The M5Dial handles this with optimistic display updates.

---

## IVT-S Shunt Messages (Read-Only)

The M5Dial also receives data from the IVT-S current shunt (if installed). These are broadcast on the CAN bus and do not need ZombieVerter configuration.

| CAN ID | Measurement | Data Format | Units | Description |
|--------|-------------|-------------|-------|-------------|
| 0x411 | Current (I) | Bytes 2-4: int24 | mA | Battery pack current |
| 0x521 | Voltage 1 (U1) | Bytes 2-4: int24 | mV | Optional voltage 1 |
| 0x522 | Voltage 2 (U2) | Bytes 2-4: int24 | mV | **Main battery voltage** |
| 0x523 | Voltage 3 (U3) | Bytes 2-4: int24 | mV | Optional voltage 3 |
| 0x526 | Temperature | Bytes 2-4: int24 | 0.1°C | Shunt temperature |
| 0x527 | Power (P) | Bytes 2-4: int24 | W | Calculated power |
| 0x528 | Charge (As) | Bytes 2-4: int24 | As | Amp-seconds counter |

**Format:** 6-byte messages with 24-bit signed integer in bytes 2-4 (little-endian)

---

## BMS Messages (Optional)

If using a Victron/REC-compatible BMS (like SimpBMS), these messages are available:

| CAN ID | Data | Description |
|--------|------|-------------|
| 0x351 | Battery limits | Charge/discharge current limits, voltage limits |
| 0x355 | SOC % | State of charge (may conflict with ZombieVerter 0x355) |
| 0x356 | Pack voltage | Total pack voltage (may conflict with ZombieVerter 0x356) |
| 0x373 | Cell data | Min/Max cell voltages and temperatures |
| 0x35A | Status flags | Charging/discharging status |

---

## Parameter Fetching (SDO Protocol)

**⚠️ CURRENTLY NOT TESTED**

The M5Dial includes code to request parameters from ZombieVerter using CANopen SDO (Service Data Object) protocol, but this functionality has **not been fully tested**.

### SDO Request Format
- CAN ID: `0x600 + NODE_ID` (typically 0x603)
- Command: `0x40` (SDO read request)
- Bytes 1-2: Parameter ID (little-endian)

### SDO Response Format  
- CAN ID: `0x580 + NODE_ID` (typically 0x583)
- Command: `0x43` (SDO read response, 4 bytes)
- Bytes 4-7: Parameter value (little-endian)

**Current Status:**
- SDO requests are implemented in code
- Responses are parsed but may show "Unexpected SDO command" errors
- This feature needs testing and validation
- For now, rely on broadcast telemetry messages (0x126, 0x210, 0x257, etc.)

---

## ZombieVerter Configuration Example

### CAN Mapping Table Settings

**Receive Parameters (Commands from M5Dial):**
```
ID: 27  (Gear)         → CAN ID: 0x300, Direction: Rx, Offset: 0
ID: 129 (Motor Active) → CAN ID: 0x301, Direction: Rx, Offset: 0  
ID: 61  (Regen Max)    → CAN ID: 0x302, Direction: Rx, Offset: 0
```

**Transmit Parameters (Telemetry to M5Dial):**
```
ID: 6  (tmphs)      → CAN ID: 0x126, Direction: Tx, Offset: 4
ID: 8  (uaux)       → CAN ID: 0x210, Direction: Tx, Offset: 4
ID: 1  (speed)      → CAN ID: 0x257, Direction: Tx, Offset: 0
ID: 7  (SOC)        → CAN ID: 0x355, Direction: Tx, Offset: 0
ID: 5  (tmpm)       → CAN ID: 0x356, Direction: Tx, Offset: 4
```

**Important:** ZombieVerter does **not support Tx+Rx** on the same CAN ID. Each ID must be either Tx or Rx only.

---

## Message Update Rates

- **Telemetry broadcasts:** Typically 100ms (10 Hz)
- **IVT-S messages:** 1000ms (1 Hz) per message ID
- **Control commands:** Sent immediately when encoder rotates
- **SDO requests:** On-demand (not used in normal operation)

---

## Troubleshooting

### No Parameter Updates
- Check ZombieVerter CAN table has Tx enabled for required IDs
- Verify CAN_NODE_ID in Config.h matches ZombieVerter
- Confirm CAN bus has proper 120Ω termination

### Control Commands Not Working  
- Verify 0x300/0x301/0x302 configured as **Rx** in ZombieVerter
- Check CAN TX/RX wiring (GPIO 2 and GPIO 1)
- Monitor serial output for "CAN TX:" messages

### Values Show "NO DATA"
- Parameter not configured in ZombieVerter CAN table
- CAN message not being broadcast
- Wrong parameter ID in params.json

---

## Additional Resources

- ZombieVerter CAN Documentation: https://openinverter.org/wiki/CAN_Mapping
- IVT-S Protocol: Isabellenhutte datasheet
- Victron BMS Protocol: REC BMS CAN specification
