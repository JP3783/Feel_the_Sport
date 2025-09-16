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
// bool rtcIrq = false;

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

//Display time and date
void displayTime(){
  snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
  tft->fillRect(0, 0, 240, 40, TFT_BLACK); //Clear previous time area
  tft->setCursor(0, 0);
  tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  tft->println(buf);
}

void syncRTCWithLaptopTime(){
  const char *dateStr = __DATE__;
    const char *timeStr = __TIME__;

    char monthStr[4];
    int year, day, hour, minute, second;
    int month;

    sscanf(dateStr, "%s %d %d", monthStr, &day, &year);
    sscanf(timeStr, "%d:%d:%d", &hour, &minute, &second);

    //Convert month string to number
    const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    month = (strstr(months, monthStr) - months) / 3 + 1;

    //Set RTC
    ttgo->rtc->setDateTime(year, month, day, hour, minute, second);

    Serial.printf("RTC synced to compile time: %04d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
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

  //Call syncwithrtc
  syncRTCWithLaptopTime();

  Serial.println("Tennis hit detection started...");

  //Start Bluetooth with this device name
  SerialBT.begin("FeeltheSportWatch");
  Serial.println("Bluetooth started. FeeltheSportWatch initialised.");

  tft->setTextFont(2);
  tft->setTextSize(2);
  showBatteryStatus();
}

void loop() {
  //Update time display
  static unsigned long lastTimeUpdate = 0;
  if(millis() - lastTimeUpdate > 1000){
    displayTime();
    lastTimeUpdate = millis();
  }


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

  // showBatteryStatus();

  delay(10);  //~100 Hz sampling
}
