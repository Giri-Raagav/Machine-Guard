#include <Wire.h>
#include <Adafruit_INA219.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// =====================================================
// I2C
// =====================================================

#define SDA_PIN 6
#define SCL_PIN 7

#define MPU6050_ADDR 0x68
#define INA219_ADDR  0x40
#define LCD_ADDR     0x27

// =====================================================
// DS18B20
// =====================================================

#define DS18B20_PIN 4

OneWire oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);

// =====================================================
// LCD
// =====================================================

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// =====================================================
// INA219
// =====================================================

Adafruit_INA219 ina219(INA219_ADDR);

// =====================================================
// MOTOR
// =====================================================

#define PWMA  3
#define AIN1  5
#define AIN2  8
#define STBY 10

#define MOTOR_SPEED 80

// =====================================================
// SAFETY THRESHOLDS
// =====================================================

#define VIBRATION_THRESHOLD   0.400
#define TEMPERATURE_THRESHOLD 35.0

// =====================================================
// CURRENT DIAGNOSTIC THRESHOLDS
// =====================================================

#define LOW_CURRENT_THRESHOLD  50.0
#define HIGH_CURRENT_THRESHOLD 300.0

// =====================================================
// SPIKE FILTER
// =====================================================

#define MAX_SAMPLE_CHANGE 0.50

// =====================================================
// MPU6050 REGISTERS
// =====================================================

#define PWR_MGMT_1   0x6B
#define ACCEL_CONFIG 0x1C
#define ACCEL_XOUT_H 0x3B

// =====================================================
// VARIABLES
// =====================================================

float previousX = 0;
float previousY = 0;
float previousZ = 0;

bool havePrevious = false;

bool safetyTriggered = false;
bool motorRunning = false;
bool manualStop = false;

// =====================================================
// CURRENT TEST MODE
// =====================================================
// 0 = NORMAL LIVE MODE
// 1 = HIGH CURRENT TEST
// 2 = LOW CURRENT TEST

byte currentTestMode = 0;

// =====================================================
// VIBRATION STATISTICS
// =====================================================

float minVibration = 999.0;
float maxVibration = 0.0;
float sumVibration = 0.0;

unsigned long validReadings = 0;
unsigned long invalidReadings = 0;
unsigned long rejectedSpikes = 0;

// =====================================================
// MPU6050 WRITE
// =====================================================

void writeMPU(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// =====================================================
// MPU6050 READ
// =====================================================

bool readAcceleration(float &x, float &y, float &z)
{
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(ACCEL_XOUT_H);

  if (Wire.endTransmission(false) != 0)
    return false;

  if (Wire.requestFrom(MPU6050_ADDR, (uint8_t)6) != 6)
    return false;

  int16_t rawX = (Wire.read() << 8) | Wire.read();
  int16_t rawY = (Wire.read() << 8) | Wire.read();
  int16_t rawZ = (Wire.read() << 8) | Wire.read();

  x = rawX / 16384.0;
  y = rawY / 16384.0;
  z = rawZ / 16384.0;

  if (fabs(x) > 2.1 ||
      fabs(y) > 2.1 ||
      fabs(z) > 2.1)
  {
    return false;
  }

  return true;
}

// =====================================================
// MOTOR START
// =====================================================

void motorStart()
{
  digitalWrite(STBY, HIGH);

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  analogWrite(PWMA, MOTOR_SPEED);

  motorRunning = true;
  manualStop = false;
}

// =====================================================
// MOTOR STOP
// =====================================================

void motorStop()
{
  analogWrite(PWMA, 0);

  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);

  digitalWrite(STBY, LOW);

  motorRunning = false;
}

// =====================================================
// SERIAL COMMAND HANDLER
// =====================================================

void checkSerialCommand()
{
  if (!Serial.available())
    return;

  char command = Serial.read();

  if (command >= 'a' && command <= 'z')
    command = command - 32;

  // ===================================================
  // STOP
  // ===================================================

  if (command == 'S')
  {
    currentTestMode = 0;
    manualStop = true;

    motorStop();

    Serial.println();
    Serial.println("========================================");
    Serial.println("             MOTOR STOPPED");
    Serial.println("========================================");
    Serial.println("Press N to resume normal monitoring.");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MOTOR STOPPED");
    lcd.setCursor(0, 1);
    lcd.print("Press N");

    return;
  }

  // ===================================================
  // NORMAL
  // ===================================================

  if (command == 'N')
  {
    currentTestMode = 0;

    manualStop = false;
    safetyTriggered = false;

    motorStart();

    Serial.println();
    Serial.println("========================================");
    Serial.println("        NORMAL LIVE MONITORING");
    Serial.println("========================================");
    Serial.println("INA219 current diagnosis = LIVE");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NORMAL MODE");
    lcd.setCursor(0, 1);
    lcd.print("LIVE MONITOR");

    delay(500);
    return;
  }

  // ===================================================
  // HIGH CURRENT TEST
  // ===================================================

  if (command == 'H')
  {
    currentTestMode = 1;

    manualStop = false;
    safetyTriggered = false;

    motorStart();

    Serial.println();
    Serial.println("****************************************");
    Serial.println("        HIGH CURRENT TEST MODE");
    Serial.println("****************************************");
    Serial.println("Current diagnosis will be FORCED HIGH");
    Serial.println("Actual INA219 current is still displayed.");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HIGH CURRENT");
    lcd.setCursor(0, 1);
    lcd.print("TEST MODE");

    delay(500);
    return;
  }

  // ===================================================
  // LOW CURRENT TEST
  // ===================================================

  if (command == 'L')
  {
    currentTestMode = 2;

    manualStop = false;
    safetyTriggered = false;

    motorStart();

    Serial.println();
    Serial.println("****************************************");
    Serial.println("         LOW CURRENT TEST MODE");
    Serial.println("****************************************");
    Serial.println("Current diagnosis will be FORCED LOW");
    Serial.println("Actual INA219 current is still displayed.");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LOW CURRENT");
    lcd.setCursor(0, 1);
    lcd.print("TEST MODE");

    delay(500);
    return;
  }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("========================================");
  Serial.println("      ESP32-C3 SAFETY MONITOR");
  Serial.println("MPU6050 + DS18B20 + INA219 + LCD");
  Serial.println("========================================");

  // ===================================================
  // MOTOR PINS
  // ===================================================

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  motorStop();

  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  delay(300);

  Serial.println();
  Serial.println("I2C CONFIGURATION");
  Serial.println("------------------");
  Serial.println("SDA = GPIO 6");
  Serial.println("SCL = GPIO 7");

  // ===================================================
  // LCD
  // ===================================================

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SAFETY MONITOR");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(1500);

  // ===================================================
  // MPU6050 CHECK
  // ===================================================

  Wire.beginTransmission(MPU6050_ADDR);

  if (Wire.endTransmission() != 0)
  {
    Serial.println("ERROR: MPU6050 NOT FOUND!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MPU6050 ERROR");
    lcd.setCursor(0, 1);
    lcd.print("NOT FOUND");

    while (1)
      delay(1000);
  }

  Serial.println("MPU6050 FOUND AT 0x68");

  writeMPU(PWR_MGMT_1, 0x00);
  delay(100);

  writeMPU(ACCEL_CONFIG, 0x00);
  delay(100);

  Serial.println("MPU6050 INITIALIZED");

  // ===================================================
  // INA219
  // ===================================================

  if (!ina219.begin())
  {
    Serial.println("ERROR: INA219 NOT FOUND!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INA219 ERROR");
    lcd.setCursor(0, 1);
    lcd.print("NOT FOUND");

    while (1)
      delay(1000);
  }

  Serial.println("INA219 FOUND AT 0x40");
  Serial.println("INA219 INITIALIZED");

  // ===================================================
  // DS18B20
  // ===================================================

  tempSensor.begin();

  int deviceCount = tempSensor.getDeviceCount();

  Serial.print("DS18B20 DEVICES: ");
  Serial.println(deviceCount);

  if (deviceCount == 0)
  {
    Serial.println("ERROR: DS18B20 NOT FOUND!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DS18B20 ERROR");
    lcd.setCursor(0, 1);
    lcd.print("NOT FOUND");

    while (1)
      delay(1000);
  }

  Serial.println("DS18B20 INITIALIZED");

  // ===================================================
  // INITIAL TEMPERATURE
  // ===================================================

  tempSensor.requestTemperatures();

  float initialTemperature =
      tempSensor.getTempCByIndex(0);

  Serial.print("Initial temperature: ");
  Serial.print(initialTemperature, 2);
  Serial.println(" C");

  // ===================================================
  // CURRENT THRESHOLDS
  // ===================================================

  Serial.println();
  Serial.println("CURRENT DIAGNOSTIC");
  Serial.println("------------------");

  Serial.print("LOW  threshold  : ");
  Serial.print(LOW_CURRENT_THRESHOLD, 1);
  Serial.println(" mA");

  Serial.print("HIGH threshold  : ");
  Serial.print(HIGH_CURRENT_THRESHOLD, 1);
  Serial.println(" mA");

  // ===================================================
  // COMMANDS
  // ===================================================

  Serial.println();
  Serial.println("COMMANDS");
  Serial.println("------------------");
  Serial.println("N = NORMAL LIVE MONITORING");
  Serial.println("H = HIGH CURRENT TEST");
  Serial.println("L = LOW CURRENT TEST");
  Serial.println("S = STOP MOTOR");
  Serial.println();

  // ===================================================
  // READY
  // ===================================================

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM READY");
  lcd.setCursor(0, 1);
  lcd.print("Motor starts...");

  Serial.println("SYSTEM READY");
  Serial.println("Motor starting in 2 seconds...");

  delay(2000);

  motorStart();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MOTOR RUNNING");
  lcd.setCursor(0, 1);
  lcd.print("STATUS: NORMAL");

  Serial.println("MOTOR STARTED");
  Serial.println("----------------------------------------");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  // ===================================================
  // CHECK SERIAL COMMAND
  // ===================================================

  checkSerialCommand();

  // ===================================================
  // MANUAL STOP
  // ===================================================

  if (manualStop)
  {
    motorStop();
    delay(100);
    return;
  }

  // ===================================================
  // SAFETY LATCH
  // ===================================================

  if (safetyTriggered)
  {
    motorStop();
    delay(100);
    return;
  }

  // ===================================================
  // READ MPU6050
  // ===================================================

  float x, y, z;

  if (!readAcceleration(x, y, z))
  {
    invalidReadings++;

    Serial.println("IGNORED: INVALID MPU6050 READING");

    delay(50);
    return;
  }

  // ===================================================
  // FIRST VALID READING
  // ===================================================

  if (!havePrevious)
  {
    previousX = x;
    previousY = y;
    previousZ = z;

    havePrevious = true;

    Serial.println("First valid MPU6050 reading accepted.");

    delay(50);
    return;
  }

  // ===================================================
  // SPIKE PROTECTION
  // ===================================================

  float changeX = fabs(x - previousX);
  float changeY = fabs(y - previousY);
  float changeZ = fabs(z - previousZ);

  if (changeX > MAX_SAMPLE_CHANGE ||
      changeY > MAX_SAMPLE_CHANGE ||
      changeZ > MAX_SAMPLE_CHANGE)
  {
    rejectedSpikes++;

    Serial.println("IGNORED SPIKE");

    delay(50);
    return;
  }

  previousX = x;
  previousY = y;
  previousZ = z;

  validReadings++;

  // ===================================================
  // VIBRATION
  // ===================================================

  float magnitude = sqrt(
    x * x +
    y * y +
    z * z
  );

  float vibration = fabs(magnitude - 1.0);

  // ===================================================
  // TEMPERATURE
  // ===================================================

  tempSensor.requestTemperatures();

  float temperature =
      tempSensor.getTempCByIndex(0);

  bool temperatureValid =
      (temperature != DEVICE_DISCONNECTED_C &&
       temperature > -40.0 &&
       temperature < 125.0);

  // ===================================================
  // INA219
  // ===================================================

  float busVoltage =
      ina219.getBusVoltage_V();

  float current_mA =
      ina219.getCurrent_mA();

  float power_mW =
      ina219.getPower_mW();

  // ===================================================
  // STATISTICS
  // ===================================================

  if (vibration < minVibration)
    minVibration = vibration;

  if (vibration > maxVibration)
    maxVibration = vibration;

  sumVibration += vibration;

  float averageVibration =
      sumVibration / validReadings;

  // ===================================================
  // SERIAL SENSOR DATA WITH UNITS
  // ===================================================

  Serial.println();
  Serial.println("========================================");

  Serial.print("X: ");
  Serial.print(x, 3);
  Serial.print(" g | Y: ");
  Serial.print(y, 3);
  Serial.print(" g | Z: ");
  Serial.print(z, 3);
  Serial.println(" g");

  Serial.print("VIBRATION: ");
  Serial.print(vibration, 3);
  Serial.println(" g");

  Serial.print("TEMPERATURE: ");

  if (temperatureValid)
    Serial.print(temperature, 2);
  else
    Serial.print("INVALID");

  Serial.println(" C");

  Serial.print("INA219 | Voltage: ");
  Serial.print(busVoltage, 3);
  Serial.print(" V | Current: ");
  Serial.print(current_mA, 1);
  Serial.print(" mA | Power: ");
  Serial.print(power_mW, 1);
  Serial.println(" mW");

  // ===================================================
  // CURRENT DIAGNOSIS
  // ===================================================

  bool lowCurrent = false;
  bool highCurrent = false;
  bool normalCurrent = false;

  if (currentTestMode == 0)
  {
    if (current_mA < LOW_CURRENT_THRESHOLD)
    {
      lowCurrent = true;
    }
    else if (current_mA > HIGH_CURRENT_THRESHOLD)
    {
      highCurrent = true;
    }
    else
    {
      normalCurrent = true;
    }
  }
  else if (currentTestMode == 1)
  {
    highCurrent = true;
  }
  else if (currentTestMode == 2)
  {
    lowCurrent = true;
  }

  // ===================================================
  // CURRENT DIAGNOSIS DISPLAY
  // ===================================================

  Serial.println();
  Serial.println("****************************************");
  Serial.println("           CURRENT DIAGNOSIS");
  Serial.println("****************************************");

  Serial.print("Measured Current : ");
  Serial.print(current_mA, 1);
  Serial.println(" mA");

  Serial.print("Low Threshold    : ");
  Serial.print(LOW_CURRENT_THRESHOLD, 1);
  Serial.println(" mA");

  Serial.print("High Threshold   : ");
  Serial.print(HIGH_CURRENT_THRESHOLD, 1);
  Serial.println(" mA");

  if (highCurrent)
  {
    Serial.println("RESULT           : HIGH CURRENT");
    Serial.println(">>> HIGH CURRENT DETECTED <<<");
  }
  else if (lowCurrent)
  {
    Serial.println("RESULT           : LOW CURRENT");
    Serial.println(">>> LOW CURRENT DETECTED <<<");
  }
  else
  {
    Serial.println("RESULT           : NORMAL CURRENT");
    Serial.println(">>> CURRENT NORMAL <<<");
  }

  Serial.println("----------------------------------------");

  if (currentTestMode == 1)
  {
    Serial.println("TEST MODE : HIGH CURRENT TEST");
  }
  else if (currentTestMode == 2)
  {
    Serial.println("TEST MODE : LOW CURRENT TEST");
  }
  else
  {
    Serial.println("TEST MODE : NORMAL LIVE");
  }

  Serial.println("----------------------------------------");

  // ===================================================
  // SAFETY CHECK
  // ===================================================

  bool highVibration =
      vibration >= VIBRATION_THRESHOLD;

  bool highTemperature =
      temperatureValid &&
      temperature >= TEMPERATURE_THRESHOLD;

  // ===================================================
  // HIGH VIBRATION SAFETY
  // ===================================================

  if (highVibration)
  {
    motorStop();
    safetyTriggered = true;

    Serial.println();
    Serial.println("!!! HIGH VIBRATION DETECTED !!!");

    Serial.print("Vibration = ");
    Serial.print(vibration, 3);
    Serial.println(" g");

    if (highTemperature)
    {
      Serial.println("!!! HIGH TEMPERATURE DETECTED !!!");

      Serial.print("Temperature = ");
      Serial.print(temperature, 2);
      Serial.println(" C");
    }

    Serial.println("MOTOR STOPPED");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HIGH VIBRATION");
    lcd.setCursor(0, 1);
    lcd.print(vibration, 3);
    lcd.print(" g");

    return;
  }

  // ===================================================
  // HIGH TEMPERATURE SAFETY
  // ===================================================

  if (highTemperature)
  {
    motorStop();
    safetyTriggered = true;

    Serial.println();
    Serial.println("!!! HIGH TEMPERATURE DETECTED !!!");

    Serial.print("Temperature = ");
    Serial.print(temperature, 2);
    Serial.println(" C");

    Serial.print("Vibration = ");
    Serial.print(vibration, 3);
    Serial.println(" g");

    Serial.println("MOTOR STOPPED");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HIGH TEMP");

    lcd.setCursor(0, 1);
    lcd.print(temperature, 1);
    lcd.print(" C");

    return;
  }

  // ===================================================
  // LCD NORMAL DISPLAY
  // ===================================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(vibration, 2);
  lcd.print("g ");

  if (temperatureValid)
  {
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C");
  }

  lcd.setCursor(0, 1);

  if (highCurrent)
  {
    lcd.print("HIGH I:");
    lcd.print(current_mA, 0);
    lcd.print("mA");
  }
  else if (lowCurrent)
  {
    lcd.print("LOW I:");
    lcd.print(current_mA, 0);
    lcd.print("mA");
  }
  else
  {
    lcd.print("NORMAL I:");
    lcd.print(current_mA, 0);
    lcd.print("mA");
  }

  // ===================================================
  // SERIAL STATUS
  // ===================================================

  Serial.println("STATUS: NORMAL");

  Serial.print("MIN: ");
  Serial.print(minVibration, 3);
  Serial.print(" g | MAX: ");
  Serial.print(maxVibration, 3);
  Serial.print(" g | AVG: ");
  Serial.print(averageVibration, 3);
  Serial.println(" g");

  Serial.println("----------------------------------------");

  delay(200);
}
