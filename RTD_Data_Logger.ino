// Edit 'Config.h' and then upload this file.
// No need to edit anything in this file

// 15/07/2021
// WTN

// Libraries
#include <SD.h>
#include <SD_t3.h>
#include <TimeLib.h>
#include "Config.h"
#include "Adafruit_MAX31865_Modified.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED configs
#define SCREEN_ADDRESS  0x3C  // Display i2c address
#define OLED_RESET      4     // Reset pin #
#define SCREEN_WIDTH    128   // OLED display width, in pixels
#define SCREEN_HEIGHT   64    // OLED display height, in pixels

#define SERIAL_DEBUG  0

// Input Pin Definitions
#define button1Pin      37    // Button to start/stop sampling
#define button2Pin      35    // Button for scrolling right
#define button3Pin      36    // Button for scrolling left
#define button4Pin      34    // Button for ...
#define button5Pin      33    // Button for ...

// Output Pin Definitions
#define sampleLedPin    22    // LED to illuminate when test is underway
#define SDErrLedPin     23    // LED to illuminate when error with SD card

// RTD to Digital Objects
Adafruit_MAX31865_Modified max_0 = Adafruit_MAX31865_Modified(6);
Adafruit_MAX31865_Modified max_1 = Adafruit_MAX31865_Modified(7);
Adafruit_MAX31865_Modified max_2 = Adafruit_MAX31865_Modified(8);
Adafruit_MAX31865_Modified max_3 = Adafruit_MAX31865_Modified(9);
Adafruit_MAX31865_Modified max_4 = Adafruit_MAX31865_Modified(10);
Adafruit_MAX31865_Modified max_5 = Adafruit_MAX31865_Modified(1);
Adafruit_MAX31865_Modified max_6 = Adafruit_MAX31865_Modified(2);
Adafruit_MAX31865_Modified max_7 = Adafruit_MAX31865_Modified(3);
Adafruit_MAX31865_Modified max_8 = Adafruit_MAX31865_Modified(4);
Adafruit_MAX31865_Modified max_9 = Adafruit_MAX31865_Modified(5);

// Display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // 128x64 OLED

// RTD Definitions
#define RREF      430.0             // The value of the Rref resistor. Use 430.0 for PT100
#define RNOMINAL  100.0             // The 'nominal' 0-degrees-C resistance of the sensor

#define chipSelect  BUILTIN_SDCARD  // Chip select definition for built in SD card
#define TIME_HEADER "T"             // Header tag for serial time sync message

// Create logfile object
File logfile;

void setup() {

  setSyncProvider(getTeensy3Time);  // Set the Time library to use Teensy 3.0's RTC to keep time
//  Serial.begin(115200);             // Used for real time monitoring over USB

  // init display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setRotation(2);
  display.display();

  // clear display
  display.clearDisplay();
  display.display();

  // Pin configurations
  pinMode(sampleLedPin, OUTPUT);
  pinMode(SDErrLedPin, OUTPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  pinMode(button4Pin, INPUT);
  pinMode(button5Pin, INPUT);

  // Initialise MAX31865
  max_0.begin(CH1_WIRE_CONFIG);
  max_1.begin(CH2_WIRE_CONFIG);
  max_2.begin(CH3_WIRE_CONFIG);
  max_3.begin(CH4_WIRE_CONFIG);
  max_4.begin(CH5_WIRE_CONFIG);
  max_5.begin(CH6_WIRE_CONFIG);
  max_6.begin(CH7_WIRE_CONFIG);
  max_7.begin(CH8_WIRE_CONFIG);
  max_8.begin(CH9_WIRE_CONFIG);
  max_9.begin(CH10_WIRE_CONFIG);

  // Take dummy sample
  float temp;
  temp = max_0.temperature(RNOMINAL, RREF);
  temp = max_1.temperature(RNOMINAL, RREF);
  temp = max_2.temperature(RNOMINAL, RREF);
  temp = max_3.temperature(RNOMINAL, RREF);
  temp = max_4.temperature(RNOMINAL, RREF);
  temp = max_5.temperature(RNOMINAL, RREF);
  temp = max_6.temperature(RNOMINAL, RREF);
  temp = max_7.temperature(RNOMINAL, RREF);
  temp = max_8.temperature(RNOMINAL, RREF);
  temp = max_9.temperature(RNOMINAL, RREF);

  SdFile::dateTimeCallback(dateTime);
}

void loop() {

  // 3 cases for SD card error:
  // No SPI connection            (0b100)
  // No unique filename available (0b010)
  // Failed to create file on SD  (0b001)
  byte error = 0b000;


  // variables
  bool samplingState = 0;
  bool samplingStateOld = 0;
  bool button1State = 0;
  bool button1StateOld = 0;
  bool button2State = 0;
  bool button2StateOld = 0;
  bool button3State = 0;
  bool button3StateOld = 0;
  long timeOld = -1000000;
  unsigned long samplePeriod = 1000 / samplingFrequency;
  float dt = 0;
  unsigned long startTime = 0;
  unsigned long sampleCounter = 0;
  int screenCounter = 10; // start on screen which shows all temperatures. 0-9 show channels 1-10 individually

  // Filename for SD file, new unique numbers get automatically generated
  char filename[] = "LOG00.csv";

  // Array for temperature measurements
  float T[10] = {0};

  while (1) {

    // Check if SD card present, and light error LED accordingly
    // Only check for SD error before sampling - during sampling throws timings off?
    if (!samplingState) {
      errorCheckSDPresent(&error, SD);
//      errorDisplay(&error);
    }

    // Check if button has been pressed
    button1State = !digitalRead(button1Pin);
    button2State = !digitalRead(button2Pin);
    button3State = !digitalRead(button3Pin);

    // If button 1 pressed and wasn't previously, debounce
    if (button1State && !button1StateOld) {
      delay(5);
      button1State = !digitalRead(button1Pin);
      // If button 1 still pressed, accept valid button press
      if (button1State) {
        samplingState = !samplingState;
        button1StateOld = 1;
      }
    }else if (!button1State) {
      button1StateOld = 0;
    }

    // If button 2 pressed and wasn't previously, increment screen counter
    if (button2State && !button2StateOld) {
      delay(5);
      button2State = !digitalRead(button2Pin);
      // If button 2 still pressed, accept valid button press
      if (button2State) {
        screenCounter++;
        button2StateOld = 1;
      }
    }else if (!button2State) {
      button2StateOld = 0;
    }

    // If button 3 pressed and wasn't previously, decrement screen counter
    if (button3State && !button3StateOld) {
      delay(5);
      button3State = !digitalRead(button3Pin);
      // If button 1 still pressed, accept valid button press
      if (button3State) {
        screenCounter--;
        button3StateOld = 1;
      }
    }else if (!button3State) {
      button3StateOld = 0;
    }

    // wrap around screens if counter > 10 or < 0
    if(screenCounter > 10){
      screenCounter = 0;
    }else if(screenCounter < 0){
      screenCounter = 10;
    }

   if (sampleCounter > testLength * samplingFrequency) {
     samplingState = 0;
   }

    // If sampling is true, log to sd card
    if (samplingState) {

      // If first time through loop, generate unique file
      if (!samplingStateOld) {
        samplingStateOld = 1;
        // Turn sample LED on
        digitalWrite(sampleLedPin, HIGH);
        // Generate unique filename and update error
        if (!error) {
          generateFilename(filename, &filename[3], &filename[4], &error);
        }
        // Open file on SD
        if (!error) {
          logfile = SD.open(filename, FILE_WRITE);
        }
        if (!error && !logfile) {
          // Failed to create file error
          error = error | 0b001;
        } else {
          // No error, file created
          error = error & 0b110;
        }

        if (!error) {
          // Generate header with time stamp - SD
          logfile.println("10 Channel PT100 Data Logger");
          logfile.println(String(day()) + "/" + String(month()) + "/" + String(year()));
          logfile.printf("%02d:%02d:%02d\n", hour(), minute(), second());
          logfile.println();
          logfile.println();
          if (unit == 'K' || unit == 'k') {
            logfile.println("Time (s), T1 (K), T2 (K),  T3 (K), T4 (K), T5 (K), T6 (K), T7 (K), T8 (K), T9 (K), T10 (K)");
          } else {
            logfile.println("Time (s), T1 (C), T2 (C),  T3 (C), T4 (C), T5 (C), T6 (C), T7 (C), T8 (C), T9 (C), T10 (C)");
          }
          logfile.flush();
        }
        startTime = millis();
      } // end of first pass

      // If no error, write to SD
      if (!error) {
        logfile.printf("%.2f,", dt);
        for (byte i = 0; i < 10; i++) {
          if (i == 9) {
            // If last temperature, print new line
            if ((T[i] == -999) || ((T[i] > 1213.8) && (T[i] < 1214.0))) {
              logfile.println('-');
            } else {
              logfile.println(T[i]);
            }
          } else {
            // Else print comma separated on single line
            if ((T[i] == -999) || ((T[i] > 1213.8) && (T[i] < 1214.0))) {
              logfile.print('-');
            } else {
              logfile.print(T[i]);
            }
            logfile.print(",");
          }
        }
        // commit to SD
        logfile.flush();
      }
    }

    if (millis() - timeOld >= samplePeriod){
      sampleCounter++;
      dt = millis() - startTime;
      dt /= 1000;
      timeOld = millis();

        // Sample temperatures
      sampleTemperature(T, max_0, 0);
      sampleTemperature(T, max_1, 1);
      sampleTemperature(T, max_2, 2);
      sampleTemperature(T, max_3, 3);
      sampleTemperature(T, max_4, 4);
      sampleTemperature(T, max_5, 5);
      sampleTemperature(T, max_6, 6);
      sampleTemperature(T, max_7, 7);
      sampleTemperature(T, max_8, 8);
      sampleTemperature(T, max_9, 9);

      // update display with new temperatures
      updateDisplay(T,samplingState,filename,error,screenCounter);

//      // Serial print data - stream values even if not logging to SD
//      Serial.printf("%.2f,", dt);
//      for (byte i = 0; i < 10; i++) {
//        if (i == 9) {
//          // If last temperature, print new line
//          if (T[i] == -999) {
//            Serial.println('-');
//          } else {
//            Serial.println(T[i]);
//          }
//        } else {
//          // Else print comma separated on single line
//          if (T[i] == -999) {
//            Serial.print('-');
//          } else {
//            Serial.print(T[i]);
//          }
//          Serial.print(",");
//        }
//      }
    } // end of (millis - timeOld)

    // If sampling has ended, and we were previously sampling
    if (!samplingState && samplingStateOld) {
      sampleCounter = 0;
      dt = 0;
      timeOld = -1000000;
      samplingStateOld = 0;
      logfile.println("End of Test \n");
      logfile.flush();
      logfile.close();
      digitalWrite(sampleLedPin, LOW);
      endOfTestBlink();
    }
  } // end of while(1)
} // end of loop

void errorCheckSDPresent(byte *errorP, SDClass &SD) {
  // Function to check if SD card is physically present in slot
  // If no SD card present, update error variable with correct error bit
  if (!SD.begin(chipSelect)) {
    *errorP = *errorP | 0b100;
  } else {
    // no error for SD card presence
    *errorP = *errorP & 0b011;
  }
}

//void errorDisplay(byte *errorP) {
//  if (SERIAL_DEBUG) {
//    Serial.println(*errorP, BIN);
//  }
//  if (*errorP) {
//    digitalWrite(SDErrLedPin, LOW);
//  } else {
//    digitalWrite(SDErrLedPin, HIGH);
//  }
//
//  if (SERIAL_DEBUG) {
//    if ((*errorP & 0b100) >> 2) {
//      Serial.println("SD error: No SD card detected");
//    }
//    if ((*errorP & 0b010) >> 1) {
//      Serial.println("SD error: No unique filename available");
//    }
//    if (*errorP & 0b001) {
//      Serial.println("SD error: File couldn't be created");
//    }
//  }
//
//}

void generateFilename(char filename[], char *number1, char *number2, byte *errorP) {
  int i = 1;
  // No error for unique filename999
  *errorP = *errorP & 0b101;
  while (SD.exists(filename)) {
    *number1 = i / 10 + '0';
    *number2 = i % 10 + '0';
    if (i > 99) {
      // no unique filename
      *errorP = *errorP | 0b010;
      break;
    }
    i++;
  }
}

void sampleTemperature(float T[], Adafruit_MAX31865_Modified &maxN, byte idx) {
  float temp = maxN.temperature(RNOMINAL, RREF);
  if (temp != -999 & (unit == 'K' || unit == 'k')) {
    temp += 273.15;
  }
  T[idx] = temp;
}

void endOfTestBlink() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(sampleLedPin, HIGH);
    delay(100);
    digitalWrite(sampleLedPin, LOW);
    delay(50);
  }
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    return pctime;
    if ( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
}

void dateTime(uint16_t* date, uint16_t* time)
{
  *date = FAT_DATE(year(), month(), day());
  *time = FAT_TIME(hour(), minute(), second());
}

void updateDisplay(float T[], bool samplingState, char filename[], byte err, int screenCounter){

  // clear non-temperature readings
  // 31.13 or 1261.94 appear when no PT100 attached
  for(int i=0; i<10; i++){
    // if(((T[i] > 31.11) && (T[i] < 31.15)) || ((T[i] > 1261.92) && (T[i] < 1261.96))){
    if((T[i] == -999) || ((T[i] > 1213.8) && (T[i] < 1214.0))){
      T[i] = 0.0f;
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if(screenCounter < 10){
    // display individual screens on screens 0-9
    char tBuf[2];
    char sBuf[6];
    char vBuf[8];
     
    // top row - <   T_   >
    display.setTextSize(1);
    display.setCursor(0,0);
    display.write("<");
    display.setCursor(55,0);
    display.write("T");
    sprintf(tBuf,"%i",screenCounter+1);
    display.write(tBuf);
    display.setCursor(122,0);
    display.write(">");

    // temperature display
    display.setTextSize(2);
    display.setCursor(20,25);
    dtostrf(T[screenCounter],6,2,sBuf);
    sprintf(vBuf,"%s K",sBuf);
    display.write(vBuf);

    // SD card status
    display.setTextSize(1);
    display.setCursor(0,57);
//    display.write("SD ");
    // if SD present, show tick, otherwise show cross
    if(err){
      display.write("No SD!");
    }else{
      display.write("SD detected");
    }

    // if logging, show file name
    display.setCursor(73,57);
    if(samplingState && !err){
      display.write(filename);
    }

    // commit changes
    display.display();
    
  }else{
    // show all on screen 10
    
    // T1
    char s1[6];
    dtostrf(T[0],6,2,s1);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.write("1: ");
    display.write(s1);

    // T2
    char s2[6];
    dtostrf(T[1],6,2,s2);
    display.setTextSize(1);
    display.setCursor(0,10);
    display.write("2: ");
    display.write(s2);

    // T3
    char s3[6];
    dtostrf(T[2],6,2,s3);
    display.setTextSize(1);
    display.setCursor(0,20);
    display.write("3: ");
    display.write(s3);

    // T4
    char s4[6];
    dtostrf(T[3],6,2,s4);
    display.setTextSize(1);
    display.setCursor(0,30);
    display.write("4: ");
    display.write(s4);

    // T5
    char s5[6];
    dtostrf(T[4],6,2,s5);
    display.setTextSize(1);
    display.setCursor(0,40);
    display.write("5: ");
    display.write(s5);

    // T6
    char s6[6];
    dtostrf(T[5],6,2,s6);
    display.setTextSize(1);
    display.setCursor(68,0);
    display.write("6: ");
    display.write(s6);

    // T7
    char s7[6];
    dtostrf(T[6],6,2,s7);
    display.setTextSize(1);
    display.setCursor(68,10);
    display.write("7: ");
    display.write(s7);

    // T8
    char s8[6];
    dtostrf(T[7],6,2,s8);
    display.setTextSize(1);
    display.setCursor(68,20);
    display.write("8: ");
    display.write(s8);

    // T9
    char s9[6];
    dtostrf(T[8],6,2,s9);
    display.setTextSize(1);
    display.setCursor(68,30);
    display.write("9: ");
    display.write(s9);

    // T10
    char s10[6];
    dtostrf(T[9],6,2,s10);
    display.setTextSize(1);
    display.setCursor(62,40);
    display.write("10: ");
    display.write(s10);

    // SD card status
    display.setTextSize(1);
    display.setCursor(0,57);
//    display.write("SD ");
    // if SD present, show tick, otherwise show cross
    if(err){
      display.write("No SD!");
    }else{
      display.write("SD detected");
    }

    // if logging, show file name
    if(samplingState && !err){
      display.setCursor(73,57);
      display.write(filename);
    }

    display.display();
  }

}
