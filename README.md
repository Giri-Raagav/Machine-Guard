# Machine-Guard
Machine Guard is an ESP32-C3-based machine health and safety monitoring system that uses multi-sensor data from the MPU6050, DS18B20, and INA219 to detect abnormal vibration, high temperature, and abnormal motor current conditions, with real-time LCD/Serial monitoring and automatic motor safety shutdown.
# Machine Guard

### Real-Time Edge Monitoring and Safety System for Machine Health

Machine Guard is an ESP32-C3-based machine health and safety monitoring system designed to monitor important operating parameters of a motor in real time.

The system combines vibration, temperature, voltage, current, and power measurements to identify abnormal operating conditions and provide an early warning of potential machine faults.

## Features

- Real-time vibration monitoring using MPU6050
- Temperature monitoring using DS18B20
- Voltage, current, and power monitoring using INA219
- 16x2 I2C LCD status display
- Motor forward and reverse control
- Low-current detection
- Normal-current detection
- High-current detection
- High-vibration detection
- High-temperature detection
- Automatic motor shutdown during critical conditions
- Serial Monitor diagnostic output
- Manual test modes for current-fault demonstration

## Hardware Used

- ESP32-C3
- MPU6050
- INA219
- DS18B20
- 16x2 I2C LCD
- DC Motor
- Motor Driver
- Power Supply

## Sensor Connections

| Device | ESP32-C3 |
|---|---|
| I2C SDA | GPIO 6 |
| I2C SCL | GPIO 7 |
| DS18B20 | GPIO 4 |
| Motor PWM | GPIO 3 |
| Motor AIN1 | GPIO 5 |
| Motor AIN2 | GPIO 8 |
| Motor STBY | GPIO 10 |

## I2C Addresses

| Device | Address |
|---|---|
| MPU6050 | 0x68 |
| INA219 | 0x40 |
| LCD | 0x27 |

## Current Monitoring

Machine Guard uses the INA219 to continuously monitor motor current.

Current thresholds:

- Below 50 mA → LOW CURRENT
- 50–300 mA → NORMAL CURRENT
- Above 300 mA → HIGH CURRENT

The thresholds can be adjusted in the source code.

## Test Commands

Commands can be entered through the Serial Monitor.

| Command | Function |
|---|---|
| `N` | Normal live monitoring |
| `H` | High-current test mode |
| `L` | Low-current test mode |
| `S` | Stop motor |

The H and L modes are demonstration/test modes that allow the current-fault detection logic to be tested deliberately.

## Safety Monitoring

Machine Guard continuously checks:

- Vibration
- Temperature
- Motor current

If a critical vibration or temperature condition is detected, the motor is automatically stopped.

## Output

The system provides information through:

### Serial Monitor

Displays:

- X, Y, Z acceleration
- Vibration
- Temperature
- INA219 voltage
- INA219 current
- INA219 power
- Current diagnosis
- System status

### LCD

Displays the current machine condition locally.

Example:

```text
LOW I:7mA
