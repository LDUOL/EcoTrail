#include <Adafruit_SI1145.h>

#include <LiquidCrystal_I2C.h> //LCD

#include <Wire.h> //I2C

#include <Adafruit_Sensor.h> // BMP280
#include <Adafruit_BMP280.h> // BMP280

//-----------------------------//

Adafruit_SI1145 sunlightSensor = Adafruit_SI1145(); // Grv Sunlight

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD address and size

Adafruit_BMP280 bmp; // BMP280 address

// Taster Pin
const int buttonPin = D3;
int buttonState = 0;
int lastButtonState = 0;
int displayMode = 0; // 0 = BMP280, 1 = Sunlight Sensor, 2 = MQ 135 Gassensor

// MQ-135 Pin
const int mq135Pin = A0;

// LED Pins
const int redLEDPin = D5;
const int yellowLEDPin = D6;
const int greenLEDPin = D7;

// Threshold values for critical and warning levels
const float temperatureCritical = 35.0; // Critical temperature in Celsius
const float temperatureWarning = 30.0;  // Warning temperature in Celsius

const float pressureCritical = 600.0;  // Critical pressure in hPa
const float pressureWarning = 800.0;   // Warning pressure in hPa

const float uvCritical = 10.0;          // Critical UV index
const float uvWarning = 7.0;            // Warning UV index

const int mq135Critical = 700;          // Critical MQ-135 value
const int mq135Warning = 300;           // Warning MQ-135 value

//----------------------------//

void setup() {
  Serial.begin(115200); // Initialize serial communication

  lcd.init(); // Initialize the LCD
  lcd.backlight();

  // Initialize the BMP280
  if (!bmp.begin(0x76)) { // Check the I2C address of BMP280
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  // Initialize Grv Sunlight
  if (!sunlightSensor.begin()) {
    Serial.println("Could not find a valid Grove Sunlight Sensor, check wiring!");
    while (1);
  }

  // Initialize the button pin as an input
  pinMode(buttonPin, INPUT);

  // Print initial message to LCD
  lcd.setCursor(0, 0);
  lcd.print("System starting");
  lcd.setCursor(0, 1);
  lcd.print("     ...");

  delay(5000); // Show the message for 5 seconds
  lcd.clear();
}

void loop() { //-------------------------------------------------------------------------------------------//
  // Variables to hold sensor values
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Convert to hPa
  float uv = sunlightSensor.readUV() / 100.0;   // Convert to UV index
  int mq135Value = analogRead(mq135Pin);

  // Reset LED states
  digitalWrite(redLEDPin, LOW);
  digitalWrite(yellowLEDPin, LOW);
  digitalWrite(greenLEDPin, HIGH);

  // Check for critical or warning values
  if (temperature >= temperatureCritical || pressure <= pressureCritical || uv >= uvCritical || mq135Value >= mq135Critical) {
    // Critical condition
    digitalWrite(redLEDPin, HIGH);
    digitalWrite(greenLEDPin, LOW);
  } else if (temperature >= temperatureWarning || pressure <= pressureWarning || uv >= uvWarning || mq135Value >= mq135Warning) {
    // Warning condition
    digitalWrite(yellowLEDPin, HIGH);
    digitalWrite(greenLEDPin, LOW);
  }

  // Read the button state
  buttonState = digitalRead(buttonPin);

  // Check if the button is pressed
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      // Change the display mode
      displayMode = (displayMode + 1) % 2;
      lcd.clear(); //!! not tested
    }
    delay(50); // Debounce delay
  }

  lastButtonState = buttonState;

  // Display based on the current mode
  if (displayMode == 0) {

    // Read temperature and pressure from BMP280
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // Convert to hPa

    // Print temperature to LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C");

    // Print pressure to LCD
    lcd.setCursor(0, 1);
    lcd.print("Pressure: ");
    lcd.print(pressure);
    lcd.print(" hPa");

  } else if (displayMode == 1) {

    // Read data from Grove Sunlight Sensor
    float uv = sunlightSensor.readUV();
    uv /= 100.0; // Convert to UV index

    float visible = sunlightSensor.readVisible();
    float ir = sunlightSensor.readIR();

    // Print UV index to LCD
    lcd.setCursor(0, 0);
    lcd.print("UV Index: ");
    lcd.print(uv);

  } else if(displayMode == 2) {

    // Read analog value from MQ-135
    int mq135Value = analogRead(mq135Pin);

    // Print MQ-135 value to LCD
    lcd.setCursor(0, 0);
    lcd.print("MQ-135 Value:");
    lcd.setCursor(0, 1);
    lcd.print(mq135Value);

  }

  // Wait for 500 milliseconds before updating
  delay(500);
}
