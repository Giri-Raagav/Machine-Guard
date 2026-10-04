# Machine Guard

## Multi-Sensor Edge-Based Machine Condition Monitoring and Fault Detection System

Machine Guard is an ESP32-C3 based embedded system designed to monitor the operating condition of a motor using multiple physical parameters.

Instead of depending on a single sensor, the system combines vibration, temperature, electrical current, and voltage measurements to identify abnormal operating conditions in real time.

The system provides local diagnostic information through both the Serial Monitor and a 16×2 I²C LCD.

---

##  Project Objective

The main objective of Machine Guard is to develop a low-cost embedded machine-condition monitoring system capable of detecting abnormal motor operating conditions.

The system continuously observes multiple parameters:

-  Motor current and electrical power
-  Bus voltage
-  Motor/system temperature
-  Mechanical vibration
-  Real-time machine status
-  Abnormal operating conditions

The measurements are processed directly on the ESP32-C3, allowing the system to make condition decisions at the edge without requiring a computer for normal operation.

---

##  System Architecture

The overall system can be represented as:

    ┌─────────────────────┐
    │    Machine / Motor  │
    └──────────┬──────────┘
               │
       ┌───────┼────────┐
       │       │        │
       ▼       ▼        ▼
    Vibration Temperature Current
     MPU6050   DS18B20   INA219
       │       │        │
       └───────┼────────┘
               │
               ▼
       ┌─────────────────┐
       │    ESP32-C3     │
       │   Edge Monitor  │
       └────────┬────────┘
                │
        ┌───────┼────────┐
        │       │        │
        ▼       ▼        ▼
     Serial    16×2    Fault
     Monitor    LCD     Logic
                         │
                         ▼
                  Machine Status

---

##  Hardware Components

| Component | Purpose |
|---|---|
| ESP32-C3 | Main processing and edge-monitoring controller |
| MPU6050 | Vibration / acceleration measurement |
| DS18B20 | Temperature measurement |
| INA219 | Voltage, current and power measurement |
| 16×2 I²C LCD | Local status and parameter display |
| DC Motor | Machine under monitoring |
| Motor driver / power stage | Motor control |
| Breadboard and jumper wires | Prototyping and connections |

---

#  Sensor Monitoring

Machine Guard combines three primary physical measurements.

---

## 1.  Vibration Monitoring — MPU6050

The MPU6050 measures acceleration along the X, Y and Z axes.

The acceleration magnitude is calculated from the three axes:

    Magnitude = √(X² + Y² + Z²)

The vibration value used by the system is based on the deviation of the measured acceleration magnitude from approximately 1 g:

    Vibration = |Magnitude - 1|

The system compares the calculated vibration value against a configured vibration threshold.

### Vibration Threshold

    VIBRATION_THRESHOLD = 0.400 g

If the vibration exceeds the configured threshold, the system identifies the condition as a high-vibration condition.

### Example

    Vibration below threshold
            ↓
        NORMAL

    Vibration above threshold
            ↓
     HIGH VIBRATION

This allows mechanical abnormalities to be detected through changes in acceleration.

---

## 2.  Temperature Monitoring — DS18B20

The DS18B20 is used to continuously measure the temperature associated with the monitored system.

The temperature measurement is displayed in degrees Celsius.

Example:

    Temperature: 31.7 °C

The measured temperature is compared against the configured temperature limit.

The resulting condition can be classified as:

    NORMAL TEMPERATURE

or

    HIGH TEMPERATURE

The temperature condition is also included in the overall machine-status decision.

---

## 3.  Electrical Monitoring — INA219

The INA219 provides real-time electrical measurements from the monitored motor circuit.

The system obtains:

- Bus voltage
- Current
- Power

Example Serial Monitor output:

    INA219 | Voltage: 4.104 V | Current: 135.3 mA | Power: 1354.0 mW

This allows Machine Guard to monitor both the electrical supply condition and the current consumed by the motor.

---

#  INA219 Current Diagnostic System

One of the important features of Machine Guard is the INA219-based current diagnostic system.

The measured motor current is classified into three operating conditions.

### Current Classification

| Measured Current | Classification |
|---:|---|
| Current < 50 mA | LOW CURRENT |
| 50 mA ≤ Current ≤ 300 mA | NORMAL CURRENT |
| Current > 300 mA | HIGH CURRENT |

The classification is performed directly by the ESP32-C3.

### Low Current

When the measured current is below the lower threshold:

    Current < 50 mA
          ↓
      LOW CURRENT

The system reports a low-current condition.

### Normal Current

When the measured current remains within the expected operating range:

    50 mA ≤ Current ≤ 300 mA
              ↓
        NORMAL CURRENT

The system reports normal operation.

### High Current

When the measured current exceeds the upper threshold:

    Current > 300 mA
          ↓
      HIGH CURRENT

The system reports a high-current condition.

---

# 🧪 Controlled Current Fault Simulation

Machine Guard includes a controlled fault-simulation mode for testing the current-detection logic.

This allows abnormal current conditions to be deliberately simulated without depending on an actual electrical fault in the motor.

The system can be tested for:

    LOW CURRENT
    NORMAL CURRENT
    HIGH CURRENT

This makes it possible to verify that the INA219 measurement and fault-classification logic are operating correctly.

The purpose of this mode is testing and validation of the monitoring system.

---

#  Multi-Sensor Fault Detection

Machine Guard does not depend on only one sensor.

The ESP32-C3 combines the available measurements and evaluates the operating condition of the machine.

The monitored conditions include:

    ┌────────────────────┐
    │     Vibration      │
    └─────────┬──────────┘
              │
    ┌─────────▼──────────┐
    │    Temperature     │
    └─────────┬──────────┘
              │
    ┌─────────▼──────────┐
    │      Current       │
    └─────────┬──────────┘
              │
    ┌─────────▼──────────┐
    │       Voltage      │
    └─────────┬──────────┘
              │
              ▼
       ┌───────────────┐
       │   ESP32-C3    │
       │ Fault Logic   │
       └───────┬───────┘
               │
               ▼
        Machine Status

The final status can indicate normal operation or an abnormal condition such as:

- LOW CURRENT
- HIGH CURRENT
- HIGH TEMPERATURE
- HIGH VIBRATION

---

# 🖥️ Display System

Machine Guard provides two forms of real-time feedback.

## Serial Monitor

The Serial Monitor provides detailed numerical measurements and diagnostic information.

Example:

    INA219 | Voltage: 4.104 V | Current: 135.3 mA | Power: 1354.0 mW

    Vibration: 0.023 g
    Temperature: 31.7 °C

    STATUS: NORMAL

The units are explicitly displayed so that the measured values can be interpreted directly.

### Measurement Units

| Parameter | Unit |
|---|---|
| Voltage | V |
| Current | mA |
| Power | mW |
| Temperature | °C |
| Vibration | g |

---

## 16×2 I²C LCD

The LCD provides a compact local display of the machine condition.

Typical information displayed includes:

    V:0.029 T:31.90
    LOW I:47 mA

or:

    V:0.029 T:31.90
    HIGH I:425 mA

or during normal operation:

    V:0.02 T:31.7
    NORMAL I:84 mA

The LCD allows the machine condition to be observed without opening the Serial Monitor.

---

#  I²C Communication

The ESP32-C3 uses I²C communication for the sensor and display devices.

The project uses:

- MPU6050
- INA219
- I²C LCD

The MPU6050 is confirmed at I²C address:

    0x68

The INA219 is used at:

    0x40

The I²C bus uses the configured ESP32-C3 SDA and SCL pins.

---

#  System Operating Flow

The basic operating sequence is:

    START
      │
      ▼
    Initialize ESP32-C3
      │
      ▼
    Initialize I²C
      │
      ▼
    Initialize MPU6050
      │
      ▼
    Initialize INA219
      │
      ▼
    Initialize DS18B20
      │
      ▼
    Initialize LCD
      │
      ▼
    Read Sensor Data
      │
      ├───────────────┐
      │               │
      ▼               ▼
    Vibration      Temperature
      │               │
      └───────┬───────┘
              │
              ▼
        Read INA219
              │
       ┌──────┼──────┐
       ▼      ▼      ▼
    Voltage Current Power
              │
              ▼
        Fault Evaluation
              │
              ▼
       Machine Classification
              │
       ┌──────┴──────┐
       ▼             ▼
    Serial          LCD
    Monitor        Display
              │
              ▼
        Repeat Continuously

---

#  Fault Conditions

Machine Guard is designed to identify abnormal operating conditions using sensor thresholds.

### Current Faults

    Current < 50 mA
        → LOW CURRENT

    Current > 300 mA
        → HIGH CURRENT

### Vibration Fault

    Vibration > 0.400 g
        → HIGH VIBRATION

### Temperature Fault

    Temperature exceeds the configured temperature limit
        → HIGH TEMPERATURE

The thresholds can be adjusted according to the characteristics of the motor and the intended application.

---

#  Testing and Validation

The system was tested by deliberately creating different operating conditions.

## Normal Condition

The motor operates within the expected sensor ranges.

Expected result:

    STATUS: NORMAL

---

## Low Current Test

A controlled low-current condition is introduced.

Expected result:

    LOW CURRENT

The LCD and Serial Monitor should identify the low-current condition.

---

## High Current Test

A controlled high-current condition is introduced.

Expected result:

    HIGH CURRENT

The LCD and Serial Monitor should identify the high-current condition.

---

## High Vibration Test

The vibration condition is intentionally increased beyond the configured threshold.

Expected result:

    HIGH VIBRATION

---

## High Temperature Test

The temperature is increased beyond the configured limit.

Expected result:

    HIGH TEMPERATURE

---


#  Project Features

Machine Guard currently provides:

- Multi-sensor machine monitoring
- ESP32-C3 edge processing
- MPU6050 vibration measurement
- DS18B20 temperature measurement
- INA219 voltage measurement
- INA219 current measurement
- INA219 power measurement
- Current fault classification
- Controlled low/high current simulation
- LCD-based local monitoring
- Serial diagnostic output
- Configurable fault thresholds
- Real-time machine-condition classification

---

#  Software

The firmware is developed using the Arduino environment for the ESP32-C3.

The project uses sensor libraries for:

- MPU6050
- INA219
- DS18B20
- I²C LCD

The firmware continuously acquires sensor data, evaluates the configured thresholds and updates the machine status.

---

#  Project Structure

A suggested repository structure is:

    Machine-Guard/
    │
    ├── README.md
    │
    ├── src/
    │   └── Machine_Guard.ino
    │
    ├── images/
    │   ├── system.jpg
    │   ├── normal.jpg
    │   ├── low_current.jpg
    │   └── high_current.jpg
    │
    └── documentation/
        └── project_notes.md

---

#  Future Improvements

Possible future development includes:

- Machine-learning based fault classification
- TinyML deployment on the ESP32-C3
- More advanced sensor-fusion algorithms
- Data logging
- Historical fault analysis
- Wireless monitoring
- Web-based dashboard
- Mobile monitoring
- Automatic maintenance alerts
- Improved motor protection logic

These features can be added without changing the fundamental multi-sensor monitoring architecture.

---

#  Project Significance

Machine Guard demonstrates how an embedded controller can combine multiple physical measurements to monitor the health of an electromechanical system.

The project integrates concepts from:

- Embedded systems
- Microcontrollers
- Sensors and instrumentation
- I²C communication
- Motor monitoring
- Electrical measurement
- Signal processing
- Fault detection
- Edge computing
- Machine condition monitoring

The use of multiple sensors makes the system more informative than a single-parameter monitoring solution.

---

#  Conclusion

Machine Guard is a multi-sensor edge-based machine condition monitoring system built around the ESP32-C3.

By combining vibration, temperature, current, voltage and power measurements, the system can identify abnormal operating conditions and present the results in real time.

The INA219 current diagnostic subsystem additionally provides controlled low-current and high-current fault simulation, allowing the fault-detection logic to be tested in a repeatable manner.

The current implementation establishes the foundation for a more advanced machine-health monitoring and predictive-maintenance system.

---

##  Project

**Project Name:** Machine Guard

**Platform:** ESP32-C3

**System Type:** Multi-Sensor Edge-Based Machine Condition Monitoring and Fault Detection

**Primary Sensors:** MPU6050, DS18B20, INA219

**Display:** 16×2 I²C LCD

**Development Environment:** Arduino IDE
