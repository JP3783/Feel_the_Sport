#include "config.h"
#include <LilyGoWatch.h>
#include <Wire.h>

TTGOClass *ttgo = TTGOClass::getWatch();

Accel accelData; //Accel struct

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
    // Serial.println(sizeof(accelData.x));
    //Convert raw int values to g
    float ax = accelData.x * 0.00098;
    float ay = accelData.y * 0.00098;
    float az = accelData.z * 0.00098;

    Serial.print("X: ");
    Serial.print(ax, 3);
    Serial.print("  Y: ");
    Serial.print(ay, 3);
    Serial.print("  Z: ");
    Serial.println(az, 3);
  } else {
    Serial.println("Failed to read acceleration");
  }

  delay(100);
}
