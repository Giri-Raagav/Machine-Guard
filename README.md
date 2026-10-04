# 🛡️ Machine Guard

## Multi-Sensor Edge-Based Machine Condition Monitoring and Fault Detection System

Machine Guard is an ESP32-C3 based machine-condition monitoring system designed to detect abnormal operating conditions using multiple sensors.

The system continuously monitors:

- ⚡ Motor current and electrical power
- 🔋 Bus voltage
- 🌡️ Motor/system temperature
- 📳 Mechanical vibration
- 🖥️ Real-time machine status
- 🛑 Safety conditions requiring motor shutdown

The project combines sensor measurements at the edge and provides real-time diagnostic information through the Serial Monitor and a 16×2 I²C LCD.

The system also includes a controlled **current fault simulation mode**, allowing low-current and high-current conditions to be deliberately tested without requiring an actual electrical fault.

---

# 🎯 Project Objective

The main objective of Machine Guard is to develop a low-cost embedded machine-health monitoring system capable of identifying abnormal machine operating conditions.

Instead of monitoring only one parameter, Machine Guard combines multiple physical parameters:

```text
             ┌─────────────────┐
             │     Machine     │
             │      Motor      │
             └────────┬────────┘
                      │
       ┌──────────────┼──────────────┐
       │              │              │
       ▼              ▼              ▼
   Vibration      Temperature      Current
    MPU6050         DS18B20        INA219
       │              │              │
       └──────────────┼──────────────┘
                      ▼
              ┌───────────────┐
              │   ESP32-C3    │
              │ Edge Monitor  │
              └───────┬───────┘
                      │
          ┌───────────┼───────────┐
          ▼           ▼           ▼
       Serial       LCD       Fault Logic
       Monitor     Display         │
                                   ▼
                              Motor Safety
                               Shutdown

System Concept

Machine Guard monitors the machine using three primary physical measurements:

1. Vibration

The MPU6050 measures acceleration along the X, Y and Z axes.

The acceleration magnitude is calculated as:

Magnitude = √(X² + Y² + Z²)

The vibration value used by the system is:

Vibration = |Magnitude - 1|

A vibration threshold is then used to determine whether an abnormal vibration condition exists.

Current threshold:

VIBRATION_THRESHOLD = 0.400 g
2. Temperature

A DS18B20 temperature sensor monitors the machine temperature.

Current temperature threshold:

TEMPERATURE_THRESHOLD = 35 °C

If the measured temperature reaches or exceeds the threshold, the system reports:

HIGH TEMPERATURE DETECTED

and the motor is stopped.

3. Electrical Current

An INA219 current/voltage sensor measures:

Bus voltage
Current
Electrical power

The measured current is compared against two thresholds.

LOW CURRENT THRESHOLD  = 50 mA
HIGH CURRENT THRESHOLD = 300 mA

The resulting states are:

LOW CURRENT
NORMAL CURRENT
HIGH CURRENT

** INA219 Current Diagnostic System**

One of the important features of Machine Guard is the INA219-based current diagnostic system.

The INA219 provides real-time electrical measurements:

Voltage
Current
Power

Example:

INA219 | Voltage: 4.104 V | Current: 135.3 mA | Power: 1354.0 mW

The current is then classified according to the configured thresholds.

Current classification
Current < 50 mA
        ↓
LOW CURRENT

50 mA ≤ Current ≤ 300 mA
        ↓
NORMAL CURRENT

Current > 300 mA
        ↓
HIGH CURRENT

This allows electrical abnormalities to be detected independently from vibration and temperature.

** Controlled Current Fault Simulation**

Machine Guard includes a test system that allows current-related fault conditions to be deliberately simulated.

This is useful during development because actual electrical faults do not have to be created to test the detection logic.

The Serial Monitor accepts keyboard commands.

Serial Commands
Command	Function
N	Normal live monitoring
H	High-current test mode
L	Low-current test mode
S	Stop motor
Normal Mode

Press:

N

The system returns to real-time current monitoring.

The actual INA219 measurement is used to determine:

LOW CURRENT
NORMAL CURRENT
HIGH CURRENT
High Current Test

Press:

H

The system enters:

HIGH CURRENT TEST MODE

The diagnostic result is deliberately forced to:

HIGH CURRENT DETECTED

while the actual INA219 current continues to be displayed.

This allows the fault-detection interface and logic to be tested safely.

Low Current Test

Press:

L

The system enters:

LOW CURRENT TEST MODE

The diagnostic result is deliberately forced to:

LOW CURRENT DETECTED

The actual INA219 measurement remains visible.

Stop Motor

Press:

S

The motor is stopped immediately.

The display reports:
**
MOTOR STOPPED**

The system remains stopped until normal operation is requested again.

Press:

N

to resume monitoring.

**Safety Detection**

Machine Guard continuously evaluates the sensor measurements.

The current implementation checks:

Vibration
Temperature

If a critical vibration or temperature condition is detected, the motor is stopped.

High vibration
!!! HIGH VIBRATION DETECTED !!!

The motor is stopped and the LCD displays the vibration warning.

High temperature
!!! HIGH TEMPERATURE DETECTED !!!

The motor is stopped and the LCD displays the temperature warning.

This provides a basic machine-protection mechanism.

 **Sensor Fusion**

Machine Guard is designed around a multi-sensor monitoring approach.

Instead of depending on a single sensor:

        MPU6050
           │
           ▼
       Vibration

the system combines:

              MPU6050
             Vibration
                 │
                 │
DS18B20 ──────── ESP32-C3 ──────── INA219
Temperature       │                Current
                 │                 Voltage
                 │                 Power
                 ▼
          Condition Monitoring
                 │
       ┌─────────┼─────────┐
       ▼         ▼         ▼
    Normal    Warning    Fault

This architecture can later be extended into a machine-learning based fault-classification system.

🤖 Future TinyML / Machine Learning Extension

Machine Guard is also structured as a foundation for future TinyML development.

Sensor features can be collected into a dataset containing:

Vibration
Temperature
Current
Voltage
Power

Example dataset structure:

Timestamp,
Vibration,
Temperature,
Current,
Voltage,
Power,
Status

Possible machine states could include:

NORMAL
LOW_CURRENT
HIGH_CURRENT
HIGH_VIBRATION
HIGH_TEMPERATURE

The collected dataset can later be used to train an edge machine-learning model.

The final architecture could become:

             Sensors
                │
                ▼
          Feature Extraction
                │
                ▼
          TinyML Model
                │
       ┌────────┼────────┐
       ▼        ▼        ▼
    NORMAL    WARNING    FAULT
                │
                ▼
          Motor Protection
🔌 Hardware
**Main Controller**
ESP32-C3
Sensors
MPU6050 — vibration / acceleration
DS18B20 — temperature
INA219 — voltage / current / power
Display
16×2 I²C LCD
Actuator
DC motor
Motor driver

** GPIO Configuration**

The current hardware configuration is:

Component	ESP32-C3 GPIO
I²C SDA	GPIO 6
I²C SCL	GPIO 7
DS18B20	GPIO 4
Motor PWM	GPIO 3
Motor AIN1	GPIO 5
Motor AIN2	GPIO 8
Motor STBY	GPIO 10
I²C addresses
MPU6050 → 0x68
INA219  → 0x40
LCD     → 0x27
🔧 Motor Control

The motor is controlled through:

PWMA
AIN1
AIN2
STBY

The configured motor speed is:

MOTOR_SPEED = 80

Motor control functions include:

motorStart()
motorStop()

The system starts the motor after initialization and allows the operator to stop or restart it through the Serial Monitor.

🖥️ Serial Monitor

The system uses:

115200 baud

The Serial Monitor provides real-time sensor information.

Example format:

X: 0.012 g | Y: -0.018 g | Z: 1.002 g

VIBRATION: 0.002 g

TEMPERATURE: 29.45 C

INA219 | Voltage: 4.104 V | Current: 135.3 mA | Power: 554.9 mW

The current diagnostic section reports:

****************************************
           CURRENT DIAGNOSIS
****************************************

Measured Current : 135.3 mA
Low Threshold    : 50.0 mA
High Threshold   : 300.0 mA

RESULT           : NORMAL CURRENT
>>> CURRENT NORMAL <<<
** LCD Display**

The 16×2 LCD provides a compact local machine-status display.

During normal operation it can display:

V:0.123g T:29.5C
NORMAL

During abnormal conditions, the LCD changes to a warning message.

Examples:

HIGH VIBRATION
0.523 g

or:

HIGH TEMP
36.2 C

The LCD therefore provides an immediate local indication without requiring a computer.

** Signal Filtering**

The MPU6050 readings include basic validation and spike rejection.

A maximum permitted sample change is defined as:

MAX_SAMPLE_CHANGE = 0.50

If an acceleration reading changes beyond this limit, the reading is treated as a possible spike and rejected.

This helps prevent a single abnormal sensor sample from immediately triggering a fault condition.

** Vibration Statistics**

Machine Guard maintains running vibration statistics.

The system records:

Minimum vibration
Maximum vibration
Average vibration

Example:

MIN: 0.001 g
MAX: 0.076 g
AVG: 0.023 g

These values can later become useful features for machine-learning analysis.

** Operating Flow**

The overall software flow is:

             START
               │
               ▼
        Initialize ESP32-C3
               │
               ▼
        Initialize I²C
               │
       ┌───────┼────────┐
       ▼       ▼        ▼
    MPU6050  INA219   DS18B20
       │       │        │
       └───────┼────────┘
               ▼
          Initialize LCD
               │
               ▼
          Start Motor
               │
               ▼
        Read Sensor Data
               │
       ┌───────┼──────────┐
       ▼       ▼          ▼
   Vibration Temperature Current
       │       │          │
       └───────┼──────────┘
               ▼
         Fault Evaluation
               │
       ┌───────┼────────┐
       ▼       ▼        ▼
    NORMAL   WARNING   FAULT
                         │
                         ▼
                    Stop Motor

** Development Test Modes**

The project includes controlled testing of the current-detection system.

Test 1 — Normal
N

Expected:

TEST MODE : NORMAL LIVE

The actual current is evaluated against the thresholds.

Test 2 — Low Current
L

Expected:

TEST MODE : LOW CURRENT TEST

RESULT : LOW CURRENT
>>> LOW CURRENT DETECTED <<<
Test 3 — High Current
H

Expected:

TEST MODE : HIGH CURRENT TEST

RESULT : HIGH CURRENT
>>> HIGH CURRENT DETECTED <<<
Test 4 — Motor Stop
S

Expected:

MOTOR STOPPED
Press N

** Suggested Repository Structure**

A clean GitHub repository can be organized as:

Machine-Guard/
│
├── README.md
│
├── firmware/
│   └── Machine_Guard.ino
│
├── hardware/
│   ├── wiring.md
│   └── circuit_diagram.png
│
├── dataset/
│   └── README.md
│
├── documentation/
│   └── system_architecture.md
│
└── images/
    ├── prototype.jpg
    ├── serial_monitor.jpg
    └── lcd_display.jpg

** Software Requirements**

The project is developed using the Arduino environment with ESP32 board support.

Required libraries:

Wire
Adafruit INA219
OneWire
DallasTemperature
LiquidCrystal_I2C

The MPU6050 is accessed directly through I²C registers in the current firmware.

** Configuration**

Important parameters are defined near the beginning of the program.

Vibration
#define VIBRATION_THRESHOLD   0.400
Temperature
#define TEMPERATURE_THRESHOLD 35.0
Current
#define LOW_CURRENT_THRESHOLD  50.0
#define HIGH_CURRENT_THRESHOLD 300.0
Motor speed
#define MOTOR_SPEED 80
MPU6050 spike filtering
#define MAX_SAMPLE_CHANGE 0.50

These values can be adjusted during experimentation.

** Current Measurement Philosophy**

The INA219 measurement is treated as an important electrical feature of the machine.

The system displays:

Bus Voltage
Current
Power

The current classification is based on the measured current.

This provides an electrical signature that can be compared with mechanical vibration and temperature.

For example:

Normal machine:

Current     → Normal
Vibration   → Normal
Temperature → Normal

Possible abnormal condition:

Current     → High
Vibration   → High
Temperature → Increasing

This multi-parameter behavior is particularly useful for future machine-learning experiments.

**Future Improvements**

Possible future development includes:

1. Dataset Collection

Automatically save:

Voltage
Current
Power
Temperature
Vibration
Status

into CSV files.

**3. Automatic Fault Classification
**
Classify machine conditions such as:

Normal
Low Current
High Current
High Vibration
High Temperature
Combined Fault
**4. IoT Connectivity**

Add wireless monitoring using the ESP32-C3.

Possible future interface:

Machine
   │
   ▼
ESP32-C3
   │
   ▼
Wi-Fi
   │
   ▼
Cloud / Dashboard
5.** Predictive Maintenance**

Instead of only detecting a fault after it occurs, the system could eventually identify patterns that indicate developing machine degradation.

🎓 Project Applications

Machine Guard can be used as a prototype for:

Industrial motor monitoring
Predictive maintenance
Embedded condition monitoring
Edge AI research
TinyML applications
Industrial IoT
Electrical fault monitoring
Mechanical fault monitoring
Academic ECE projects
Sensor-fusion research
**Project Status**

Current implementation includes:

 ESP32-C3 controller
 MPU6050 vibration monitoring
 DS18B20 temperature monitoring
 INA219 voltage monitoring
 INA219 current monitoring
 INA219 power monitoring
 16×2 I²C LCD
 DC motor control
 Vibration threshold detection
 Temperature threshold detection
 Current threshold detection
 Low-current test mode
 High-current test mode
 Normal live mode
 Manual motor stop
 Serial command control
 Sensor spike filtering
 Vibration statistics
 Automated dataset collection
 TinyML fault classifier
 Wireless dashboard
 Predictive maintenance model
 License

This project can be released under an open-source license of your choice.

For example:

MIT License
 Project Summary

Machine Guard is a multi-sensor embedded machine-condition monitoring platform built around the ESP32-C3.

It combines:

MPU6050
   +
DS18B20
   +
INA219
   +
ESP32-C3
   +
LCD
   +
Motor Control

to create a real-time machine monitoring system.

The key idea is to observe the machine from multiple physical domains:

Mechanical
    ↓
Vibration

Thermal
    ↓
Temperature

Electrical
    ↓
Current / Voltage / Power

These measurements are processed locally by the ESP32-C3 and converted into understandable machine-condition information.

The current implementation establishes the embedded monitoring and fault-detection foundation. The next major development stage is to use the collected multi-sensor data for TinyML-based machine fault classification and predictive maintenance.

 Machine Guard
Multi-Sensor Edge Machine Condition Monitoring
Sense → Process → Diagnose → Protect → Learn
