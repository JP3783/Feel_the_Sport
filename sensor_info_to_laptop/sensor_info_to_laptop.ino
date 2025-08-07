#include "config.h"
#include <LilyGoWatch.h>
#include <Wire.h>

TTGOClass *ttgo = TTGOClass::getWatch();

Accel accelData;  // Declare accelData here — note the space between 'Accel' and 'accelData'

void setup() {
  Serial.begin(115200);
  while (!Serial);

  ttgo->begin();

  ttgo->bma->begin();
  ttgo->bma->enableAccel();

  Serial.println("BMA423 Accelerometer initialized.");
  Serial.println("Streaming acceleration data...");
}

void loop() {
  if (ttgo->bma->getAccel(accelData)) {
    Serial.print("X: ");
    Serial.print(accelData.x, 3);
    Serial.print("  Y: ");
    Serial.print(accelData.y, 3);
    Serial.print("  Z: ");
    Serial.println(accelData.z, 3);
  } else {
    Serial.println("Failed to read acceleration");
  }

  delay(100);
}
