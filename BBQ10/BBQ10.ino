/*
 * ## Power Saving
 * Without power saving options the whole device consumes around 60mA.
 * While it's active, that's fine, but it consumes the same amount of
 * energy while it's idle. Since the device is idle for most of the time,
 * reducing the idle power is important.
 * 
 * For this there are multiple options:
 * 
 * ### IDLE_TIMEOUT
 * This is the basic setting that determines after how many ms
 * powersaving features are triggered. If the device was idle for more
 * than the given time, the keyboard backlight is disabled and the given
 * power save mode is applied (if any).
 * 
 * ### POWERSAVE_IDLE
 * This is the recommended powersaving mode. It's a light sleep that keeps
 * the USB connection active. It saves around 40mA. It introduces a delay
 * of up to 120ms when the device is woken up.
 * 
 * ### POWERSAVE_POWERDOWN
 * This is the more aggressive powersaving method. It completely turns off
 * the microcontroller, saving around 55mA. This takes the power consumption
 * down to almost nothing, but USB needs to reconnect after the keyboard
 * is woken up. This takes about 1-2 seconds.
 * 
 * You can use either POWERSAVE_IDLE or POWERSAVE_POWERDOWN or none, but not both.
 */
#define IDLE_TIMEOUT 20L * 1000L
#define POWERSAVE_IDLE
//#define POWERSAVE_POWERDOWN


/*
 * To visually show that the device is in cursor mode, the keyboard backlight
 * will blink rapidly if BLINK_IN_CURSOR_MODE is set. Change BLINK_TIMEOUT
 * to change the blinking frequency.
 */
#define BLINK_IN_CURSOR_MODE
#define BLINK_TIMEOUT 20L


#if defined(POWERSAVE_IDLE) && defined(POWERSAVE_POWERDOWN)
  #error "Cannot have POWERSAVE_IDLE and POWERSAVE_POWERDOWN at the same time!"
#endif


#include "Keyboard.h"

#include "LowPower.h"

byte rows[] = {9,8,7,6,5,4,A2};
const int rowCount = sizeof(rows)/sizeof(rows[0]);
 
byte cols[] = {A1,A0,15,14,16};
const int colCount = sizeof(cols)/sizeof(cols[0]);

const byte KEYBOARD_LIGHT_PIN = 10;

bool keys[colCount][rowCount];
bool lastValue[colCount][rowCount];
bool changedValue[colCount][rowCount];
char keyboard[colCount][rowCount] = {
  {'q', 'w', NULL, 'a',  KEY_LEFT_ALT, ' ', KEY_LEFT_CTRL}, // sym, ALT, mic
  {'e', 's', 'd',  'p',  'x',  'z', KEY_LEFT_SHIFT}, // left shift
  {'r', 'g', 't',  KEY_RIGHT_SHIFT, 'v',  'c', 'f'}, // right shift
  {'u', 'h', 'y',  KEY_RETURN, 'b',  'n', 'j'}, // Enter
  {'o', 'l', 'i', KEY_BACKSPACE, '???', 'm', 'k'} // Backspace
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
bool usbNeedsReinit = false;

bool symbolSelected;

bool keyboardInit = false;
bool cursorMode = false;

void setup() {
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

            #ifdef POWERSAVE_POWERDOWN
              if (usbNeedsReinit) {
                usbNeedsReinit = false;
                USBDevice.attach();
                keyboardInit = false;
                delay(50);
                Keyboard.begin(KeyboardLayout_en_US);
                delay(300);
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
                Keyboard.press(toPrint);
              }
            } else {
              if (toPrint!=NULL) {
                Keyboard.release(toPrint);
              }
              if (other1!=NULL) {
                Keyboard.release(other1);
              }
              if (other2!=NULL) {
                Keyboard.release(other2);
              }
            }
          }
        }   
    }
}

void loop() {
  idleWakeup = false;
  readMatrix();
  
  printMatrix();

  // key 3,3 is the enter key
  if (keyPressed(3,3)) {
    if (!keyboardInit) {
      Keyboard.begin();
      //Keyboard.begin(KeyboardLayout_de_DE);
    }
    Keyboard.write('\n');
  }
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
  if (idleTimeout<millis()) {
    changeKeyboardBacklight(0, false);
    #ifdef POWERSAVE_POWERDOWN
      Keyboard.end();
      USBDevice.detach();
      usbNeedsReinit = true;
      LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
    #endif
    #ifdef POWERSAVE_IDLEuz
      LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON);
    #endif
  }
  delay(10);
}
