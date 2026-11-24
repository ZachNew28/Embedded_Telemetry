# STM32 Embedded Telemetry System

**Version 1.0 — Bare-Metal Sensor Telemetry**

A real-time embedded telemetry application built with C on the STM32F446RE microcontroller. The system acquires 6-axis inertial measurements from an MPU-6050 over I²C and streams structured sensor data to a host computer over UART.

The project demonstrates embedded firmware development, peripheral configuration, modular driver design, serial communication, and timing verification.

## Hardware

| Component | Purpose |
|---|---|
| STM32 NUCLEO-F446RE | Microcontroller development board |
| Adafruit MPU-6050 | 3-axis accelerometer and 3-axis gyroscope |
| STEMMA QT to jumper cable | Sensor connection |
| USB cable | Board power, programming, debugging and serial communication |

### Wiring

| MPU-6050 connection | STM32 connection |
|---|---|
| VCC (red) | 3.3 V |
| GND (black) | GND |
| SDA (blue) | PB9 / Arduino D14 |
| SCL (yellow) | PB8 / Arduino D15 |

The MPU-6050 communicates over I²C1 at 100 kHz using 7-bit address `0x68`.

## Firmware Architecture

| File | Responsibility |
|---|---|
| `Core/Src/main.c` | Application initialization, timing, UART telemetry, statistics and LED heartbeat |
| `Core/Inc/mpu6050.h` | Public MPU-6050 driver API and sample structure |
| `Core/Src/mpu6050.c` | Sensor identification, configuration, I²C reads and unit conversion |

The driver exposes `MPU6050_Init()` and `MPU6050_Read()` for application-level sensor access.

## Sensor Configuration

| Parameter | Configuration |
|---|---|
| Acquisition rate | 100 Hz |
| UART telemetry rate | 20 Hz |
| Accelerometer range | ±2 g |
| Gyroscope range | ±250°/s |
| Accelerometer sensitivity | 16,384 LSB/g |
| Gyroscope sensitivity | 131 LSB/(°/s) |
| UART configuration | USART2, 115200 baud, 8N1 |

## Telemetry Format

Sensor readings are transmitted as comma-separated values with the following field order:

`sample_count,accel_x_g,accel_y_g,accel_z_g,gyro_x_dps,gyro_y_dps,gyro_z_dps`

An example telemetry message:

`125,0.012,-0.018,0.997,0.24,-0.12,0.08`

Every 10 seconds, firmware also reports measured acquisition and telemetry rates, along with I²C communication errors.

Statistics messages begin with `# STATS:` so they can be distinguished from sensor data.

## Building and Running

The project was developed using STM32CubeMX for peripheral configuration and STM32CubeIDE with the STM32 HAL libraries.

Open the STM32CubeIDE project, build the firmware, and program the board using the integrated ST-LINK debugger.

Connect a serial terminal such as Tera Term to the ST-LINK Virtual COM Port with 115200 baud, 8 data bits, no parity, 1 stop bit, and no flow control.

The board starts collecting sensor measurements automatically after startup.

## Verification

The application has been tested on the NUCLEO-F446RE with a physically connected MPU-6050.

Sensor readings were verified by changing the board's orientation and observing the corresponding accelerometer and gyroscope measurements.

The firmware includes runtime statistics to evaluate whether acquisition and UART transmissions meet their configured rates.

**Measured test results:** `# STATS: IMU=100.0 Hz, UART=20.0 Hz, I2C errors=0`, `# STATS: IMU=100.1 Hz, UART=20.0 Hz, I2C errors=0`

## Current Limitations

Version 1 uses a cooperative, millisecond-based polling scheduler and blocking HAL I²C/UART operations. Average operation rates are measured, but worst-case scheduling latency and jitter have not yet been characterized.

Sensor calibration, DMA-based communication, watchdog recovery and automated hardware-in-the-loop testing are not yet implemented.

## Planned Development

**Version 2:** BME280 environmental sensing, FreeRTOS, C++ driver abstractions, interrupt-driven acquisition, binary telemetry and automated verification.

**Version 3:** Watchdog and fault recovery, hardware-in-the-loop testing, CI, Ethernet-based UDP telemetry and performance profiling.

## References

- [ST NUCLEO-F446RE product documentation](https://www.st.com/en/evaluation-tools/nucleo-f446re.html)
- [STM32 Nucleo-64 user manual (UM1724)](https://www.st.com/resource/en/user_manual/dm00105823.pdf)

