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
- [ ] Step 2 — Rotary encoder integration
- [ ] Step 3 — CAN telemetry transmission
- [ ] Step 4 — Python CAN monitoring tools
- [ ] Step 5 — SPI communication demo
- [ ] Step 6 — GNSS integration
- [ ] Step 7 — Bootloader and firmware update workflow

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

## Tool Used
- STM32CubeIDE
- STM32 HAL
- PuTTY serial terminal
- ST-LINK debugger
