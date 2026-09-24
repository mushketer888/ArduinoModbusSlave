# Arduino Modbus Slave

A Modbus RTU slave implementation for the Arduino Uno, built with PlatformIO.

## Overview

This project turns an Arduino Uno into a Modbus RTU slave device. It uses a
SoftwareSerial-based RS485 connection (with an HW-726 auto-direction module) to
communicate with a Modbus master. Holding register `40001` controls the
on-board LED, and registers `40002`-`40010` are general-purpose read/write
registers available for the master to use.

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
| Holding regs | 10 (registers 40001–40010) |
| Random reg   | `RANDOM_REGISTER` (see below) |

The holding register array is linked directly to the Modbus slave object via
`modbus.configureHoldingRegisters()`, so writes from the master update the
array automatically in the background.

## How it works

- `src/main.cpp:43` — `modbus.poll()` is called every loop iteration to listen
  for master commands.
- `src/main.cpp:64` — the **random register** (`RANDOM_REGISTER`) is refilled with a
  fresh `uint16` on every loop iteration when enabled. Setting
  `RANDOM_REGISTER = -1` disables this and the register behaves like a normal
  read/write register.
- `src/main.cpp:47` — `holdingRegisters[0]` (Modbus register 40001) is read and
  used to drive `LED_BUILTIN` high or low.
- `src/main.cpp:16` — `holdingRegisters` is a 10-element `uint16_t` array backed
  by all 10 holding registers (40001–40010). Registers 40002–40010 are
  general-purpose and can be read or written by the master without any FPGA-side
  logic; they simply mirror the array values.

## Random register

One holding register can be filled automatically with a random `uint16` value.
This is useful for testing the Modbus link, generating synthetic data, or
providing a changing value to a master that polls the slave.

It is configured with the `RANDOM_REGISTER` constant in `src/main.cpp`:

| `RANDOM_REGISTER` | Behavior |
|-------------------|----------|
| `-1` | Disabled. The register behaves like any other read/write register and only changes when the master writes to it. Default. |
| `>= 0` | Index of the holding register that the slave refills with `random(0, 65536)` on every `loop()` iteration. |

```cpp
const int16_t RANDOM_REGISTER = -1;   // disabled by default; set to 0..9 to enable
```

The PRNG is seeded from analog noise on `A0` in `setup()`, so the value is
different on every power cycle and changes on every read. A `static_assert`
catches an out-of-range index at compile time.

Note: `RANDOM_REGISTER` must not collide with index `0` (the LED register),
otherwise the random value will keep overwriting the LED control.

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

## Register map

| Modbus Address | Index | Description                            |
|----------------|-------|----------------------------------------|
| 40001          | 0     | LED control (`1` = on, `0` = off)      |
| 40002          | 1     | General-purpose read/write register (random if `RANDOM_REGISTER=1`) |
| 40003          | 2     | General-purpose read/write register    |
| 40004          | 3     | General-purpose read/write register    |
| 40005          | 4     | General-purpose read/write register    |
| 40006          | 5     | General-purpose read/write register    |
| 40007          | 6     | General-purpose read/write register    |
| 40008          | 7     | General-purpose read/write register    |
| 40009          | 8     | General-purpose read/write register    |
| 40010          | 9     | General-purpose read/write register    |

All registers are `uint16_t` (16-bit holding registers). Registers 40002–40010
are writable and readable by the master and initialized to `0` on startup.

## Usage

From a Modbus master, write to holding register `40001`:

- `1` (or any value > 0) → LED on
- `0` → LED off

Use registers `40002`–`40010` as needed for your application data (e.g., sensor
values, flags, counters). The master can read any or all registers using
standard FC03 (Read Holding Registers) and write them using FC06 (Write Single
Holding Register) or FC16 (Write Multiple Holding Registers).

---

# Arduino Modbus Slave (на русском)

Реализация ведомого устройства (slave) Modbus RTU для Arduino Uno, собранная с
помощью PlatformIO.

## Обзор

Этот проект превращает Arduino Uno в ведомое устройство Modbus RTU. Для связи с
ведущим устройством (master) используется подключение RS485 на базе SoftwareSerial
(с модулем авто-направления HW-726). Управление встроенным светодиодом
осуществляется через holding-регистр `40001`, а регистры `40002`–`40010`
представляют собой универсальные регистры для чтения/записи, доступные ведущему
устройству.

## Аппаратное обеспечение

- **Плата:** Arduino Uno
- **RS485 модуль:** HW-726 (автоматическое управление направлением, пины DE/RE
  не требуются)
- **Подключение RS485 (SoftwareSerial):**
  - RX → вывод 2
  - TX → вывод 3
- **Светодиод:** встроенный `LED_BUILTIN`

## Конфигурация

| Параметр        | Значение           |
|-----------------|--------------------|
| Slave ID        | 1                  |
| Скорость (baud) | 9600               |
| Режим UART      | 8 бит данных, без чётности, 1 стоп-бит (`SERIAL_8N1`) |
| Holding-регистры | 10 (регистры 40001–40010) |

Массив holding-регистров связывается напрямую с объектом ведомого устройства
Modbus через `modbus.configureHoldingRegisters()`, поэтому записи от ведущего
устройства обновляют массив автоматически в фоновом режиме.

## Как это работает

- `src/main.cpp:43` — `modbus.poll()` вызывается на каждой итерации цикла для
  прослушивания команд ведущего устройства.
- `src/main.cpp:47` — считывается `holdingRegisters[0]` (регистр Modbus 40001) и
  используется для установки `LED_BUILTIN` в высокое или низкое состояние.
- `src/main.cpp:16` — `holdingRegisters` — это массив из 10 элементов `uint16_t`,
  соответствующий всем 10 holding-регистрам (40001–40010). Регистры 40002–40010
являются универсальными и могут быть прочитаны или записаны ведущим устройством
без какой-либо логики на стороне контроллера; они просто отражают значения
массива.

## Random-регистр

Один holding-регистр может автоматически заполняться случайным значением
`uint16`. Это удобно для тестирования Modbus-связи, синтетических данных или
предоставления меняющегося значения ведущему устройству, которое опрашивает
slave.

Настройка производится константой `RANDOM_REGISTER` в `src/main.cpp`:

| `RANDOM_REGISTER` | Поведение |
|-------------------|-----------|
| `-1` | Отключено. Регистр ведёт себя как обычный, меняется только при записи ведущего. По умолчанию. |
| `>= 0` | Индекс holding-регистра, который slave перезаписывает случайным `random(0, 65536)` на каждой итерации `loop()`. |

```cpp
const int16_t RANDOM_REGISTER = -1;   // отключено по умолчанию; 0..9 включает random
```

PRNG инициализируется аналоговым шумом на `A0` в `setup()`, поэтому значение
отличается при каждом включении и меняется при каждом чтении. `static_assert`
проверяет, что индекс не выходит за границы массива, на этапе компиляции.

Примечание: `RANDOM_REGISTER` не должен совпадать с индексом `0`
(светодиод), иначе случайное значение будет перетирать управление светодиодом.

## Сборка и загрузка

В этом проекте используется [PlatformIO](https://platformio.org/).

```bash
# Сборка
pio run

# Загрузка на плату
pio run --target upload

# Мониторинг последовательного вывода (отладка, 9600 бод)
pio device monitor -b 9600
```

Зависимости (`platformio.ini`): `ModbusRTUSlave`, `ModbusADU`,
`ModbusSlaveLogic`, `ModbusRTUComm`.

## Карта регистров

| Адрес Modbus | Индекс | Описание                                 |
|--------------|--------|------------------------------------------|
| 40001        | 0      | Управление светодиодом (`1` = вкл, `0` = выкл) |
| 40002        | 1      | Обычный регистр чтения/записи (random, если `RANDOM_REGISTER=1`) |
| 40003        | 2      | Универсальный регистр чтения/записи      |
| 40004        | 3      | Универсальный регистр чтения/записи      |
| 40005        | 4      | Универсальный регистр чтения/записи      |
| 40006        | 5      | Универсальный регистр чтения/записи      |
| 40007        | 6      | Универсальный регистр чтения/записи      |
| 40008        | 7      | Универсальный регистр чтения/записи      |
| 40009        | 8      | Универсальный регистр чтения/записи      |
| 40010        | 9      | Универсальный регистр чтения/записи      |

Все регистры имеют тип `uint16_t` (16-битные holding-регистры). Регистры
40002–40010 доступны для записи и чтения ведущим устройством и
инициализируются значением `0` при запуске.

## Использование

Из ведущего устройства Modbus выполните запись в holding-регистр `40001`:

- `1` (или любое значение > 0) → светодиод вкл
- `0` → светодиод выкл

Используйте регистры `40002`–`40010` по необходимости для данных вашего
приложения (например, значения датчиков, флаги, счётчики). Ведущее устройство
может прочитать любой или все регистры с помощью стандартной функции FC03 (Read
Holding Registers) и записать их с помощью FC06 (Write Single Holding Register)
или FC16 (Write Multiple Holding Registers).
