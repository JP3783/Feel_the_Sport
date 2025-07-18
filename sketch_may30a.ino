#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>


//Nordic UART UUIDs
#define UART_SERVICE_UUID          "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_TX_CHAR_UUID          "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX_CHAR_UUID          "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;
const int VIBRATION_PIN = 4;
const int TOP = 0;
const int CENTRE_FIRST = 100;
const int CENTRE_CENTRE = 120;
const int CENTRE_SECOND = 140;
const int BOTTOM = 200;

BLEScan* pBLEScan;
BLEAdvertisedDevice* myDevice = nullptr;
BLERemoteCharacteristic* pRemoteTXCharacteristic;
BLEClient* pClient = nullptr;

bool deviceFound = false;
bool connected = false;

void buzzManualLow(int ms) {
  int elapsed = 0;
  while (elapsed < ms) {
    digitalWrite(VIBRATION_PIN, HIGH);
    delay(20);
    digitalWrite(VIBRATION_PIN, LOW);
    delay(230);
    elapsed += 250;
  }
}

void buzzManualHigh(int ms) {
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(ms);
  digitalWrite(VIBRATION_PIN, LOW);
}

void showBatteryStatus(){
  tft->fillRect(0, BOTTOM, 240, 20, TFT_BLACK); //Clear the bottom area
  //Check whether the battery is connected properly
  if (power->isBatteryConnect()) {
      //To display the charging status, first discharge the battery,
      //and it is impossible to read the full charge when it is fully charged
      if (power->isChargeing()) {
        tft->setCursor(0, BOTTOM);
        tft->println("BAT: charging        ");
      } else {
        tft->setCursor(0, BOTTOM);
        tft->print("BAT: "); tft->print(power->getBattPercentage()); tft->println("%");    
      }
  } else {
      tft->setCursor(0, BOTTOM);
      tft->println("BAT: disconnected");
  }
}




//Everything above this is screen setup**************

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    Serial.print("Seen device: ");
    Serial.println(advertisedDevice.getName().c_str());

    if(advertisedDevice.getName() == "BBC micro:bit [tavop]"){
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      deviceFound = true;
      pBLEScan->stop();
      
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(UART_SERVICE_UUID))) {
        Serial.println("Found micro:bit UART device!");
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        deviceFound = true;
        pBLEScan->stop();
      }
    }
  }
};

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.print("Received from micro:bit: ");
  // for (size_t i = 0; i < length; i++) {
  //   Serial.print((char)pData[i]);
  // }
  for (int i = 0; i < length; i++){
    Serial.println((char)pData[i]);
  }
  
  Serial.println();
}


bool connectToMicrobit() {
  if (myDevice == nullptr) return false;

  pClient = BLEDevice::createClient();
  Serial.println("Connecting to micro:bit...");
    if (!pClient->connect(myDevice)) {
      Serial.println("Failed to connect.");
      return false;
    }

  BLERemoteService* pRemoteService = pClient->getService(UART_SERVICE_UUID);
    if (pRemoteService == nullptr) {
      Serial.println("Failed to find UART service.");
      pClient->disconnect();
      return false;
    }

  pRemoteTXCharacteristic = pRemoteService->getCharacteristic(UART_TX_CHAR_UUID); //odd situai
    if (pRemoteTXCharacteristic == nullptr) {
      Serial.println("Failed to find TX characteristic.");
      pClient->disconnect();
      return false;
    }

  Serial.print("canNotify() returns: ");
  Serial.println(pRemoteTXCharacteristic->canNotify() ? "true" : "false");

  
  // if (pRemoteTXCharacteristic->canNotify()) {
  //   pRemoteTXCharacteristic->registerForNotify(notifyCallback);

  //   Serial.println("It REACHES HEREXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    
  //   //Enable notifications manually
  //   auto pDescriptor = pRemoteTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902));
  //   if (pDescriptor != nullptr) {
  //     uint8_t notifyOn[] = {0x01, 0x00};
  //     pDescriptor->writeValue(notifyOn, 2, true);
  //     Serial.println("Notifications enabled.");
  //   } else {
  //     Serial.println("Notification descriptor not found.");
  //   }
  // } else {
  //   Serial.println("TX characteristic cannot notify.");
  // }
  //Attempt to register for notifications regardless of canNotify()
  pRemoteTXCharacteristic->registerForNotify(notifyCallback);
  Serial.println("Forcing notify registration...");

  //Try to get the descriptor and manually enable notifications
  BLERemoteDescriptor* pDescriptor = pRemoteTXCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902));
  if (pDescriptor != nullptr) {
    uint8_t notifyOn[] = {0x01, 0x00};
    pDescriptor->writeValue(notifyOn, 2, true);
    Serial.println("Forced notifications enabled via descriptor.");
  } else {
    Serial.println("❗ 0x2902 descriptor not found – can't enable notifications manually.");
  }

  connected = true;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client...");
  BLEDevice::init("");

  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10, false);  //initial scan 10 seconds

  //Initialize screen and power
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL(); //Turn on backlight

  tft = ttgo->tft;
  power = ttgo->power;

  tft->fillScreen(TFT_BLACK);
  tft->setTextFont(2);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE);
  tft->setCursor(30, TOP);

  //Draw title
  tft->println("Feel the Sport");
  showBatteryStatus();
}

void loop() {
  if (!connected) {
    if (!deviceFound) {
      //Clear old devices
      pBLEScan->clearResults();
      Serial.println("Scanning for micro:bit...");
      pBLEScan->start(10, false); //scan for 10 seconds
    } else {
      if (connectToMicrobit()) {
        Serial.println("Connected to micro:bit!");
        //Clear the area where previous messages might have been
        tft->fillRect(0, CENTRE_FIRST, 240, 50, TFT_BLACK);
        //Set cursor to the CENTRE_CENTRE for the "Connected!" message
        tft->setCursor(30, CENTRE_CENTRE);
        tft->println("Connected!");

        Serial.println("Waiting for notifications...");
      } else {
        Serial.println("Connection failed. Restart scanning...");
        deviceFound = false;
      }
    }
  } else {
    //Check connection status
    if (!pClient->isConnected()) {
      Serial.println("Disconnected from micro:bit.");
      tft->fillRect(0, CENTRE_FIRST, 240, 50, TFT_BLACK);
      connected = false;
      deviceFound = false;
      delete myDevice;
      myDevice = nullptr;
    }
  }
 
  delay(100); //small delay
}
