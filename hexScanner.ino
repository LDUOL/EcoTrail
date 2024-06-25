#include <Wire.h>

void setup() {
  Wire.begin(D2, D1); // I2C Pins für ESP8266
  Serial.begin(115200);
  while (!Serial); // Warten auf serielle Schnittstelle

  Serial.println("\nI2C Scanner");

  byte count = 0;

  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at address 0x");
      if (i < 16) Serial.print("0");
      Serial.print(i, HEX);
      Serial.println("  !");
      count++;
    }
  }
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println(" device(s).");
}

void loop() {
  // Nichts zu tun hier
}
