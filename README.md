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

## Tool Used
- STM32CubeIDE
- STM32 HAL
- PuTTY serial terminal
- ST-LINK debugger
