#include <SI114X.h>
#include <Si115X.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Adresse des LCD-Moduls (ersetzen Sie 0x27 durch die vom I2C-Scanner gefundene Adresse)
#define I2C_ADDR 0x27

LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2);

SI114X SI1145 = SI114X();

void setup() {

  Serial.begin(115200);
  Wire.begin(D2, D1);   // Initialisierung der I2C-Schnittstelle

  // Initialisierung des LCD
  lcd.init(); // Verwenden Sie init() anstelle von begin()
  lcd.backlight();

  delay(100);

  while(!SI1145.Begin()){
    Serial.println("Si1145 is not ready!");
    delay(1000);
  }
  Serial.println("Si1145 is ready!");
}

void loop() {
  write("This is a Test",0,0);
}

void write(String message, int xPos, int yPos) {
  lcd.setCursor(xPos, yPos);
  lcd.print(message);
}

