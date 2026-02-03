# ZombieVerter M5Stack Dial Display

A professional rotary controller and display interface for the ZombieVerter electric vehicle motor controller using the M5Stack Dial (ESP32-S3).

![M5Stack Dial](https://static-cdn.m5stack.com/resource/docs/products/core/M5Dial/img-0f7dda07-1853-46ea-a8e9-f62e44d4fc12.webp)

## Features

### Real-Time Monitoring
- **11 Information Screens** displaying live vehicle data
- Motor RPM, power output, temperatures, voltages, and currents
- Battery state of charge (SOC) and BMS data
- Efficiency metrics and system status

### Rotary Control Interface
- **Gear Selection** - Rotate to switch between LOW/HIGH/AUTO/HI-LO modes
- **Motor Mode** - Choose between MG1 only, MG2 only, MG1+MG2, Blended
- **Regenerative Braking** - Adjust regen strength from -35% to 0%
- Smooth 1:1 encoder sensitivity for precise control

### Direct CAN Bus Communication
- Receives real-time telemetry from ZombieVerter via CAN bus
- Sends control commands using dedicated CAN message IDs
- Optimistic UI updates for instant visual feedback
- No SDO overhead - direct parameter control

### Built-in WiFi Configuration
- Optional WiFi access point for wireless parameter adjustment
- Web interface for advanced settings (configurable)
- Button-activated WiFi mode

## Hardware Requirements

- **M5Stack Dial** (ESP32-S3 based rotary controller)
- **CAN Transceiver Module** - Connects to Grove Port (GPIO 2 TX, GPIO 1 RX)
- **ZombieVerter Motor Controller** with CAN bus enabled
- 12V power supply for M5Dial (or USB-C for testing)

### Wiring

**M5Stack Dial Grove Port to CAN Transceiver:**
- Pin 1 (Yellow) - GPIO 2 → CAN TX
- Pin 2 (White) - GPIO 1 → CAN RX
- Pin 3 (Red) - 5V → VCC
- Pin 4 (Black) - GND → GND

**CAN Transceiver to ZombieVerter:**
- CAN_H → ZombieVerter CAN_H
- CAN_L → ZombieVerter CAN_L
- Ensure proper 120Ω termination resistors on CAN bus

**Note:** The M5Stack Dial has one external Grove port on the bottom/side of the device. Use GPIO 2 (TX) and GPIO 1 (RX) for CAN communication.

See [M5DIAL_PINOUT_CONFIRMED.md](M5DIAL_PINOUT_CONFIRMED.md) for detailed pinout information.

## ZombieVerter CAN Configuration

The M5Dial communicates with ZombieVerter via CAN bus using two types of messages:

### Control Parameters (M5Dial → ZombieVerter)

Configure these CAN message IDs as **Receive (Rx)** in ZombieVerter web interface:

| Parameter | CAN ID | Direction | Data Format | Values |
|-----------|--------|-----------|-------------|--------|
| Gear | 0x300 | Rx | Byte 0: 0-3 | 0=LOW, 1=HIGH, 2=AUTO, 3=HI/LO |
| Motor Active | 0x301 | Rx | Byte 0: 0-3 | 0=MG1 only, 1=MG2 only, 2=MG1+MG2, 3=Blended |
| Regen Max | 0x302 | Rx | Bytes 0-1: int16 | -35 to 0 (percentage) |

### Telemetry Parameters (ZombieVerter → M5Dial)

Configure these CAN message IDs as **Transmit (Tx)** in ZombieVerter web interface:

| Parameter | CAN ID | Direction | Data Format | Units |
|-----------|--------|-----------|-------------|-------|
| Inverter Temp (tmphs) | 0x126 | Tx | Bytes 4-5: int16 | °C |
| 12V Supply (uaux) | 0x210 | Tx | Bytes 4-5: int16 | V |
| Motor Speed (speed) | 0x257 | Tx | Bytes 0-1: int16 | RPM |
| Battery SOC | 0x355 | Tx | Bytes 0-1: int16 | % |
| Motor Temp (tmpm) | 0x356 | Tx | Bytes 4-5: int16 | °C |

**Important Notes:**
- These IDs should be configured as **Rx only** for control parameters (0x300-0x302)
- These IDs should be configured as **Tx only** for telemetry (0x126, 0x210, 0x257, 0x355, 0x356)
- ZombieVerter does not support bidirectional (Tx+Rx) on the same CAN ID
- The M5Dial uses optimistic updates - display changes immediately without waiting for CAN confirmation

**Parameter Fetching (SDO):**
- The M5Dial includes code for requesting parameters via CANopen SDO protocol
- **⚠️ This feature is currently NOT TESTED and may not work correctly**
- For reliable operation, use the broadcast telemetry messages listed above

See [CAN_PROTOCOL.md](CAN_PROTOCOL.md) for complete CAN protocol documentation including IVT-S shunt messages and BMS integration.

All other parameters (speed, voltage, current, temperatures, etc.) are received from ZombieVerter's normal broadcast messages.

## Installation

### Using PlatformIO (Recommended)

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
3. Clone this repository:
   ```bash
   git clone https://github.com/robertwa1974/Zombieverter-Dial-Display.git
   cd Zombieverter-Dial-Display
   ```
4. Open the project folder in VS Code
5. Connect M5Stack Dial via USB-C
6. Click "Upload" in PlatformIO toolbar (→ icon)

### Using Arduino IDE

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs": `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Install required libraries via Library Manager:
   - M5Dial
   - ESP32-TWAI-CAN (or ESP32CAN)
   - ESP32Encoder
   - OneButton
4. Open `src/main.cpp` in Arduino IDE
5. Select board: **ESP32S3 Dev Module**
6. Upload to M5Dial

## Quick Start

1. **Power on** the M5Dial - It boots to the Dashboard screen
2. **Rotate encoder** to navigate through screens
3. **Press button** to toggle WiFi mode (if needed)
4. **Long-press button** to return to Dashboard

### Control Screens

**GEAR Screen:**
- Rotate encoder clockwise: LOW → HIGH → AUTO → HI-LO
- Rotate counter-clockwise: reverse direction
- Changes are sent immediately to ZombieVerter

**MOTOR Screen:**
- Rotate to select: MG1 only → MG2 only → MG1+MG2 → Blended
- Motor mode changes instantly

**REGEN Screen:**
- Rotate to adjust regenerative braking percentage
- Range: -35% (maximum regen) to 0% (no regen)
- Fine adjustment: 1% per encoder click

## Screens Overview

1. **Dashboard** - Main overview with key metrics
2. **Power** - Real-time power, voltage, and current
3. **Temperature** - Motor and inverter temperatures
4. **Battery** - SOC, voltage, and cell information
5. **BMS** - Detailed battery management data
6. **GEAR** - Transmission gear selection
7. **MOTOR** - Motor mode selection
8. **REGEN** - Regenerative braking adjustment
9. **WiFi** - WiFi configuration status
10. **Settings** - System settings (future expansion)

## Configuration

### CAN Bus Settings
Edit `include/Config.h`:
```cpp
#define CAN_TX_PIN 2   // Grove Port Pin 1 (Yellow)
#define CAN_RX_PIN 1   // Grove Port Pin 2 (White)
#define CAN_NODE_ID 3  // ZombieVerter node ID (usually 3, sometimes 1)
```

### WiFi Settings
```cpp
#define WIFI_AP_SSID "ZombieVerter-Dial"
#define WIFI_AP_PASSWORD "zombieverter"
```

### Parameter Mapping
Edit `data/params.json` to customize which parameters are displayed and their scaling factors.

## Troubleshooting

### No CAN Communication
- Check CAN transceiver wiring (TX/RX pins correct?)
- Verify 120Ω termination resistors on CAN bus
- Confirm ZombieVerter CAN bus is enabled
- Check CAN_NODE_ID matches ZombieVerter configuration

### Encoder Not Responding
- Encoder works for screen navigation but not on GEAR screen → Check ZombieVerter CAN Rx configuration
- Encoder too sensitive (multiple screens per click) → Update to latest firmware with sensitivity fix

### Display Shows "NO DATA"
- Parameter not configured in ZombieVerter CAN mapping
- CAN message not being broadcast
- Check `data/params.json` parameter ID matches ZombieVerter

See [CAN_TROUBLESHOOTING.md](CAN_TROUBLESHOOTING.md) for detailed debugging steps.

## Development

### Project Structure
```
├── src/
│   ├── main.cpp          # Main application logic
│   ├── CANData.cpp       # CAN message parsing and parameter management
│   ├── UIManager.cpp     # Screen rendering and display logic
│   ├── InputManager.cpp  # Encoder and button handling
│   ├── WiFiManager.cpp   # WiFi AP and web server
│   └── Hardware.cpp      # Hardware initialization
├── include/
│   ├── Config.h          # Pin definitions and settings
│   ├── CANData.h         # CAN data structures
│   ├── UIManager.h       # UI screen definitions
│   ├── InputManager.h    # Input handling interface
│   └── WiFiManager.h     # WiFi configuration
├── data/
│   └── params.json       # Parameter definitions and scaling
└── platformio.ini        # PlatformIO configuration
```

### Adding New Screens
1. Add screen enum to `include/UIManager.h`
2. Implement draw function in `UIManager.cpp`
3. Add screen to rotation in `main.cpp` encoder handler

### Adding New Parameters
1. Define parameter in `data/params.json`
2. Add CAN parsing in `CANData.cpp::handleGenericMessage()`
3. Display parameter in appropriate screen's draw function

## Documentation

- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Detailed build and upload guide
- [CAN_PROTOCOL.md](CAN_PROTOCOL.md) - Complete CAN bus protocol reference
- [SCREENS_GUIDE.md](SCREENS_GUIDE.md) - Complete screen-by-screen documentation
- [PARAMS_JSON_GUIDE.md](PARAMS_JSON_GUIDE.md) - Parameter configuration reference
- [WIFI_FEATURE.md](WIFI_FEATURE.md) - WiFi setup and usage
- [CAN_TROUBLESHOOTING.md](CAN_TROUBLESHOOTING.md) - CAN bus debugging guide

## Credits

- **Original LilyGO Implementation:** [Jamie Jones (jamiejones85)](https://github.com/jamiejones85/ZombieVerterDisplay) - Initial CAN display code for LilyGO T-Display
- **M5Dial Adaptation:** Robert Wahler - Port to M5Stack Dial with rotary encoder control
- **Hardware:** [M5Stack Dial](https://shop.m5stack.com/products/m5stack-dial-esp32-s3-smart-rotary-knob-w-1-28-round-touch-screen)
- **Motor Controller:** [ZombieVerter](https://openinverter.org/wiki/Zombieverter)
- **OpenInverter Project:** [openinverter.org](https://openinverter.org)

This project builds upon Jamie Jones' excellent work creating a CAN bus display for ZombieVerter using the LilyGO T-Display. The core CAN parsing logic, parameter management system, and display concepts are based on their original [ZombieVerterDisplay](https://github.com/jamiejones85/ZombieVerterDisplay) implementation. This version adapts the code for the M5Stack Dial hardware and adds rotary encoder control capabilities.

See [CREDITS.md](CREDITS.md) for detailed acknowledgments.

## License

This project is open source. Feel free to use, modify, and distribute.

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Submit a pull request with detailed description

## Support

- **Issues:** [GitHub Issues](https://github.com/robertwa1974/Zombieverter-Dial-Display/issues)
- **OpenInverter Forum:** [forum.openinverter.org](https://openinverter.org/forum/)

## Version History

### v1.0.0 (2025-02-03)
- Initial release
- 11 information screens
- Rotary encoder control for Gear/Motor/Regen
- Direct CAN bus control
- Optimistic UI updates
- WiFi configuration mode
- Real-time telemetry display

---

**Note:** This is a community project and is not officially affiliated with the ZombieVerter or OpenInverter projects.
