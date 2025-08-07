#include "config.h"
#include <LilyGoWatch.h>
#include <Wire.h>
#include <math.h>
#include <BluetoothSerial.h>

TTGOClass *ttgo = TTGOClass::getWatch();
TFT_eSPI *tft;
Accel accelData;
BluetoothSerial SerialBT;
// AXP20X_Class *power;

unsigned long lastHitTime = 0;
const float hitThreshold = 2.0;       //Hit threshold in g
const int hitCooldownMs = 300;        //Cooldown between hits

// void showBatteryStatus(){
//   tft->fillRect(0, 200, 240, 20, TFT_BLACK); //Clear the bottom area
//   if(power->isBatteryConnect()){
//     if(power->isCharging()){
//       tft->setCursor(0, 200);
//     }
//   }
// }

void setup() {
  Serial.begin(115200);
  while (!Serial);

  //Initialise watch and accelerometer
  ttgo->begin();
  ttgo->bma->begin();
  ttgo->bma->enableAccel();

  Serial.println("Tennis hit detection started...");

  //Start Bluetooth with this device name
  SerialBT.begin("T-Watch Sensor");
  Serial.println("Bluetooth started. T-Watch Sensor initialised.");
}

void loop() {
  if (ttgo->bma->getAccel(accelData)) {
    //Convert to g
    float ax = accelData.x * 0.00098;
    float ay = accelData.y * 0.00098;
    float az = accelData.z * 0.00098;

    //Calculate total acceleration magnitude
    float mag = sqrt(ax * ax + ay * ay + az * az);

    //Prepare raw data message
    String rawData = "X: " + String(ax, 3) +
                     "  Y: " + String(ay, 3) +
                     "  Z: " + String(az, 3) +
                     "  Mag: " + String(mag, 3);

    // //Send raw data
    // Serial.println(rawData);
    // SerialBT.println(rawData);

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
