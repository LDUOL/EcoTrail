#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <LiquidCrystal_I2C.h>

// Pins for MP135 sensor
#define MP135_PIN A0

// BMP280 I2C address
#define BMP280_I2C_ADDRESS 0x76

// LED pins
#define GREEN_LED_PIN D5
#define YELLOW_LED_PIN D6
#define RED_LED_PIN D7


// Initialize the LiquidCrystal_I2C library with the I2C address of your LCD (usually 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// BMP280 sensor object
Adafruit_BMP280 bmp;

// MPU6050 sensor object
Adafruit_MPU6050 mpu;

// Variables for step counting
int stepCounter = 0;
int motionCount = 0;

// program start
bool programEnabled = true;


void setup() {
  Serial.begin(115200);

  // Initialize the BMP280 sensor
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }


  // Initialize the MPU6050 sensor
  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Setup motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);	// Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  delay(100);
  

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  // Initialize the LED pins
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  delay(100);
}

void loop() {

  if (mpu.getMotionInterruptStatus()) {
      stepCounter++;
      delay(10);
  }
  // Read MP135 sensor value
  int mp135Value = analogRead(MP135_PIN);

  // Read BMP280 sensor values
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude(1021.00); // Adjust for your local sea level pressure


  // Calculate the magnitude of the acceleration vector
  if (mpu.getMotionInterruptStatus()) {
      stepCounter++;
      delay(10);
  }

// LED control based on temperature and pressure
  if (temperature > 35.0 || pressure < 950.0 || mp135Value < 512) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else if ((temperature >= 30.0 && temperature <= 35.0) || (pressure >= 950.0 && pressure <= 1000.0) || (mp135Value <= 1023 && mp135Value >=513)) {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }

  // Print values to Serial for debugging
  Serial.print("MP135 Value: ");
  if (mp135Value < 1024) {
    Serial.print("Gefährliches Gas erkannt");
  } else {
    Serial.print("Kein schädliches Gas");
  }
  Serial.println(mp135Value);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");
  Serial.print("Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");
  Serial.print("Steps: ");
  Serial.println(stepCounter);
  

  // Display values on the LCD in a loop

  // First screen: MP135 and temperature
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp.:");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  if (mp135Value < 1024) {
    lcd.print("!GAS-GEFAHR!");
  } else {
    lcd.print("Keine Gas-Gefahr");
  }
  if (mpu.getMotionInterruptStatus()) {
    stepCounter++;
    delay(10);
  }
  delay(2000); // Wait for 2 seconds

  // Second screen: Pressure and Altitude
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Druck:");
  lcd.print(pressure / 100); // Convert Pa to hPa
  lcd.print("hPa");
  lcd.setCursor(0, 1);
  lcd.print("Hoehe:");
  lcd.print(altitude);
  lcd.print("m");
  if (mpu.getMotionInterruptStatus()) {
    stepCounter++;
    delay(10);
  }
  delay(2000); // Wait for 2 seconds

  // Third screen: Step Counter
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Schritte:");
  lcd.print(stepCounter);
  if (mpu.getMotionInterruptStatus()) {
    stepCounter++;
      delay(10);
  }
  delay(2000); // Wait for 2 seconds
}
