# GPS-GSM-Tracker
<img src="https://github.com/curcuman/GPS-GSM-Tracker/assets/93979835/13118b91-a07c-41dd-8add-ee403fea27e9" alt="drawing" width="500"/>


# Connections resume
For what regards the STM32 pins:

![image](https://github.com/curcuman/GPS-GSM-Tracker/assets/93979835/4651a463-d4fb-49e6-8654-f92fa3b303e1)

STM32F1    |  A9G
--- | ---
Uart1_RX(PA10)(A10)  |  TX
Uart1_TX(PA9)(A9)    |  RX
GND                  |  GND


Power from usb or battery: PWR_KEY to GND

UART debugging info
STM32F1  | NUCLEO (as way to the usb port)
--- | ---
A2        |                   RX
A3        |                   TX


# A9G Communication Firmware

---

This repository contains firmware for communication between the STM32F1 microcontroller and the A9G module. Below is a brief overview of the code structure and functionality.

## Overview

- **Author:** Filippo Zeggio
- **License:** See [LICENSE](LICENSE) file for details.

## Code Structure

- `main.c`: Main program body containing initialization and main loop.
- `dma.h`: Header file for Direct Memory Access configuration.
- `usart.h`: Header file for USART (UART) configuration.
- `gpio.h`: Header file for GPIO configuration.

## Functionality

1. **Initialization:** Initializes peripherals including GPIO, DMA, and USART for communication.
2. **Main Loop:**
   - Waits for A9G initialization.
   - Handles communication with A9G module.
   - Retrieves position and sends messages.

## Key Functions

- `quickBlink()`: Blinks LED for visual indication.
- `found()`, `foundCREG()`, `foundRING()`, `foundMESSAGE()`: Checks for specific responses from A9G.
- `get_position_send_message()`: Retrieves position and sends message.
- `HAL_UART_RxCpltCallback()`: UART receive callback function.

## How to Use

1. Ensure all necessary connections are made between STM32F1 and A9G.
2. Flash the firmware onto the STM32F1 microcontroller.
3. Monitor UART communication to interact with the A9G module.

## Notes

- Adjust timeout and delay values as needed for your specific application.
- Ensure proper power supply and signal integrity for reliable communication.

