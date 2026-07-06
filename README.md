# Arduino Modbus Slave

A Modbus RTU slave implementation for the Arduino Uno, built with PlatformIO.

## Overview

This project turns an Arduino Uno into a Modbus RTU slave device. It uses a
SoftwareSerial-based RS485 connection (with an HW-726 auto-direction module) to
communicate with a Modbus master. A single holding register controls the
on-board LED: writing a non-zero value turns the LED on, writing zero turns it
off.

## Hardware

- **Board:** Arduino Uno
- **RS485 module:** HW-726 (auto direction control, no DE/RE pins needed)
- **RS485 wiring (SoftwareSerial):**
  - RX → pin 2
  - TX → pin 3
- **LED:** on-board `LED_BUILTIN`

## Configuration

| Setting      | Value              |
|--------------|--------------------|
| Slave ID     | 1                  |
| Baud rate    | 9600               |
| Serial mode  | 8 data bits, No parity, 1 stop bit (`SERIAL_8N1`) |
| Holding regs | 1 (register 40001) |

The holding register array is linked directly to the Modbus slave object via
`modbus.configureHoldingRegisters()`, so writes from the master update the
array automatically in the background.

## How it works

- `src/main.cpp:43` — `modbus.poll()` is called every loop iteration to listen
  for master commands.
- `src/main.cpp:47` — `holdingRegisters[0]` (Modbus register 40001) is read and
  used to drive `LED_BUILTIN` high or low.

## Building and uploading

This project uses [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload to the board
pio run --target upload

# Monitor the serial output (debug, 9600 baud)
pio device monitor -b 9600
```

Dependencies (`platformio.ini`): `ModbusRTUSlave`, `ModbusADU`,
`ModbusSlaveLogic`, `ModbusRTUComm`.

## Usage

From a Modbus master, write to holding register `40001`:

- `1` (or any value > 0) → LED on
- `0` → LED off
