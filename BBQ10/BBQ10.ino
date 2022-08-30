#define FAIRBERRY_V0_1_1 1
#define ARDUINO 10
#define BEETLE 11
#define ESP32 12


#define CHIP_ATMEGA32U4 1
#define CHIP_ESP32 2

#include "configuration.h"

#if BOARD_TYPE == FAIRBERRY_V0_1_1 || BOARD_TYPE == ARDUINO || BOARD_TYPE==BEETLE
  #define CHIP_TYPE CHIP_ATMEGA32U4
#elif BOARD_TYPE == ESP32
  #define CHIP_ESP32
#endif

#if defined(POWERSAVE_ARDUINO_IDLE) && defined(POWERSAVE_ARDUINO_POWERDOWN)
  #error "Cannot have POWERSAVE_ARDUINO_IDLE and POWERSAVE_ARDUINO_POWERDOWN at the same time!"
#endif

#if CHIP_TYPE != CHIP_ATMEGA32U4 && defined(POWERSAVE_ARDUINO_IDLE)
  #error "Only boards with ATMEGA32U4 chips suport POWERSAVE_ARDUINO_IDLE!"
#endif

#if CHIP_TYPE != CHIP_ATMEGA32U4 && defined(POWERSAVE_ARDUINO_POWERDOWN)
  #error "Only boards with ATMEGA32U4 chips suport POWERSAVE_ARDUINO_POWERDOWN!"
#endif

#if defined(POWERSAVE_ARDUINO_CLOCKDOWN) && !defined(ARDUINO_CLOCKDOWN_DIVISION)
  #error "ARDUINO_CLOCKDOWN_DIVISION needs to be set (to 1,2,4,8,16,32,64,128 or 256) if POWERSAVE_ARDUINO_CLOCKDOWN is enabled"
#endif

#ifdef ARDUINO_CLOCKDOWN_DIVISION
  #if ARDUINO_CLOCKDOWN_DIVISION == 1
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000000
  #elif ARDUINO_CLOCKDOWN_DIVISION == 2
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000001
  #elif ARDUINO_CLOCKDOWN_DIVISION == 4
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000010
  #elif ARDUINO_CLOCKDOWN_DIVISION == 8
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000011
  #elif ARDUINO_CLOCKDOWN_DIVISION == 16
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000100
  #elif ARDUINO_CLOCKDOWN_DIVISION == 32
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000101
  #elif ARDUINO_CLOCKDOWN_DIVISION == 64
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000110
  #elif ARDUINO_CLOCKDOWN_DIVISION == 128
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00000111
  #elif ARDUINO_CLOCKDOWN_DIVISION == 256
    #define ARDUINO_CLOCKDOWN_DIVISION_VALUE 0b00001000
  #endif
#endif

#if BOARD_TYPE == FAIRBERRY_V0_1_1
  #define F_CPU 16000000
  #define TX_RX_LED_INIT  DDRE |= (1<<6), DDRB |= (1<<0)
  #define TXLED0      PORTE |= (1<<6)
  #define TXLED1      PORTE &= ~(1<<6)
  #define RXLED0      PORTB |= (1<<0)
  #define RXLED1      PORTB &= ~(1<<0)
#endif

#if CHIP_TYPE == CHIP_ATMEGA32U4
  #if !defined(DEBUG_SERIAL_INSTEAD_OF_USB)
    #include "Keyboard.h"
  #endif
  #include "LowPower.h"

  #ifdef DEBUG_SERIAL_INSTEAD_OF_USB
    #define KEYBOARD_BEGIN(layout) Serial.begin()
    #define KEYBOARD_BEGIN(layout) ;
    #define KEYBOARD_PRESS(key) Serial.print("Pressed ");Serial.println(key);
    #define KEYBOARD_RELEASE(key) Serial.print("Released ");Serial.println(key);
  #else
    #define KEYBOARD_BEGIN(layout) Keyboard.begin(layout)
    #define KEYBOARD_END() Keyboard.end()
    #define KEYBOARD_PRESS(key) Keyboard.press(key)
    #define KEYBOARD_RELEASE(key) Keyboard.release(key)
  #endif
#elif BOARD_TYPE == ESP32
  #include "analogWrite.h"
  #define USE_NIMBLE
  #include <BleKeyboard.h>
  #include "esp_bt.h"
  #include "esp_pm.h"
  #include "esp_wifi.h"
  BleKeyboard bleKeyboard;

  #define SLEEP_DURATION 120000L
  #define KEYBOARD_BEGIN(layout) bleKeyboard.begin()
  #define KEYBOARD_END() bleKeyboard.end()
  #define KEYBOARD_PRESS(key) bleKeyboard.press(key)
  #define KEYBOARD_RELEASE(key) bleKeyboard.release(key)
#endif

#if BOARD_TYPE == FAIRBERRY_V0_1_1
  byte rows[] = {A1,A2,8,9,10,5,A0};
  byte cols[] = {4,30,1,12,6};

  const byte KEYBOARD_LIGHT_PIN = 13;
  #define LED0_PIN 0
  #define LED1_PIN 11
#elif BOARD_TYPE == ARDUINO
  byte rows[] = {9,8,7,6,5,4,A2};
  byte cols[] = {A1,A0,15,14,16};

  const byte KEYBOARD_LIGHT_PIN = 10;
#elif BOARD_TYPE == BEETLE
  byte rows[] = {A5,11,10,9,18,19,20};
  byte cols[] = {3,2,0,14,15};

  const byte KEYBOARD_LIGHT_PIN = 10;
#elif BOARD_TYPE == ESP32
  #define RESET_PIN 33
  byte rows[] = {27,25,32,4,0,2,22};
  byte cols[] = {5,23,19,18,26};

  const byte KEYBOARD_LIGHT_PIN = 35;
#endif

#ifdef DEBUG_SERIAL_INSTEAD_OF_USB
  #define KEY_LEFT_ALT '?'
  #define KEY_LEFT_CTRL '?'
  #define KEY_LEFT_SHIFT '?'
  #define KEY_RIGHT_SHIFT '?'
  #define KEY_RIGHT '?'
  #define KEY_RETURN '?'
  #define KEY_BACKSPACE '?'
  #define KEY_ESC '?'
  #define KEY_UP_ARROW '?'
  #define KEY_LEFT_ARROW '?'
  #define KEY_DOWN_ARROW '?'
  #define KEY_RIGHT_ARROW '?'
  #define KEY_TAB '?'
  #define KEY_F3 '?'
  #define KEY_F4 '?'
  #define KEY_F5 '?'
  #define KEY_F6 '?'
  #define KEY_F7 '?'
  #define KEY_F8 '?'
  #define KEY_F15 '?'
#endif

const int rowCount = sizeof(rows)/sizeof(rows[0]);
const int colCount = sizeof(cols)/sizeof(cols[0]);



bool keys[colCount][rowCount];
bool lastValue[colCount][rowCount];
bool changedValue[colCount][rowCount];
char keyboard[colCount][rowCount] = {
  {'q', 'w', NULL, 'a',  KEY_LEFT_ALT, ' ', KEY_LEFT_CTRL}, // sym, ALT, mic
  {'e', 's', 'd',  'p',  'x',  'z', KEY_LEFT_SHIFT}, // left shift
  {'r', 'g', 't',  KEY_RIGHT_SHIFT, 'v',  'c', 'f'}, // right shift
  {'u', 'h', 'y',  KEY_RETURN, 'b',  'n', 'j'}, // Enter
  {'o', 'l', 'i', KEY_BACKSPACE, '€', 'm', 'k'} // Backspace
};
char keyboard_symbol[colCount][rowCount] = {
  {'#', '1', NULL, '*',  KEY_LEFT_ALT, ' ', '0'}, // sym, ALT, SPACE
  {'2', '4', '5',  '@',  '8',  '7', KEY_LEFT_SHIFT}, // left shift
  {'3', '/', '(',  KEY_RIGHT_SHIFT, '?',  '9', '6'}, // right shift
  {'_', ':', ')',  KEY_RETURN, '!',  ',', ';'}, // Enter
  {'+', '"', '-', KEY_BACKSPACE, NULL, '.', '\''} // $/Vol
};
char keyboard_cursor[colCount][rowCount] = {
  {KEY_ESC, KEY_UP_ARROW, NULL, KEY_LEFT_ARROW,  KEY_TAB, ' ', '0'}, // sym, ALT, SPACE
  {'2', KEY_DOWN_ARROW, KEY_RIGHT_ARROW,  KEY_F15,  '8',  '7', KEY_LEFT_SHIFT}, // left shift
  {'3', '/', '(',  KEY_RIGHT_SHIFT, '?',  '9', '6'}, // right shift
  {KEY_F7, ':', ')',  KEY_F4, '!', ',', ';'}, // Enter
  {KEY_F6, '"', KEY_F8, KEY_F3, KEY_F5, '.', '\''} // $/Vol
};


int keyboardLightSteps = 20;
int keyboardLight = 200;
volatile unsigned long idleTimeout = millis() + IDLE_TIMEOUT;
volatile unsigned long blinkTimeout = millis() + BLINK_TIMEOUT;
bool blinkState = true;
bool idleWakeup = false;
bool connectionNeedsReinit = false;

bool symbolSelected;

bool keyboardInit = false;
bool cursorMode = false;

void setup() {
    #ifdef LED0_PIN
      pinMode(LED0_PIN, OUTPUT);
    #endif
    #ifdef LED1_PIN
      pinMode(LED1_PIN, OUTPUT);
    #endif
    #if BOARD_TYPE == ESP32
      KEYBOARD_BEGIN(KeyboardLayout_en_US);
      keyboardInit = true;
      
      esp_bt_sleep_enable();
      
      esp_pm_config_esp32_t pm_config;
      pm_config.max_freq_mhz = 80;
      pm_config.min_freq_mhz = 80;
      pm_config.light_sleep_enable = true;
      esp_pm_configure(&pm_config);

      esp_wifi_set_mode(WIFI_MODE_NULL);
    #endif
    #ifdef POWERSAVE_ARDUINO_CLOCKDOWN
      CLKPR = 0b10000000;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
    #endif
    for(int x=0; x<rowCount; x++) {
        //Serial.print(rows[x]); Serial.println(" as input");
        pinMode(rows[x], INPUT);
    }
 
    for (int x=0; x<colCount; x++) {
        //Serial.print(cols[x]); Serial.println(" as input-pullup");
        pinMode(cols[x], INPUT_PULLUP);
    }

    // set pins for the keyboard backlight
    pinMode(KEYBOARD_LIGHT_PIN, OUTPUT);

    setKeyboardBacklight(keyboardLight, true);

    symbolSelected = false;
}

void readMatrix() {
    int delayTime = 0;
    // iterate the columns
    for (int colIndex=0; colIndex < colCount; colIndex++) {
        // col: set to output to low
        byte curCol = cols[colIndex];
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);
 
        // row: interate through the rows
        for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
            byte rowCol = rows[rowIndex];
            pinMode(rowCol, INPUT_PULLUP);
            delay(1); // arduino is not fast enought to switch input/output modes so wait 1 ms

            bool buttonPressed = (digitalRead(rowCol) == LOW);
            
            keys[colIndex][rowIndex] = buttonPressed;
            if ((lastValue[colIndex][rowIndex] != buttonPressed)) {
              changedValue[colIndex][rowIndex] = true;
            } else {
              changedValue[colIndex][rowIndex] = false;
            }

            lastValue[colIndex][rowIndex] = buttonPressed;
            pinMode(rowCol, INPUT);
        }
        // disable the column
        pinMode(curCol, INPUT);
    }

    if (keyPressed(0, 2)) {
      symbolSelected = true;
    }
}

bool keyPressed(int colIndex, int rowIndex) {
  return changedValue[colIndex][rowIndex] && keys[colIndex][rowIndex] == true;
}

bool keyReleased(int colIndex, int rowIndex) {
  return changedValue[colIndex][rowIndex] && keys[colIndex][rowIndex] == false;
}

bool keyChanged(int colIndex, int rowIndex) {
  return changedValue[colIndex][rowIndex];
}

bool keyActive(int colIndex, int rowIndex) {
  return keys[colIndex][rowIndex] == true;
}

bool isPrintableKey(int colIndex, int rowIndex) {
  return keyboard_symbol[colIndex][rowIndex] != NULL || keyboard[colIndex][rowIndex] != NULL;
}

void changeKeyboardBacklight(int change, bool on) {
  setKeyboardBacklight(keyboardLight + change, on);
}

void setKeyboardBacklight(int pwmValue, bool on) {
  if (pwmValue > 255) {
    pwmValue = 255;
  }
  if (pwmValue < 0) {
    pwmValue = 0;
  }
  keyboardLight = pwmValue;
  if (on) {
    analogWrite(KEYBOARD_LIGHT_PIN, keyboardLight);
  } else {
    analogWrite(KEYBOARD_LIGHT_PIN, 0);
  }
}

void printMatrix() {
    for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
        for (int colIndex=0; colIndex < colCount; colIndex++) {
          // we only want to print if the key is pressed and it is a printable character
          if (keyActive(colIndex, rowIndex)) {
            changeKeyboardBacklight(0, true);
            idleTimeout = millis() + IDLE_TIMEOUT;
            #ifdef POWERSAVE_ARDUINO_CLOCKDOWN
              CLKPR = 0b10000000;
              CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
              CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
              CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
            #endif
            #ifdef POWERSAVE_ARDUINO_POWERDOWN
              if (connectionNeedsReinit) {
                connectionNeedsReinit = false;
                USBDevice.attach();
                keyboardInit = false;
                delay(50);
                KEYBOARD_BEGIN(KeyboardLayout_en_US);
                delay(300);
              }
            #endif
            #ifdef POWERSAVE_ESP32_LIGHT_SLEEP
              if (connectionNeedsReinit) {
                pinMode(RESET_PIN, OUTPUT);
                digitalWrite(RESET_PIN, LOW);
                
                /*connectionNeedsReinit = false;
                btStart();
                KEYBOARD_BEGIN(KeyboardLayout_en_US);*/
              }
            #endif
          }
          /*if (keyActive(colIndex, rowIndex)) {
            idleTimeout = millis() + IDLE_TIMEOUT;
            changeKeyboardBacklight(0, true);
          }*/
          if (keyChanged(colIndex, rowIndex) && isPrintableKey(colIndex, rowIndex)) {
            char toPrint;
            char other1;
            char other2;
            if (cursorMode) {
              toPrint = keyboard_cursor[colIndex][rowIndex];
              other1  = keyboard[colIndex][rowIndex];
              other2  = keyboard_symbol[colIndex][rowIndex];
            } else if (symbolSelected) {
              symbolSelected = false;
              toPrint = keyboard_symbol[colIndex][rowIndex];
              other1  = keyboard[colIndex][rowIndex];
              other2  = keyboard_cursor[colIndex][rowIndex];
            } else {
              toPrint = keyboard[colIndex][rowIndex];
              other1  = keyboard_symbol[colIndex][rowIndex];
              other2  = keyboard_cursor[colIndex][rowIndex];
            }

            // keys 1,6 and 2,3 are Shift keys, so we want to upper case
           /* if (keyActive(1,6) || keyActive(2,3)) {
              toPrint.toUpperCase();
            }*/
            if (keyPressed(colIndex, rowIndex)) {
              if (toPrint!=NULL) {
                KEYBOARD_PRESS(toPrint);
              }
            } else {
              if (toPrint!=NULL) {
                KEYBOARD_RELEASE(toPrint);
              }
              if (other1!=NULL) {
                KEYBOARD_RELEASE(other1);
              }
              if (other2!=NULL) {
                KEYBOARD_RELEASE(other2);
              }
            }
          }
        }   
    }
}

void loop() {
  idleWakeup = false;
  unsigned long startms = millis();
  #if BOARD_TYPE == ESP32
  if (bleKeyboard.isConnected()) {
  #endif
    readMatrix();
    printMatrix();
    //Serial.print("matrix: ");
    //Serial.println(millis()-startms);
  #if BOARD_TYPE == ESP32
  }
  #endif

  // increase backlight if mic key + sym key is pressed
  if (keyActive(0,6) && keyPressed(0,2)) {
    changeKeyboardBacklight(keyboardLightSteps, true);
  }

  // decrease backlight if mic key + right shift key is pressed
  if (keyActive(0,6) && keyPressed(2,3)) {
    changeKeyboardBacklight(-keyboardLightSteps, true);
  }

  if ((keyActive(1,6) && keyPressed(2,3)) || (keyPressed(1,6) && keyActive(2,3))) {
    cursorMode = !cursorMode;
  }
  #ifdef BLINK_IN_CURSOR_MODE
    if (idleTimeout>=millis()) {
      if (cursorMode && blinkTimeout<millis()) {
        blinkTimeout = millis() + BLINK_TIMEOUT;
        blinkState = !blinkState;
      } else if (!cursorMode) {
        blinkState = true;
      }
      changeKeyboardBacklight(0, blinkState);
    }
  #endif
  #ifdef LED0_IN_CURSOR_MODE
    digitalWrite(LED0_PIN, cursorMode);
  #endif

  //Serial.print("Loop ");
  //Serial.println(idleTimeout-millis());
  if (idleTimeout<millis()) {
    //Serial.println("Sleep");
    changeKeyboardBacklight(0, false);
    #ifdef POWERSAVE_ARDUINO_POWERDOWN
      Keyboard.end();
      USBDevice.detach();
      connectionNeedsReinit = true;
      LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
    #endif
    #ifdef POWERSAVE_ARDUINO_CLOCKDOWN
      CLKPR = 0b10000000;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
    #endif
    #ifdef POWERSAVE_ARDUINO_IDLE
      LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON);
    #endif
    #ifdef POWERSAVE_ESP32_LIGHT_SLEEP
      if (!connectionNeedsReinit) {
        btStop();
        connectionNeedsReinit = true;
      }
      esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
      esp_light_sleep_start();
    #endif
  }
  //Serial.print("total:  ");
  //Serial.println(millis()-startms);
}
