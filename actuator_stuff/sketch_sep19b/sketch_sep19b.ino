#include "config.h"
#include <BluetoothSerial.h>
#include <stdio.h>
#include <string.h>

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;
BluetoothSerial SerialBT;

const int VIBRATION_PIN = 4;

//LVGL objects
lv_obj_t *batteryLabel;
lv_obj_t *titleLabel;

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        Serial.println("Clicked");
    }
}

void updateBatteryStatus() {
    if (!batteryLabel) return;

    String text = "BAT: ";
    if (power->isBatteryConnect()) {
        if (power->isChargeing()) {
            text += "charging";
        } else {
            text += String(power->getBattPercentage()) + "%";
        }
    } else {
        text = "BAT: disconnected";
    }

    lv_label_set_text(batteryLabel, text.c_str());
}

void setupUI() {
    //Create style for title
    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, LV_STATE_DEFAULT, &lv_font_montserrat_28); // 28px for title

    //Create style for battery
    static lv_style_t style_battery;
    lv_style_init(&style_battery);
    lv_style_set_text_font(&style_battery, LV_STATE_DEFAULT, &lv_font_montserrat_28); // 20px for battery

    //Title label at top
    titleLabel = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(titleLabel, LV_LABEL_PART_MAIN, &style_title);
    lv_label_set_text(titleLabel, "Feel the Sport");
    lv_obj_align(titleLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 10); //top-center

    //Center button
    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn, event_handler);
    lv_obj_align(btn, NULL, LV_ALIGN_CENTER, 0, 0); //center of screen

    lv_obj_t *label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Start");

    //Battery label at bottom
    batteryLabel = lv_label_create(lv_scr_act(), NULL);
    lv_obj_add_style(batteryLabel, LV_LABEL_PART_MAIN, &style_battery);
    lv_obj_align(batteryLabel, NULL, LV_ALIGN_IN_BOTTOM_MID, -80, 0);

    updateBatteryStatus();
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

void buzzManualMed(int ms) {
  int elapsed = 0;
  while (elapsed < ms) {
    digitalWrite(VIBRATION_PIN, HIGH);
    delay(20);
    digitalWrite(VIBRATION_PIN, LOW);
    delay(80);
    elapsed += 100;
  }
}

void buzzManualHigh(int ms) {
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(ms);
  digitalWrite(VIBRATION_PIN, LOW);
}


void setup() {
    Serial.begin(115200);
    SerialBT.begin("Actuator_Watch");
    pinMode(VIBRATION_PIN, OUTPUT);

    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
    ttgo->lvgl_begin();

    tft = ttgo->tft;
    power = ttgo->power;

    //Set background color to white
    tft->fillScreen(TFT_WHITE);

    setupUI();

    Serial.println("Setup complete.");
}

void loop() {
    lv_task_handler();        //Update LVGL
    updateBatteryStatus();    //Update battery info

    //Read from Bluetooth
    if (SerialBT.available()) {
      String cmd = SerialBT.readStringUntil('\n');
      Serial.print("BT Received: ");
      Serial.println(cmd);

      int commaIndex = cmd.indexOf(',');
      if (commaIndex != -1) {
        int duration = cmd.substring(0, commaIndex).toInt();
        String strength = cmd.substring(commaIndex + 1);
        strength.trim();

        if (strength.equalsIgnoreCase("Low")) {
          // SerialBT.println("ACK");
          buzzManualLow(duration);
        } else if (strength.equalsIgnoreCase("High")) {
          // SerialBT.println("ACK");
          buzzManualHigh(duration);
        } else if (strength.equalsIgnoreCase("Med")){
          buzzManualMed(duration);
        } else {
          Serial.println("Invalid strength. Use Low/High.");
        }
        updateBatteryStatus(); //Refresh battery status
      } else {
        Serial.println("Invalid format. Use <duration>,<Low|High>");
      }
    }

    delay(10); //Reduce flicker and save CPU
}
