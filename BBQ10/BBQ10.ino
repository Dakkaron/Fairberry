#define USB_HID_OUT
#define IDLE_TIMEOUT 60 * 1000

#ifdef USB_HID_OUT
#include "KeyboardDE.h"
#endif

#include "LowPower.h"

byte rows[] = {9,8,7,6,5,4,3};
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
  {KEY_U_UMLAUT, ':', ')',  KEY_F14, '!', ',', ';'}, // Enter
  {KEY_O_UMLAUT, '"', KEY_A_UMLAUT, KEY_F13, KEY_ESZETT, '.', '\''} // $/Vol
};


int keyboardLightSteps = 20;
int keyboardLight = 200;
volatile unsigned long idleTimeout = millis() + IDLE_TIMEOUT;
bool idleWakeup = false;
bool usbNeedsReinit = false;

bool symbolSelected;

bool keyboardInit = false;
bool cursorMode = false;

void setup() {
    #ifndef USB_HID_OUT
      Serial.begin(9600);
      Serial.println("HERE");
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

    setKeyboardBacklight(keyboardLight);

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

void setKeyboardBacklight(int keyboardLight) {
    analogWrite(KEYBOARD_LIGHT_PIN, keyboardLight);
}


void printMatrix() {
    for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
        for (int colIndex=0; colIndex < colCount; colIndex++) {
          // we only want to print if the key is pressed and it is a printable character
          if (usbNeedsReinit && keyActive(colIndex, rowIndex)) {
            usbNeedsReinit = false;
            setKeyboardBacklight(keyboardLight);
            idleTimeout = millis() + IDLE_TIMEOUT;
            USBDevice.attach();
            keyboardInit = false;
            delay(50);
            //Keyboard.begin(KeyboardLayout_de_DE);
          }
          if (keyChanged(colIndex, rowIndex) && isPrintableKey(colIndex, rowIndex)) {
            #ifdef USB_HID_OUT
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
            #else
              String toPrint;
              if (symbolSelected) {
                symbolSelected = keyActive(0,2);
                toPrint = String(keyboard_symbol[colIndex][rowIndex]);
              } else {
                toPrint = String(keyboard[colIndex][rowIndex]);
              }
            #endif

            // keys 1,6 and 2,3 are Shift keys, so we want to upper case
           /* if (keyActive(1,6) || keyActive(2,3)) {
              toPrint.toUpperCase();
            }*/
            #ifdef USB_HID_OUT
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
            #else
              if (keyPressed(colIndex, rowIndex)) {
                Serial.print(toPrint);
              }
            #endif
          }
        }   
    }
}

// a helper method that checks boundaries for the PWM values
void changeBackgroundLight(int pwmValue) {
  if (pwmValue > 255) {
    pwmValue = 255;
  }
  if (pwmValue < 0) {
    pwmValue = 0;
  }
  keyboardLight = pwmValue;
  setKeyboardBacklight(keyboardLight);
}

void loop() {
  idleWakeup = false;
  readMatrix();
  
  printMatrix();

  // key 3,3 is the enter key
  if (keyPressed(3,3)) {
    #ifdef USB_HID_OUT
      if (!keyboardInit) {
        Keyboard.begin();
        //Keyboard.begin(KeyboardLayout_de_DE);
      }
      Keyboard.write('\n');
    #else
      Serial.println();
    #endif
  }
  // increase backlight if symbol key is pressed
  if (keyActive(0,6) && keyPressed(0,2)) {
    changeBackgroundLight(keyboardLight + keyboardLightSteps);
  }

  // decrease backlight if right shift key is pressed
  if (keyActive(0,6) && keyPressed(2,3)) {
    changeBackgroundLight(keyboardLight - keyboardLightSteps);
  }

  if (keyActive(1,6) && keyPressed(2,3)) {
    cursorMode = !cursorMode;
  }
  if (idleTimeout<millis()) {
    setKeyboardBacklight(0);
    Keyboard.end();
    USBCON = 0;
    usbNeedsReinit = true;
    LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
  }
  delay(10);
}
