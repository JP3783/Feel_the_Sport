#include "config.h"
#include <stdio.h>
#include <string.h>
#include "BluetoothSerial.h"


TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power;
BluetoothSerial SerialBT;

int totalFiles = 5;
int selectedFile = 0;
char sportFileNames[5][19] = { "File: Tennis", "File: Cricket", "File: Rugby", "File: Gymnastics", "File: Football"};
char sportFileContents[5][1000] = {"11650,50,High;11800,50,Low;18900,50,High;20150,50,High;21200,50,High;22850,50,High;50100,50,High;50900,50,High;52600,50,High;53500,50,High;60500,50,Low;61500,50,Low;63700,50,Low;64950,50,Low;70750,50,High;80250,50,High;81400,50,High;82600,50,High;84000,50,High;85300,50,High;86650,50,High;87900,50,High;88900,50,High;90250,50,High;110250,50,High;110950,50,High;112600,50,High;114400,50,High;115800,50,High;117100,50,High;118000,50,High", 
                                    "2850,50,Low;3400,50,High;17800,100,Low;19300,100,High;27300,100,Low;28800,100,High;37350,50,Low;38000,50,High;38850,500,Low;56550,100,Low;57800,100,High;59900,1000,Low;77000,50,Low;77500,50,High;78600,100,High;86600,100,Low;88300,100,High;90300,100,High;95400,50,High;110700,50,Low;111450,50,High", 
                                    "3300,3000,Low;6500,50,Low;6900,2500,High;26100,300,High;27100,2000,Low;47500,50,Low;100400,500,Low;101400,50,Low;103200,100,High;104400,500,High;104900,100,Low;105400,100,High",
                                    "2650,100,Low;5000,1000,High;7600,100,Low;9300,1000,Low;11500,1000,Low;13500,1000,Low;15700,1000,Low;18000,1000,Low;19900,1000,Low;21900,1000,Low;23600,1000,Low;25000,1000,Low;27000,500,High;29000,1000,Low;31300,1000,Low;33000,1000,Low;35050,1000,Low;37050,1000,Low;38800,500,High;40800,1000,Low;42550,1000,Low;44750,100,Low;46000,1000,Low;47500,1000,High;49300,100,High",
                                    "19950,50,Low;20650,5000,High;38400,50,Low;39100,1000,High;44900,100,Low;47600,300,High;56600,50,Low;61600,200,High;78800,300,High;84900,1000,High;95700,50,Low;111700,500,Low;130450,20,Low;131050,5000,High"};

bool hasStarted = false;

bool irq = false;

//NEW constants
const int VIBRATION_PIN = 4;
const int TOP = 0;
const int CENTRE_FIRST = 100;
const int CENTRE_SECOND = 140;
const int BOTTOM = 200;

void showBatteryStatus(){
  // You can use isBatteryConnect() to check whether the battery is connected properly
  if (power->isBatteryConnect()) {
      // To display the charging status, you must first discharge the battery,
      // and it is impossible to read the full charge when it is fully charged
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
  int currMs = 0;
  while (currMs < ms) {
    digitalWrite(VIBRATION_PIN, HIGH);
    delay(20);
    digitalWrite(VIBRATION_PIN, LOW);
    delay(230);
    currMs += 250;
  }
}

void buzzManualHigh(int ms) {
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(ms);
  digitalWrite(VIBRATION_PIN, LOW);
}

void updateDisplayedFile() {
  tft->setCursor(0, CENTRE_FIRST);
  tft->println("                   ");
  tft->setCursor(0, CENTRE_FIRST);
  tft->println(sportFileNames[selectedFile]);
}

//Updates the screen countdown timer during the pre-session countdown
void updateDisplay(int seconds) {
  tft->setCursor(0, BOTTOM);
  if (seconds < 5) {
    tft->printf("Countdown: %d       \n", 5 - seconds);
  } else {
    tft->println("Go!             ");
  }
}

//Updates the screen after the haptic program ends
void updateDisplayEnd() {
  tft->setCursor(0, BOTTOM);
  tft->println("End             ");
  delay(1000);
  showBatteryStatus();
  updateDisplayedFile();
  tft->setCursor(0, CENTRE_SECOND);
  tft->println("Touch to start ...   ");
  hasStarted = false;
}

//Parses a serial command in the format "<duration>,<Low/High>" and vibrates accordingly
void handleSerialCommand(String cmd) {
  int commaIndex = cmd.indexOf(',');
  if (commaIndex != -1) {
    int duration = cmd.substring(0, commaIndex).toInt();
    String strength = cmd.substring(commaIndex + 1);
    if (strength.equalsIgnoreCase("Low")) {
      buzzManualLow(duration);
    } else if (strength.equalsIgnoreCase("High")) {
      buzzManualHigh(duration);
    } else {
      Serial.println("Invalid strength. Use 'Low' or 'High'.");
    }
  } else {
    Serial.println("Invalid format. Use: <duration>,<Low/High>");
  }
}

void startProgram() {
  tft->setCursor(0, CENTRE_FIRST); tft->println("                   ");
  tft->setCursor(0, CENTRE_SECOND); tft->println("                   ");
  Serial.println("Starting Countdown ...");

  unsigned long startTime = millis();
  int lastSec = -1;

  while (millis() - startTime < 5000) {
    int sec = (millis() - startTime) / 1000;
    if (sec != lastSec) {
      updateDisplay(sec);
      lastSec = sec;
    }
    delay(10);
  }

  Serial.println("Starting Program ...");

  char *inputCopy = strdup(sportFileContents[selectedFile]);
  char *context, *line = strtok_r(inputCopy, ";", &context);
  unsigned long programStart = millis();

  while (line) {
    char *subctx;
    int triggerTime = atoi(strtok_r(line, ",", &subctx));
    int duration = atoi(strtok_r(nullptr, ",", &subctx));
    char *strength = strtok_r(nullptr, ",", &subctx);

    while (millis() - programStart < triggerTime) delay(5);

    Serial.print("BUZZ ");
    Serial.println(strength);

    if (strcasecmp(strength, "Low") == 0)
      buzzManualLow(duration);
    else if (strcasecmp(strength, "High") == 0)
      buzzManualHigh(duration);

    line = strtok_r(nullptr, ";", &context);
  }

  free(inputCopy);
  updateDisplayEnd();
}

//Initializes the T-Watch, screen, motor, power management, and bluetooth.
void setup() {
  Serial.begin(9600);
  SerialBT.begin("FeelTheSportWatch");

  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();

  tft = ttgo->tft;
  power = ttgo->power;
  power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1, true);
  ttgo->motor_begin();

  pinMode(VIBRATION_PIN, OUTPUT);
  pinMode(TP_INT, INPUT);
  pinMode(AXP202_INT, INPUT_PULLUP);
  attachInterrupt(AXP202_INT, [] { irq = true; }, FALLING);
  power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);
  power->clearIRQ();

  tft->fillScreen(TFT_BLACK);
  tft->setTextFont(2);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setCursor(0, TOP);
  tft->println("\"Feel the Sport\"");
  tft->println("Haptics Program");

  updateDisplayedFile();
  tft->setCursor(0, CENTRE_SECOND);
  tft->println("Touch to start ...   ");
  showBatteryStatus();
}

//MAIN loop: handles bluetooth/serial input, file selection, and touch interaction to start the program
void loop() {
  if (SerialBT.available()) {
    String inputBT = SerialBT.readStringUntil('\n');
    inputBT.trim();
    handleSerialCommand(inputBT);
  }

  if (Serial.available()) {
    String inputUSB = Serial.readStringUntil('\n');
    inputUSB.trim();
    handleSerialCommand(inputUSB);
  }

  if (irq) {
    irq = false;
    power->readIRQ();
    if (power->isPEKShortPressIRQ()) {
      selectedFile = (selectedFile + 1) % totalFiles;
      updateDisplayedFile();
    }
    power->clearIRQ();
  }

  if (digitalRead(TP_INT) == LOW && !hasStarted) {
    hasStarted = true;
    startProgram();
  }

  delay(10);
}

// void handleSerialCommand(String command);

// void setup()
// {
//     // Set up logging
//     Serial.begin(9600);

//     SerialBT.begin("FeelTheSportWatch");
//     tft->println("Bluetooth Ready");

//     // Set up watch
//     ttgo = TTGOClass::getWatch();
//     ttgo->begin();
//     ttgo->openBL();

//     // Set up screen
//     tft = ttgo->tft;

//     // Set up battery 
//     power = ttgo->power;
//     power->adc1Enable(
//         AXP202_VBUS_VOL_ADC1 |
//         AXP202_VBUS_CUR_ADC1 |
//         AXP202_BATT_CUR_ADC1 |
//         AXP202_BATT_VOL_ADC1,
//         true);

//     // Set up vibration motor attach to 33 pin
//     ttgo->motor_begin();

//     tft->fillScreen(TFT_BLACK);
//     tft->setTextFont(2);
//     tft->setTextSize(2);
//     tft->setTextColor(TFT_WHITE, TFT_BLACK);

//     tft->setCursor(0, TOP);
//     tft->println("\"Feel the Sport\"");
//     tft->println("Haptics Program");

//     updateDisplayedFile();

//     tft->setCursor(0, CENTRE_SECOND);
//     tft->println("Touch to start ...   ");

//     showBatteryStatus();

//     // Setup vibration actuator pin
//     pinMode(4, OUTPUT);

//     // Setup touch screen pin
//     pinMode(TP_INT, INPUT);

//     // Setup physical button press pin
//     pinMode(AXP202_INT, INPUT_PULLUP);
//     attachInterrupt(AXP202_INT, [] {
//         irq = true;
//     }, FALLING);

//     //!Clear IRQ unprocessed  first
//     ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);
//     ttgo->power->clearIRQ();

// }

// void loop()
// {
//   if(SerialBT.available()){
//     String command = SerialBT.readStringUntil('\n');
//     command.trim();  // Clean whitespace/newlines
//     handleSerialCommand(command);  // Run your existing command handler
//   }

//   if (irq) {
//       irq = false;
//       ttgo->power->readIRQ();
//       if (ttgo->power->isPEKShortPressIRQ()) {
//           selectedFile = selectedFile + 1;
//           if(selectedFile == totalFiles) {
//             selectedFile = 0;
//           }
//           updateDisplayedFile();
//       }
//       ttgo->power->clearIRQ();
//   }

//   if (digitalRead(TP_INT) == LOW) {
//     if(hasStarted == false){
//       hasStarted = true;
//       startProgram();
//     }
//   }
  
//   if (Serial.available()){
//     String command = Serial.readStringUntil('\n');
//     command.trim();
//     handleSerialCommand(command);
//   }

//   delay(10);
// }

// void updateDisplayedFile(){
//   tft->setCursor(0, CENTRE_FIRST);
//   tft->println("                   ");
//   tft->setCursor(0, CENTRE_FIRST);
//   tft->println(sportFileNames[selectedFile]);
// }

// void updateDisplay(int _secs){
//   if(_secs < 5){
//     int countdown = 5 - _secs;
//     tft->setCursor(0, BOTTOM);
//     tft->print("Countdown: "); tft->print(countdown); tft->println("       "); 
//   } else {
//     tft->setCursor(0, BOTTOM);
//     tft->println("Go!             ");
//   }
// }

// void updateDisplayEnd(){
//   tft->setCursor(0, BOTTOM);
//   tft->println("End             ");
//   delay(1000);
//   showBatteryStatus();
//   updateDisplayedFile();
//   tft->setCursor(0, CENTRE_SECOND);
//   tft->println("Touch to start ...   ");
//   hasStarted = false;
// }

// void buzzManualSolid(int ms) {
//   digitalWrite(4, HIGH);
//   delay(ms);
//   digitalWrite(4, LOW);
// }

// void buzzManualLow(int ms) {

//   int currMs = 0;
//   int runForMs = 20;
//   int pauseForMs = 230;

//   while(currMs < ms){
//     digitalWrite(4, HIGH);
//     delay(runForMs);
//     digitalWrite(4, LOW);
//     delay(pauseForMs);
//     currMs = currMs + runForMs + pauseForMs;
//   }
// }

// void buzzManualHigh(int ms) {
//   digitalWrite(4, HIGH);
//   delay(ms);
//   digitalWrite(4, LOW);
// }


// void startProgram() {

//   tft->setCursor(0, CENTRE_FIRST);
//   tft->println("                   ");
//   tft->setCursor(0, CENTRE_SECOND);
//   tft->println("                   ");

//   Serial.print("Starting Countdown ...\n");

//   unsigned long _msAtStart = millis();

//   int _lastCheckSecs = -1;
//   int _lastCheckMs = -1;

//   while(true){
//     unsigned long _msAtNow = millis();
//     unsigned long _msSinceStart = _msAtNow - _msAtStart;

//     float _secsSinceStart = float(_msSinceStart) / 1000.00;

//     if(_msSinceStart % 1000 == 0 && _lastCheckSecs != _secsSinceStart){
//       updateDisplay(_secsSinceStart);
//     }

//     if(_secsSinceStart == 5){
//         break;
//     }
//   }

//   Serial.print("Starting Program ...\n");

//   // Copying the input string before working with it, 
//   // so that it is intact and the program can be run again
//   char *stringInputCopy = (char *)malloc(strlen(sportFileContents[selectedFile]) + 1);
//   strcpy(stringInputCopy, sportFileContents[selectedFile]);

//   // Reset time
//    _msAtStart = millis();
//   _lastCheckSecs = -1;
//   _lastCheckMs = -1;
 
//   char *pStringInput = stringInputCopy;
//   char *line;
//   char *nextMs;
//   char *nextDuration;
//   char *nextStrength;
//   bool activated = false;

//   while(true){
//     unsigned long _msAtNow = millis();
//     unsigned long _msSinceStart = _msAtNow - _msAtStart;

//     float _secsSinceStart = float(_msSinceStart) / 1000.00;

//     if(_msSinceStart % 50 == 0 && _lastCheckMs != _msSinceStart){ // check every 50 ms

//       if(_msSinceStart % 1000 == 0 && _lastCheckSecs != _secsSinceStart){
//         Serial.println(_secsSinceStart);
//       }

//       // Get next instruction
//       if(activated == false){
//         line = strtok_r(pStringInput, ";", &pStringInput); // delimiter is the semicolon
//         if(line != NULL){
//           // Extract column values
//           char *subp = line;
//           nextMs = strtok_r(subp, ",", &subp);
//           nextDuration = strtok_r(subp, ",", &subp);
//           nextStrength = strtok_r(subp, ",", &subp);
//           activated = true;
//         } else {
//           Serial.print("Program Finished ...\n");
//           free(stringInputCopy);
//           stringInputCopy = NULL;
//           updateDisplayEnd();
//           return;
//         }
//       }

//       // Perform instruction
//       if(atoi(nextMs) == _msSinceStart){
//         if(strcmp(nextStrength, "Low") == 0 || strcmp(nextStrength, "low") == 0){
//           Serial.println("BUZZ low");
//           buzzManualLow(atoi(nextDuration));
//         } else if (strcmp(nextStrength, "High") == 0 || strcmp(nextStrength, "high") == 0){
//           Serial.println("BUZZ high");
//           buzzManualHigh(atoi(nextDuration));
//         } else {
//           Serial.println("NO buzz");
//         }
//         activated =  false;
//       }      
//     }

//     _lastCheckSecs = _secsSinceStart;
//     _lastCheckMs = _msSinceStart;
//   }
// }

// void handleSerialCommand(String cmd){
//     //Example format: 100,High
//     int commaIndex = cmd.indexOf(',');
//     if (commaIndex != -1) {
//         String durationStr = cmd.substring(0, commaIndex);
//         String strengthStr = cmd.substring(commaIndex + 1);

//         int duration = durationStr.toInt();

//         if (strengthStr.equalsIgnoreCase("Low")) {
//             buzzManualLow(duration);
//         } else if (strengthStr.equalsIgnoreCase("High")) {
//             buzzManualHigh(duration);
//         } else {
//             Serial.println("Invalid strength. Use 'Low' or 'High'.");
//         }
//     } else {
//         Serial.println("Invalid format. Use: <duration>,<Low/High>");
//     }
// }
