#include "config.h"
#include <LilyGoWatch.h>
#include <Wire.h>
#include <math.h>

TTGOClass *ttgo = TTGOClass::getWatch();
Accel accelData;

unsigned long lastHitTime = 0;
const float hitThreshold = 2.0;       //Hit threshold in g
const int hitCooldownMs = 300;        //Cooldown between hits

void setup() {
  Serial.begin(115200);
  while (!Serial);

  ttgo->begin();
  ttgo->bma->begin();
  ttgo->bma->enableAccel();

  Serial.println("Tennis hit detection started...");
}

void loop() {
  if (ttgo->bma->getAccel(accelData)) {
    //Convert to g
    float ax = accelData.x * 0.00098;
    float ay = accelData.y * 0.00098;
    float az = accelData.z * 0.00098;

    //Calculate total acceleration magnitude
    float mag = sqrt(ax * ax + ay * ay + az * az);

    //Check if it's a hit
    unsigned long now = millis();
    //If magnitude is greater than the threshold AND 300 milliseconds have passed since the last hit
    if (mag > hitThreshold && (now - lastHitTime > hitCooldownMs)) {
      lastHitTime = now;
      Serial.print("Hit detected! Mag: ");
      Serial.println(mag, 3);
    }
  }

  delay(10);  //~100 Hz sampling
}
