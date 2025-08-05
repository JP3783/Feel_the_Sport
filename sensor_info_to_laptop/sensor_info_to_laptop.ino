#include <Arduino.h>
#include <TFT_eSPI.h>          //Display
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <Wire.h>
#include "pcf8563.h"

// === Display setup ===
TFT_eSPI tft = TFT_eSPI();
PCF8563_Class rtc;

// === Bluetooth setup ===
BLEServer *pServer;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID        "3303d3bf-70fd-4d26-9bbc-64be046d6cac"
#define CHARACTERISTIC_UUID "b4739761-a7cd-463e-9bdd-b803057f928b"

// === Bluetooth Callbacks ===
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { deviceConnected = true; }
  void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.print("Received time string: ");
      Serial.println(rxValue.c_str());

      // Expected format: YYYY-MM-DD HH:MM:SS
      int year, month, day, hour, minute, second;
      if (sscanf(rxValue.c_str(), "%d-%d-%d %d:%d:%d",
                 &year, &month, &day, &hour, &minute, &second) == 6) {
        rtc.setDateTime(year, month, day, hour, minute, second);
        Serial.println("RTC updated!");
      }
    }
  }
};

void setup() {
  Serial.begin(115200);

  // === Init Display ===
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);

  // === Init RTC ===
  Wire.begin();
  rtc.begin();

  // === Init Bluetooth ===
  BLEDevice::init("T-Watch Clock");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("Bluetooth Clock Ready. Waiting for PC sync...");
}

void loop() {
  // === Display current RTC time ===
  RTC_Date now = rtc.getDateTime();

  char buf[32];
  sprintf(buf, "%02d:%02d:%02d", now.hour, now.minute, now.second);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(40, 100);
  tft.setTextSize(4);
  tft.println(buf);

  delay(1000);
}


// TFT)eSPI tft = TFT_eSPI();
// PCF8563_Class rtc;

// BLEServer *pServer;
// BLECharacteristic *pCharacteristic;
// bool deviceConnected = false;

// #define SERVICE_UUID "3303d3bf-70fd-4d26-9bbc-64be046d6cac"
// #define CHARACTERISTIC_UUID "b4739761-a7cd-463e-9bdd-b803057f928b"

// class MyServerCallbacks: public BLEServerCallbacks {
//   void onConnect(BLEServer* pServer) { deviceConnected = true; }
//   void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
// };

// class MyCallbacks: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharacteristic){
//     std::string rxValue = pCharacteristic->getValue();

//     if(rxValue.length() > 0){
//       Serial.print("Received time string: ");
//       Serial.println(rxValue.c_str());

//       int year, motnh, day, hour, minute, seconds;
//       if(sscanf(rxValue.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6){
//         rtc.setDate(day, month, year);
//         rtc.setTime(hour, minute, second);
//         Serial.println("RTC updated!");
//       }
//     }
//   }
// };

// void setup() {
//   Serial.begin(115200);

//   tft.init();
//   tft.setRotation(1);
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextColor(TFT_GREEN, TFT_BLACK);
//   tft.setTextSize(2);

//   Wire.begin();
//   rtc.begin();

//   BLEDevice::init("T-Watch Clock");
//   pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());

//   BLEService *pService = pServer->createService(SERVICE_UUID);
//   pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic:: PROPERTY_WRITE);
//   pCharacteristic->setCallbacks(new MyCallbacks());
//   pService->start();

//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->start();

//   Serial.println("Bluetooth Clock Ready. Waiting for PC to sync...");
// }

// void loop() {
//   RTC_Date now = rtc.getDateTime();

//   char buf[32];
//   sprintf(buf, "%02d:%02d:%02d", now.hour, now.minute. now.second);

//   tft.fillScreen(TFT_BLACK);
//   tft.setCursor(40, 100);
//   tft.setTextSize(4);
//   tft.println(buf);

//   delay(1000);
// }
