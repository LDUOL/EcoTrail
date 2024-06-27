#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WLAN-Konfigurationsdaten
const char* ssid = "-"; // Ersetze dies durch deinen WLAN-SSID
const char* password = "-"; // Ersetze dies durch dein WLAN-Passwort

// Webserver initialisieren
ESP8266WebServer server(80);

// Pins für MP135 Sensor
#define MP135_PIN A0

// BMP280 I2C Adresse
#define BMP280_I2C_ADDRESS 0x76

// LED Pins
#define GREEN_LED_PIN D5
#define YELLOW_LED_PIN D6
#define RED_LED_PIN D7

// Button Pin
#define BUTTON_PIN D8

// Initialisiere die LiquidCrystal_I2C Bibliothek mit der I2C Adresse deines LCDs (meist 0x27 oder 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// BMP280 Sensorobjekt
Adafruit_BMP280 bmp;

// MPU6050 Sensorobjekt
Adafruit_MPU6050 mpu;

// Variablen für Schrittzählung
int stepCounter = 0;
int motionCount = 0;

// Bildschirmstatus Variable
int screenState = 0;

// Debounce Variablen
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = LOW;
int buttonState = LOW;

void setup() {
  Serial.begin(115200);

  // Mit WLAN verbinden
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Webserver-Routen definieren
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  // Initialisiere den BMP280 Sensor
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  // Initialisiere den MPU6050 Sensor
  Wire.begin();
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Setup Bewegungserkennung
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  delay(100);

  // Initialisiere das LCD
  lcd.init();
  lcd.backlight();

  // Initialisiere die LED Pins
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  // Initialisiere den Button Pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  delay(100);
}

void loop() {
  server.handleClient();

  // Lese den Buttonzustand
  int reading = digitalRead(BUTTON_PIN);

  // Prüfe auf Button-Druck
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        screenState = (screenState + 1) % 3;
      }
    }
  }

  lastButtonState = reading;

  if (mpu.getMotionInterruptStatus()) {
      stepCounter++;
      delay(10);
  }
  // Lese MP135 Sensorwert
  int mp135Value = analogRead(MP135_PIN);

  // Lese BMP280 Sensorwerte
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude(1021.00); // Adjust for your local sea level pressure

  // LED Steuerung basierend auf Temperatur und Druck
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

  // Werte auf Serial für Debugging ausgeben
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
  

  // Werte auf dem LCD basierend auf dem Bildschirmstatus anzeigen
  lcd.clear();
  switch (screenState) {
    case 0:
      // Erster Bildschirm: MP135 und Temperatur
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
      break;
    case 1:
      // Zweiter Bildschirm: Druck und Höhe
      lcd.setCursor(0, 0);
      lcd.print("Druck:");
      lcd.print(pressure / 100); // Convert Pa to hPa
      lcd.print("hPa");
      lcd.setCursor(0, 1);
      lcd.print("Hoehe:");
      lcd.print(altitude);
      lcd.print("m");
      break;
    case 2:
      // Dritter Bildschirm: Schrittzähler
      lcd.setCursor(0, 0);
      lcd.print("Schritte:");
      lcd.print(stepCounter);
      break;
  }

  delay(500); // Passe die Verzögerung nach Bedarf an
}

void handleRoot() {
  int mp135Value = analogRead(MP135_PIN);
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  float altitude = bmp.readAltitude(1021.00);

  String html = "<html><head><title>Sensor Data</title></head><body>";
  html += "<h1>Sensor Data</h1>";
  html += "<p>MP135 Value: " + String(mp135Value) + "</p>";
  html += "<p>Temperature: " + String(temperature) + " °C</p>";
  html += "<p>Pressure: " + String(pressure / 100) + " hPa</p>";
  html += "<p>Altitude: " + String(altitude) + " m</p>";
  html += "<p>Steps: " + String(stepCounter) + "</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}
