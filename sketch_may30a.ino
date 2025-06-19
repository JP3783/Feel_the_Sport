#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
// #include "config.h"
// #include <stdio.h>
// #include <string.h>
// #include <Arduino.h>


//Nordic UART UUIDs
#define UART_SERVICE_UUID          "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_TX_CHAR_UUID          "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
// #define UART_RX_CHAR_UUID          "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

BLEScan* pBLEScan;
BLEAdvertisedDevice* myDevice = nullptr;
BLERemoteCharacteristic* pRemoteTXCharacteristic = nullptr;
BLEClient* pClient = nullptr;
// TTGOClass *ttgo;


bool deviceFound = false;
bool connected = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    // Serial.print("Seen device: ");
    // Serial.println(advertisedDevice.toString().c_str());

    // //Match by name (if micro:bit is advertising its name)
    // if (advertisedDevice.getName() == "BBC micro:bit") {
    //   Serial.println("Found micro:bit by name!");
    //   myDevice = new BLEAdvertisedDevice(advertisedDevice);
    //   deviceFound = true;
    //   pBLEScan->stop();
    // }
    // Serial.println("test");
    Serial.println("Found micro:bit UART device!");
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
};

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData, size_t length, bool isNotify) {
  
  Serial.print("Received from micro:bit: ");
  for (size_t i = 0; i < length; i++) {
    Serial.print((char)pData[i]);
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

  Serial.println("Does it reach here?");

  pRemoteTXCharacteristic = pRemoteService->getCharacteristic(UART_TX_CHAR_UUID);
  if (pRemoteTXCharacteristic == nullptr) {
    Serial.println("Failed to find TX characteristic.");
    pClient->disconnect();
    return false;
  }

  if (pRemoteTXCharacteristic->canNotify()) {
    pRemoteTXCharacteristic->registerForNotify(notifyCallback);
    Serial.println("Subscribed to micro:bit notifications.");
  }

  connected = true;
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Client...");
  BLEDevice::init("");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10, false);  //initial scan 10 seconds
}

void loop() {
  if (!connected) {
    if (!deviceFound) {
      Serial.println("Scanning for micro:bit...");
      pBLEScan->start(10, false); //scan for 10 seconds
    } else {
      if (connectToMicrobit()) {
        Serial.println("Connected to micro:bit!");
      } else {
        Serial.println("Connection failed. Restart scanning...");
        deviceFound = false;
      }
    }
  } else {
    // Check connection status
    if (!pClient->isConnected()) {
      Serial.println("Disconnected from micro:bit.");
      connected = false;
      deviceFound = false;
      delete myDevice;
      myDevice = nullptr;
    }
  }
  
  delay(100); //small delay
}