# STM32 CAN Telemetry Node

Embedded systems project based on the STM32 NUCLEO-F401RE platform demonstrating real-time firmware development, communication interfaces, sensor integration, debugging workflows, and telemetry transmission.

This project is being developed incrementally to demonstrate practical embedded software engineering skills relevant to automotive and mobility systems.

---

# Hardware Platform

- STM32 NUCLEO-F401RE
- STM32CubeIDE
- UART serial debugging via ST-LINK Virtual COM Port

Future integration:
- CAN communication (SN65HVD230)
- Rotary encoder sensor
- GNSS (u-blox NEO-M8N)
- SPI communication
- Python-based CAN telemetry tools

---

# Development Roadmap

- [x] Step 1 — STM32 board bring-up and UART debugging
- [x] Step 2 — Rotary encoder GPIO integration
- [x] Step 3 — SPI loopback packet communication
- [x] Step 4 — CAN-ready hardware architecture and interface preparation
- [x] Step 5 — Python telemetry and serial logging tools
- [x] Step 6 — Memory management and telemetry buffering
- [x] Step 7a — Bootloader and firmware update workflow
- [x] Step 7b — UART Bootloader Workflow Demo
- [ ] Step 8 — GNSS (NEO-M8N) UART integration
- [ ] Step 9 — MCP2515 SPI-CAN controller integration

---

# Step 1 — STM32 Board Bring-up

## Objective

Verify:
- STM32 firmware flashing
- GPIO control
- UART serial communication
- STM32CubeIDE workflow
- ST-LINK debugging interface

---

## Implemented Features

- Configured STM32 NUCLEO-F401RE using STM32CubeIDE
- Enabled USART2 virtual COM port for UART debugging
- Implemented LED heartbeat on onboard LED (PA5)
- Verified firmware flashing and continuous execution
- Verified UART serial communication through USB ST-LINK interface

---

## UART Output

```text
System alive
System alive
System alive
```

## GPIO Heartbeat

The onboard LD2 LED connected to PA5 toggles every 500 ms to indicate active firmware execution.

## STM32CubeIDE Pin Configuration
| Peripheral  | Pin | Function        |
| ----------- | --- | --------------- |
| USART2_TX   | PA2 | UART transmit   |
| USART2_RX   | PA3 | UART receive    |
| GPIO_Output | PA5 | LD2 onboard LED |


## Project Structure
```
firmware/
docs/
images/
logic_analyzer/
python_tools/
```
## Images

### STM32CubeIDE Configuration

<p align="center">
<img src="images/step1_cubeide_config.png" width="400"/>
</p>



### UART Serial Output
<p align="center">
<img src="images/step1_uart_output.png" width="400"/>
</p>

### NUCLEO-F401RE Bring-up
<p align="center">
<img src="images/step1_nucleo_bringup.jpg" width="400"/>
</p>

<p align="center">
<img src="images/step1_nucleo_bringup.gif" width="400"/>
</p>

# Step 2 — Rotary Encoder GPIO Integration

## Objective

Interface a mechanical rotary encoder with the STM32 NUCLEO-F401RE using GPIO inputs and monitor encoder state transitions through UART debugging.

This step demonstrates external sensor integration, GPIO pull-up configuration, and real-time signal monitoring in an embedded firmware environment.

---

## Hardware Used

- STM32 NUCLEO-F401RE
- EN11-HSM rotary encoder
- Dupont jumper wires
- STM32CubeIDE
- PuTTY serial terminal

---

## Encoder Interface

The encoder was connected using two GPIO input channels and one common ground connection.

### Encoder Wiring

| Encoder Pin | STM32 Pin | Function |
|---|---|---|
| Left pin | D9 / PC7 | Encoder channel B |
| Middle pin | GND | Common ground |
| Right pin | D10 / PB6 | Encoder channel A |

---

## GPIO Configuration

Both encoder GPIO inputs were configured using STM32 internal pull-up resistors.

| STM32 Pin | Configuration |
|---|---|
| PB6 | GPIO_Input + Pull-up |
| PC7 | GPIO_Input + Pull-up |

---

## Firmware Functionality

The firmware continuously polls the encoder GPIO states and streams the real-time logic levels over UART.

### UART Monitoring Example

```text
A=0 B=0
A=1 B=0
A=1 B=1
```
These transitions confirm successful dual-channel encoder signal acquisition.

### UART Debugging

UART output was monitored through the ST-LINK virtual COM interface using PuTTY at:
````
115200 baud
8 data bits
No parity
1 stop bit
````
### Features Demonstrated
- STM32 GPIO input configuration
- Internal pull-up resistor usage
- External sensor integration
- Quadrature encoder signal monitoring
- UART-based real-time debugging
- Embedded firmware polling architecture

## Images

### Rotary Encoder Hardware Setup
<p align="center">
<img src="images/step2_rotary_encoder_hardware.jpg" width="400"/>
</p>

### UART Encoder Signal Output
<p align="center">
<img src="images/step2_encoder_uart_1Ch_output.png" width="400"/>
</p>

<p align="center">
<img src="images/step2_encoder_uart_2Ch_output.png" width="400"/>
</p>

### STM32CubeIDE GPIO Configuration
<p align="center">
<img src="images/step2_gpio_input_config.png" width="400"/>
</p>

---

# Step 3 — SPI Loopback Packet Communication

## Objective

Validate STM32 SPI peripheral configuration and full-duplex packet communication using an SPI loopback setup.

This step establishes the SPI communication layer required for future MCP2515 SPI-CAN controller integration.

---

## Hardware Used

- STM32 NUCLEO-F401RE
- Dupont jumper wires
- STM32CubeIDE
- PuTTY serial terminal

---

## SPI Configuration

SPI1 was configured in Full-Duplex Master mode.

| SPI Signal | STM32 Pin | NUCLEO Pin |
|---|---|---|
| SPI1_MOSI | PA7 | D11 |
| SPI1_MISO | PA6 | D12 |
| SPI1_SCK | PB3 | - |

---

## Loopback Wiring

A loopback connection was created by connecting:

```text
D11 (MOSI) ↔ D12 (MISO)

```

## Firmware Functionality
The firmware periodically transmits a 4-byte packet through SPI and verifies the received packet through UART debugging.

## SPI Packet Example
```
SPI TX: A5 01 02 03
SPI RX: A5 01 02 03
```
Successful packet matching confirms:

- SPI peripheral configuration
- full-duplex communication
- data transmission integrity
- loopback reception functionality

## Features Demonstrated
- STM32 SPI peripheral configuration
- Full-duplex SPI communication
- Embedded packet transmission/reception
- UART-assisted debugging
- Hardware loopback testing
- Preparation for MCP2515 SPI-CAN controller integration

## Images
### SPI Loopback Hardware Setup
<p align="center">
<img src="images/step3_spi_loopback_hardware.jpg" width="400"/>
</p>

### SPI Loopback Logic analyzer Hardware Setup
<p align="center">
<img src="images/step3_spi_logic-analyzer_hardware1.jpg" width="300"/>
</p>
<p align="center">
<img src="images/step3_spi_logic-analyzer_hardware2.jpg" width="300"/>
</p>

### SPI UART Output
<p align="center">
<img src="images/step3_spi_uart_output.png" width="400"/>
</p>

### SPI Logic Analyzer Output
<p align="center">
<img src="images/step3_spi_logic_analyzer.png" width="600"/>
</p>

### STM32CubeIDE SPI Configuration
<p align="center">
<img src="images/step3_spi_cubeide_config.png" width="400"/>
</p>

# Step 4 — CAN Interface Architecture and Hardware Bring-up

## Objective

Prepare a CAN-ready embedded telemetry architecture using STM32, SN65HVD230 CAN transceiver, and USB-CAN interface hardware.

This step focuses on hardware architecture validation and CAN interface preparation for future MCP2515 SPI-CAN controller integration.

---

## Hardware Used

- STM32 NUCLEO-F401RE
- SN65HVD230 CAN transceiver module
- USB-CAN adapter (CH340-based)
- Dupont jumper wires
- STM32CubeIDE

---

## CAN Architecture
The STM32F401RE MCU does not contain an internal CAN controller peripheral.

To support CAN communication, the system architecture was designed using:

- MCP2515 external SPI-CAN controller
- SN65HVD230 CAN transceiver

## Planned CAN Communication Architecture
```
STM32F401RE
    │ SPI
    ▼
MCP2515 CAN Controller
    │ CAN TX/RX
    ▼
SN65HVD230 CAN Transceiver
    │ CANH/CANL
    ▼
USB-CAN Adapter / Vehicle CAN Bus
```

## Hardware Bring-up
The following hardware interfaces were successfully connected and verified:
| Interface            | Status                |
| -------------------- | --------------------- |
| STM32 ↔ SN65HVD230   | Connected             |
| SN65HVD230 ↔ USB-CAN | Connected             |
| USB-CAN ↔ PC         | Detected successfully |
| CH340 USB Interface  | Enumerated on COM8    |

## Features Demonstrated
- CAN communication architecture design
- Third-party module integration planning
- Embedded hardware bring-up
- USB-CAN interface preparation
- CH340 USB device enumeration
- SPI-to-CAN integration workflow preparation

## Images
### CAN Hardware Setup
<p align="center">
<img src="images/step4_can_hardware_setup1.jpg" width="400"/>
</p>
<p align="center">
<img src="images/step4_can_hardware_setup2.jpg" width="400"/>
</p>

### USB-CAN Detection
<p align="center">
<img src="images/step4_usb_can_detection.png" width="400"/>
</p>

---

# Step 5 — Python Telemetry and Serial Logging Tools

## Objective

Develop a Python-based telemetry logging tool to monitor and record live UART data transmitted from the STM32 embedded platform.

This step establishes the software-side telemetry infrastructure required for:
- GNSS data logging
- CAN telemetry monitoring
- sensor debugging
- embedded system validation

---

## Hardware Used

- STM32 NUCLEO-F401RE
- USB connection to PC

---

## Software Used

- Python 3.12
- pyserial
- STM32CubeIDE
- Windows PowerShell

---

## System Architecture

```text
STM32 UART Telemetry
        │
        ▼
USB Virtual COM Port (COM7)
        │
        ▼
Python Serial Logger
        │
        ▼
Console Monitoring + CSV Logging
```

## Python Serial Logger Features
The Python telemetry logger performs the following tasks:
- Opens STM32 virtual COM port
- Reads UART telemetry data continuously
- Adds PC-side timestamps
- Displays live telemetry in terminal
- Saves telemetry data to CSV file

### Python Dependencies
Installed using:
```
pip install pyserial 
```
### CSV Logging
Telemetry data is automatically saved into:
```
telemetry_log.csv
```
Example CSV structure:
| timestamp_pc        | elapsed_s | uart_line           |
| ------------------- | --------- | ------------------- |
| 2026-05-13T20:15:10 | 0.250     | Telemetry heartbeat |

## Features Demonstrated
- Python serial communication
- Embedded telemetry logging
- UART monitoring tools
- Timestamped data acquisition
- PC-side debug tooling
- Embedded software validation workflow

## Files
| File                            | Description                     |
| ------------------------------- | ------------------------------- |
| `python_tools/serial_logger.py` | Python UART telemetry logger    |
| `telemetry_log.csv`             | Example logged telemetry output |

## Images
### Python Serial Logger Output
<p align="center">
<img src="images/step5_python_serial_logger_output.png" width="400"/>
</p>

## Notes
This Python tooling provides a reusable validation and testing framework for future embedded communication modules.

---

# Step 6 — Memory Management and Telemetry Buffering

## Objective

Implement a lightweight telemetry buffering system using statically allocated memory for deterministic embedded data handling.

This step introduces:
- fixed-size telemetry packets
- ring-buffer style storage
- timestamped telemetry logging
- memory-safe embedded firmware design

---

## Features Implemented

- Static telemetry packet structure
- Fixed-size telemetry buffer
- Circular write indexing
- SPI telemetry packet storage
- UART debug monitoring

---

## Telemetry Packet Structure

```c
typedef struct
{
    uint32_t timestamp_ms;
    uint8_t spi_tx[4];
    uint8_t spi_rx[4];
    uint8_t spi_ok;
} TelemetryPacket_t;
```
## Example UART Output
```
SPI TX: A5 01 02 03
SPI RX: A5 01 02 03
Telemetry buffered: index=1 count=16 spi_ok=1 timestamp=54471
```

## Features Demonstrated
- Embedded memory management
- Static allocation techniques
- Circular telemetry buffering
- Deterministic firmware behavior
- Embedded diagnostics and monitoring

## Images
### Telemetry Buffer Debug Output
<p align="center">
<img src="images/step6_telemetry_buffer_output.png" width="400"/>
</p>

## Notes
The telemetry buffering framework will later be reused for:

- GNSS telemetry packets
- CAN message buffering
- sensor data logging
- embedded communication diagnostics


# Step 7a — Bootloader and Firmware Update Workflow

## Objective

Document a future-ready STM32 firmware deployment and bootloader architecture for telemetry applications.

## Topics Covered

- STM32 bootloader concepts
- firmware deployment workflow
- flash memory organization
- future OTA update architecture
- firmware safety considerations

## Files

| File | Description |
|---|---|
| `docs/bootloader_update_workflow.md` | Firmware update and bootloader architecture documentation |

## Features Demonstrated

- Embedded firmware architecture understanding
- Bootloader workflow concepts
- OTA update planning
- Flash memory management awareness

---

# Step 7b — UART Bootloader Workflow Demo

## Objective

Implement a lightweight UART-based bootloader workflow demonstrating:
- UART command reception
- bootloader state transitions
- firmware update command handling
- fixed-size firmware chunk reception
- static RAM buffering

This step focuses on understanding embedded firmware update architecture without modifying internal Flash memory.

---

## Features Implemented

- UART bootloader command parser
- `UPDATE` command detection
- bootloader update mode state machine
- fixed-size firmware chunk reception
- static firmware RAM buffer
- firmware chunk counter and monitoring

---

## Bootloader Workflow

```text
PC Terminal
    ↓
UART Command Reception
    ↓
UPDATE Command Detection
    ↓
Bootloader Update Mode
    ↓
Firmware Chunk Reception
    ↓
Static RAM Buffer Storage
```
## Example Bootloader Interaction
```
BOOT> Ready
BOOT> Send UPDATE to enter firmware update mode

BOOT> Type command: update
BOOT> Received: update
BOOT> Update mode entered
BOOT> Ready to receive firmware chunks

BOOT> Send 8-byte firmware chunk
ABCDEFGH

BOOT> Chunk 0 received: 41 42 43 44 45 46 47 48
BOOT> Total chunks stored: 1/8
```

## Features Demonstrated
- UART communication
- Embedded command parsing
- Bootloader state-machine design
- Static memory allocation
- Firmware chunk buffering
- Embedded update workflow concepts

## Notes
This implementation is a lightweight bootloader demonstration.
The current version:

- stores firmware chunks in RAM
- does not erase or program Flash memory
- does not implement full OTA update logic

Future extensions may include:

- Flash programming
- CAN/UART firmware updates
- firmware validation
- OTA update support

## Images
### Bootloader Update Mode
<p align="center">
<img src="images/step7_bootloader_update_mode.png" width="400"/>
</p>

### Firmware Chunk Reception
<p align="center">
<img src="images/step7_bootloader_firmware_chunk_receive2.png" width="400"/>
</p>

## Tool Used
- STM32CubeIDE
- STM32 HAL
- PuTTY serial terminal
- ST-LINK debugger


## Author
**Vasan Iyer**   
Embedded Software Engineer

Focus areas:
- Embedded C
- STM32
- Interfaces: GPIO, UART, CAN, SPI

GitHub: https://github.com/Vaiy108