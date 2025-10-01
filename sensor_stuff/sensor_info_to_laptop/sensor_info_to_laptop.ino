#include "config.h"
#include <LilyGoWatch.h>
#include <Wire.h>
#include <math.h>
#include <BluetoothSerial.h>
#include <stdio.h>
#include <string.h>

TTGOClass *ttgo = TTGOClass::getWatch();
TFT_eSPI *tft;
Accel accelData;
BluetoothSerial SerialBT;
AXP20X_Class *power;
char buf[128];

unsigned long lastHitTime = 0;
const float hitThreshold = 2.0;       //Hit threshold in g
const int hitCooldownMs = 300;        //Cooldown between hits

void showBatteryStatus(){
  tft->fillRect(0, 200, 240, 20, TFT_BLACK); //Clear the bottom area
  if(power->isBatteryConnect()){
    if(power->isChargeing()){
      tft->setCursor(0, 200);
      tft->println("BAT: charging        ");
    } else{
      tft->setCursor(0, 200);
      tft->print("BAT: "); tft->print(power->getBattPercentage()); tft->println("%");
    }
  } else{
    tft->setCursor(0, 200);
    tft->println("BAT: disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  
  while (!Serial);

  //Initialize screen and power
  // ttgo = TTGOClass::getWatch(); already did it at the top
  ttgo->begin();
  ttgo->openBL(); //Turn on backlight

  tft = ttgo->tft;
  power = ttgo->power;

  //Initialise accelerometer
  ttgo->bma->begin();
  ttgo->bma->enableAccel();

  Serial.println("Tennis hit detection started...");

  //Start Bluetooth with this device name
  SerialBT.begin("Sensor_Watch");
  Serial.println("Bluetooth started. FeeltheSportWatch initialised.");

  tft->setTextFont(2);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setCursor(0, 0);
  tft->fillRect(0, 0, 240, 20, TFT_BLACK); //Clear the top area
  tft->println("Feel the Sport");
  showBatteryStatus();

  tft->setCursor(0, 30);
  tft->println("Sensor");
  

  tft->setTextFont(1);
  tft->setTextSize(2);
  tft->setCursor(0, 150);
  tft->println("Transmitting data...");
}

void loop() {
  // const char* baseText = "Transmitting data";
  // //Animate the dots
  // for (int dots = 0; dots <= 3; dots++) {
  //   tft->fillRect(0, 150, 320, 20, TFT_BLACK);
  //   // tft->fillRect(0, 0, 240, 20, TFT_BLACK); //Clear the top area
  //   tft->setCursor(0, 150);
  //   tft->print(baseText);
  //   for (int i = 0; i < dots; i++) tft->print(".");
  //   // delay(500);
  // }

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

    //Send raw data
    Serial.println(rawData);
    SerialBT.println(rawData);

    //Check if it's a hit
    unsigned long now = millis();
    //If magnitude is greater than the threshold AND 300 milliseconds have passed since the last hit
    if (mag > hitThreshold && (now - lastHitTime > hitCooldownMs)) {
      lastHitTime = now;
      String hitMsg = "Hit detected! Mag: " + String(mag, 3);
      Serial.println(hitMsg);         //Debug log
      SerialBT.println(hitMsg);       //Send to laptop via Bluetooth

      Serial.print("Hit detected! Mag: ");
      Serial.println(mag, 3);
    }
  }

  delay(10);  //~100 Hz sampling
}
