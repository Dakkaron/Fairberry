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
  #define ARDUINO_CLOCKDOWN_RESET_VALUE 0b00000000
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
  //byte cols[] = {4,30,1,12,6};
  byte cols[] = {4,5,3,6,7}; // Using PORTD numbering instead of Arduino numbering

  const byte KEYBOARD_LIGHT_PIN = 13;
  #define LED0_PIN 0
  #define LED1_PIN 11
  
  #define FASTCOLS
  #define FASTCOLS_PORT PORTD
  #define FASTCOLS_PINCONFIG DDRD
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
byte debounceLoops[colCount][rowCount];

char keyboard[colCount][rowCount] = {
  {'q', 'w', NULL, 'a',  KEY_LEFT_ALT, ' ', KEY_LEFT_CTRL}, // sym, ALT, mic
  {'e', 's', 'd',  'p',  'x',  'z', KEY_LEFT_SHIFT}, // left shift
  {'r', 'g', 't',  KEY_RIGHT_SHIFT, 'v',  'c', 'f'}, // right shift
  {'u', 'h', 'y',  KEY_RETURN, 'b',  'n', 'j'}, // Enter
  {'o', 'l', 'i', KEY_BACKSPACE, '$', 'm', 'k'} // Backspace
};
char keyboard_symbol[colCount][rowCount] = {
  {'#', '1', NULL, '*',  KEY_TAB, ' ', '0'}, // sym, ALT, SPACE
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
volatile bool isClockedDown = false;
bool blinkState = true;
bool idleWakeup = false;
bool connectionNeedsReinit = false;

bool anyKeyReleased = false;

#define STICKY_STATUS_OPEN 0
#define STICKY_STATUS_STICKY 1
#define STICKY_STATUS_LOCKED 2

byte stickySym = STICKY_STATUS_OPEN;
byte stickyLsh = STICKY_STATUS_OPEN;
byte stickyRsh = STICKY_STATUS_OPEN;
byte stickyCtrl = STICKY_STATUS_OPEN;
byte stickyAlt = STICKY_STATUS_OPEN;

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
    for(int i=0; i<rowCount; i++) {
        //Serial.print(rows[i]); Serial.println(" as input");
        pinMode(rows[i], INPUT);
    }
 
    for (int i=0; i<colCount; i++) {
        //Serial.print(cols[i]); Serial.println(" as input-pullup");
        #ifdef FASTCOLS
          FASTCOLS_PINCONFIG = FASTCOLS_PINCONFIG & !(1 << cols[i]);
          FASTCOLS_PORT = FASTCOLS_PORT & !(1 << cols[i]);
        #else
          pinMode(cols[i], INPUT);
        #endif
    }

    // set pins for the keyboard backlight
    pinMode(KEYBOARD_LIGHT_PIN, OUTPUT);

    setKeyboardBacklight(keyboardLight, true);
}

void updateStickyKeyStates() {
  if (keyPressed(0, 2)) {
    byte tmp = stickySym;
    resetStickyKeys();
    stickySym = tmp + 1;
  }
  if (keyPressed(1, 6)) {
    byte tmp = stickyLsh;
    resetStickyKeys();
    stickyLsh = tmp + 1;
  }
  if (keyPressed(2, 3)) {
    byte tmp = stickyRsh;
    resetStickyKeys();
    stickyRsh = tmp + 1;
  }
  if (stickySym==STICKY_STATUS_OPEN && keyPressed(0, 6)) {
    byte tmp = stickyCtrl;
    resetStickyKeys();
    stickyCtrl = tmp + 1;
  }
  if (keyPressed(0, 4)) {
    byte tmp = stickyAlt;
    resetStickyKeys();
    stickyAlt = tmp + 1;
  }
}

void rolloverStickyKeyStates() {
  #ifndef STICKY_SYM
    if (stickySym == STICKY_STATUS_STICKY) {
      stickySym++;
    }
  #endif
  #ifndef STICKY_LSH
    if (stickyLsh == STICKY_STATUS_STICKY) {
      stickyLsh++;
    }
  #endif
  #ifndef STICKY_RSH
    if (stickyRsh == STICKY_STATUS_STICKY) {
      stickyRsh++;
    }
  #endif
  #ifndef STICKY_CTRL
    if (stickyCtrl == STICKY_STATUS_STICKY) {
      stickyCtrl++;
    }
  #endif
  #ifndef STICKY_ALT
    if (stickyAlt == STICKY_STATUS_STICKY) {
      stickyAlt++;
    }
  #endif

  #ifndef DOUBLETAP_LOCK_SYM
    if (stickySym == STICKY_STATUS_LOCKED) {
      stickySym++;
    }
  #endif
  #ifndef DOUBLETAP_LOCK_LSH
    if (stickyLsh == STICKY_STATUS_LOCKED) {
      stickyLsh++;
    }
  #endif
  #ifndef DOUBLETAP_LOCK_RSH
    if (stickyRsh == STICKY_STATUS_LOCKED) {
      stickyRsh++;
    }
  #endif
  #ifndef DOUBLETAP_LOCK_CTRL
    if (stickyCtrl == STICKY_STATUS_LOCKED) {
      stickyCtrl++;
    }
  #endif
  #ifndef DOUBLETAP_LOCK_ALT
    if (stickyAlt == STICKY_STATUS_LOCKED) {
      stickyAlt++;
    }
  #endif

  if (stickySym > STICKY_STATUS_LOCKED) {
    stickySym = STICKY_STATUS_OPEN;
  }
  if (stickyLsh > STICKY_STATUS_LOCKED) {
    stickyLsh = STICKY_STATUS_OPEN;
  }
  if (stickyRsh > STICKY_STATUS_LOCKED) {
    stickyRsh = STICKY_STATUS_OPEN;
  }
  if (stickyCtrl > STICKY_STATUS_LOCKED) {
    stickyCtrl = STICKY_STATUS_OPEN;
  }
  if (stickyAlt > STICKY_STATUS_LOCKED) {
    stickyAlt = STICKY_STATUS_OPEN;
  }
}

void wakeEverythingUp() {
  changeKeyboardBacklight(0, true);
  idleTimeout = millis() + IDLE_TIMEOUT;
  #ifdef POWERSAVE_ARDUINO_CLOCKDOWN
    if (isClockedDown) {
      CLKPR = 0b10000000;
      CLKPR = ARDUINO_CLOCKDOWN_RESET_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_RESET_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_RESET_VALUE;
      isClockedDown = false;
    }
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

void readMatrix() {
  int delayTime = 0;
  anyKeyReleased = false;
  // row: interate through the rows
  for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
    // iterate the columns
    byte curRow = rows[rowIndex];
    pinMode(curRow, INPUT_PULLUP);
    
    for (int colIndex=0; colIndex < colCount; colIndex++) {
      // col: set to output to low
      byte curCol = cols[colIndex];
      #ifdef FASTCOLS
        FASTCOLS_PINCONFIG = FASTCOLS_PINCONFIG | (1 << curCol);
        FASTCOLS_PORT = FASTCOLS_PORT & !(1 << curCol);
      #else
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);
      #endif

      if (debounceLoops[colIndex][rowIndex]>0) { // If debouncing is still active for this key, ignore all input
        debounceLoops[colIndex][rowIndex]--;
        changedValue[colIndex][rowIndex] = false;
      } else { // If debouncing is not active, process the key press
        bool buttonPressed = (digitalRead(curRow) == LOW);
        
        keys[colIndex][rowIndex] = buttonPressed;
        if ((lastValue[colIndex][rowIndex] != buttonPressed)) {
          changedValue[colIndex][rowIndex] = true;
          debounceLoops[colIndex][rowIndex] = DEBOUNCE_LOOPS;
          anyKeyReleased = anyKeyReleased | buttonPressed;
        } else {
          changedValue[colIndex][rowIndex] = false;
        }
  
        lastValue[colIndex][rowIndex] = buttonPressed;
      }

      #ifdef FASTCOLS
        FASTCOLS_PINCONFIG = FASTCOLS_PINCONFIG & !(1 << curCol);
        FASTCOLS_PORT = FASTCOLS_PORT & !(1 << curCol);
      #else
        pinMode(curCol, INPUT);
      #endif
    }
    pinMode(curRow, INPUT);
  }

  updateStickyKeyStates();
  rolloverStickyKeyStates();

  if (anyKeyReleased) {
    wakeEverythingUp();
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

void unstickKeys() {
  if (anyKeyReleased) {
    Serial.print("UNSTICK ");
    Serial.print(stickyRsh);
    Serial.print(" ");
    stickySym = (stickySym==STICKY_STATUS_STICKY) ? keyPressed(0, 2) : stickySym;
    stickyLsh = (stickyLsh==STICKY_STATUS_STICKY) ? keyPressed(1, 6) : stickyLsh;
    stickyRsh = (stickyRsh==STICKY_STATUS_STICKY) ? keyPressed(2, 3) : stickyRsh;
    stickyCtrl= (stickyCtrl==STICKY_STATUS_STICKY)? keyPressed(0, 6) : stickyCtrl;
    stickyAlt = (stickyAlt==STICKY_STATUS_STICKY) ? keyPressed(0, 4) : stickyAlt;
    Serial.println(stickyRsh);
  }
}

void resetStickyKeys() {
  stickySym = STICKY_STATUS_OPEN;
  stickyLsh = STICKY_STATUS_OPEN;
  stickyRsh = STICKY_STATUS_OPEN;
  stickyCtrl= STICKY_STATUS_OPEN;
  stickyAlt = STICKY_STATUS_OPEN;
}

void printMatrix() {
  for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
    for (int colIndex=0; colIndex < colCount; colIndex++) {
      // we only want to print if the key is pressed and it is a printable character
      if (keyChanged(colIndex, rowIndex) && isPrintableKey(colIndex, rowIndex)) {
        char toPrint;
        char other1;
        char other2;
        if (cursorMode) {
          toPrint = keyboard_cursor[colIndex][rowIndex];
          other1  = keyboard[colIndex][rowIndex];
          other2  = keyboard_symbol[colIndex][rowIndex];
        } else if (stickySym != STICKY_STATUS_OPEN || keyActive(0,2)) {
          toPrint = keyboard_symbol[colIndex][rowIndex];
          other1  = keyboard[colIndex][rowIndex];
          other2  = keyboard_cursor[colIndex][rowIndex];
        } else {
          toPrint = keyboard[colIndex][rowIndex];
          other1  = keyboard_symbol[colIndex][rowIndex];
          other2  = keyboard_cursor[colIndex][rowIndex];
        }
  
        // Workaround for left shift key dropping while being pressed
        if (keyActive(1,6) && rowIndex!=1 && colIndex!=6) {
          KEYBOARD_RELEASE(KEY_LEFT_SHIFT);
          KEYBOARD_PRESS(KEY_LEFT_SHIFT);
        }
        if (keyPressed(colIndex, rowIndex)) {
          if (toPrint!=NULL) {
            if (stickyLsh != STICKY_STATUS_OPEN && !keyActive(1, 6)) { KEYBOARD_PRESS(KEY_LEFT_SHIFT); }
            if (stickyRsh != STICKY_STATUS_OPEN && !keyActive(2, 3)) { KEYBOARD_PRESS(KEY_RIGHT_SHIFT); }
            if (stickyCtrl!= STICKY_STATUS_OPEN && !keyActive(0, 6)) { KEYBOARD_PRESS(KEY_LEFT_CTRL); }
            if (stickyAlt != STICKY_STATUS_OPEN && !keyActive(0, 4)) { KEYBOARD_PRESS(KEY_LEFT_ALT); }
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
          if (!keyActive(1, 6)) { KEYBOARD_RELEASE(KEY_LEFT_SHIFT); }
          if (!keyActive(2, 3)) { KEYBOARD_RELEASE(KEY_RIGHT_SHIFT); }
          if (!keyActive(0, 6)) { KEYBOARD_RELEASE(KEY_LEFT_CTRL); }
          if (!keyActive(0, 4)) { KEYBOARD_RELEASE(KEY_LEFT_ALT); }
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
    unstickKeys();
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
    resetStickyKeys();
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
    resetStickyKeys();
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
      isClockedDown = true;
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
