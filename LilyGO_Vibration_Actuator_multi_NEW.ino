#include "config.h"
#include <stdio.h>
#include <string.h>
#include <BluetoothSerial.h>

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;
BluetoothSerial SerialBT;
const int VIBRATION_PIN = 4;
const int TOP = 0;
const int CENTRE_FIRST = 100;
const int CENTRE_SECOND = 140;
const int BOTTOM = 200;

//Stickman constants
int stickmanX = 0;
unsigned long lastFrameTime = 0;
bool walkingPaused = false;
unsigned long pauseStartTime = 0;
bool stepToggle = false;  //toggles to animate steps

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

void drawStickman(int x, int y, uint16_t color, bool step) {
  //Clear a bounding box for redrawing
  tft->fillRect(x - 15, y - 10, 40, 70, TFT_BLACK);

  //Head (radius 10)
  tft->drawCircle(x, y, 10, color);

  //Body
  tft->drawLine(x, y + 10, x, y + 40, color);

  //Arms (horizontal)
  tft->drawLine(x - 10, y + 20, x + 10, y + 20, color);

  //Legs: alternate for walking effect
  if (step) {
    // Step position A
    tft->drawLine(x, y + 40, x - 8, y + 60, color); //left leg
    tft->drawLine(x, y + 40, x + 8, y + 50, color); //right leg slightly bent
  } else {
    //Step position B (swap legs)
    tft->drawLine(x, y + 40, x - 8, y + 50, color);
    tft->drawLine(x, y + 40, x + 8, y + 60, color);
  }
}

void updateStickman() {
  unsigned long now = millis();

  if (walkingPaused) {
    if (now - pauseStartTime >= 2000) {
      stickmanX = -20;
      walkingPaused = false;
    }
    return;
  }

  if (now - lastFrameTime >= 100) {
    stickmanX += 6;
    stepToggle = !stepToggle;

    if (stickmanX > 240 + 20) {
      walkingPaused = true;
      pauseStartTime = now;
      return;
    }

    drawStickman(stickmanX, CENTRE_SECOND, TFT_WHITE, stepToggle);
    lastFrameTime = now;
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("FeelTheSportWatch");
  pinMode(VIBRATION_PIN, OUTPUT);

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

  Serial.println("Bluetooth ready. Waiting for command...");
  showBatteryStatus();
}

void loop() {
  updateStickman();

  //Read from Bluetooth
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    Serial.print("BT Received: ");
    Serial.println(cmd);

    if (cmd.startsWith("TIMESTAMP:")) {
      String timePart = cmd.substring(10);
      unsigned long watchMillis = millis();
      //Reply with laptop's timestamp and watch millis
      String response = "LAPTOP:" + timePart + ",WATCH:" + String(watchMillis);
      SerialBT.println(response);
    }

    int commaIndex = cmd.indexOf(',');
    if (commaIndex != -1) {
      int duration = cmd.substring(0, commaIndex).toInt();
      String strength = cmd.substring(commaIndex + 1);
      strength.trim();

      if (strength.equalsIgnoreCase("Low")) {
        buzzManualLow(duration);
      } else if (strength.equalsIgnoreCase("High")) {
        buzzManualHigh(duration);
      } else {
        Serial.println("Invalid strength. Use Low/High.");
      }
      showBatteryStatus(); //Refresh battery status
    } else {
      Serial.println("Invalid format. Use <duration>,<Low|High>");
    }
  }

  delay(10); //keep loop responsive
}