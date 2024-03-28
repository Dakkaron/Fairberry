// Fairberry Keyboard Firmware © 2024 by Dakkaron is licensed under Attribution-NonCommercial-ShareAlike 4.0 International.
// To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/
// Version 3

#define FAIRBERRY_V0_3_0 3
#define FAIRBERRY_V0_2_0 2
#define FAIRBERRY_V0_1_1 1
#define ARDUINO 10
#define BEETLE 1101
#define ESP32 1102


#define CHIP_ATMEGA32U4 1
#define CHIP_ESP32 2

#include "configuration.h"
#include "constants.h"
#include "keymaps.h"
#include "boards.h"


bool keys[colCount][rowCount];
bool lastValue[colCount][rowCount];
bool changedValue[colCount][rowCount];
byte debounceMs[colCount][rowCount];
unsigned long lastDebounceMs;


int keyboardLightSteps = 20;
int keyboardLight1 = DEFAULT_KEYBOARD_BRIGHTNESS;
int keyboardLight2 = DEFAULT_KEYBOARD_BRIGHTNESS;
volatile unsigned long idleTimeout = millis() + IDLE_TIMEOUT;
volatile unsigned long backlightTimeout = millis() + BACKLIGHT_TIMEOUT;
volatile unsigned long blinkTimeout = millis() + BLINK_TIMEOUT;
volatile bool isClockedDown = false;
bool blinkState = true;
bool idleWakeup = false;
bool firstSleep = true;
bool connectionNeedsReinit1 = false;
volatile unsigned long connectionNeedsReinit2 = false;

#define USB_ADDRESS_EXISTS (_BV(ADDEN)==0x80 && UDINT > 0)
bool usbGotDisconnected = false;

bool anyKeyReleased = false;

byte stickySym = STICKY_STATUS_OPEN;
byte stickyLsh = STICKY_STATUS_OPEN;
byte stickyRsh = STICKY_STATUS_OPEN;
byte stickyCtrl = STICKY_STATUS_OPEN;
byte stickyAlt = STICKY_STATUS_OPEN;
bool cursorMode = false;

#define KEYBOARD_CONNECTION_READY keyboardConnectionReadyTimeout < millis()
#define ACTIVATE_KEYBOARD_CONNECTION_BUFFER() keyboardConnectionReadyTimeout = millis() + 500;
unsigned long keyboardConnectionReadyTimeout=0;

void(* resetFunc) (void) = 0;

void setup() {
    #ifdef LED0_PIN
      pinMode(LED0_PIN, OUTPUT);
    #endif
    #ifdef LED1_PIN
      pinMode(LED1_PIN, OUTPUT);
    #endif
    #if BOARD_TYPE == ESP32
      KEYBOARD_BEGIN(KeyboardLayout_en_US);
      
      esp_bt_sleep_enable();
      
      esp_pm_config_esp32_t pm_config;
      pm_config.max_freq_mhz = 80;
      pm_config.min_freq_mhz = 80;
      pm_config.light_sleep_enable = true;
      esp_pm_configure(&pm_config);

      esp_wifi_set_mode(WIFI_MODE_NULL);
    #endif
    for(int i=0; i<rowCount; i++) {
        #ifdef SERIAL_DEBUG_LOG
          Serial.print(rows[i]); Serial.println(" as input");
        #endif
        pinMode(rows[i], INPUT);
    }
 
    for (int i=0; i<colCount; i++) {
        #ifdef SERIAL_DEBUG_LOG
          Serial.print(cols[i]); Serial.println(" as input-pullup");
        #endif
        #ifdef FASTCOLS
          FASTCOLS_PINCONFIG = FASTCOLS_PINCONFIG & !(1 << cols[i]);
          FASTCOLS_PORT = FASTCOLS_PORT & !(1 << cols[i]);
        #else
          pinMode(cols[i], INPUT);
        #endif
    }

    // set pins for the keyboard backlight
    #ifdef KEYBOARD_LIGHT_PIN_1
      pinMode(KEYBOARD_LIGHT_PIN_1, OUTPUT);
    #endif
    #ifdef KEYBOARD_LIGHT_PIN_2
      pinMode(KEYBOARD_LIGHT_PIN_2, OUTPUT);
    #endif

    setKeyboardBacklight(keyboardLight1, keyboardLight2, true);
    lastDebounceMs = millis();

    while (USBDevice.isSuspended()) {}
    USBCON |= (1 << USBE);
    USBDevice.attach();
}

void updateStickyKeyStates() {
  if (keyPressed(K_SYM) && !keyActive(K_MIC)) { // SYM(0,2) (inactive while CTRL(0,6) is held, because of brightness down)
    byte tmp = stickySym;
    resetStickyKeys();
    stickySym = tmp + 1;
  }
  if (keyPressed(K_LSH)) { // LSH(1,6)
    byte tmp = stickyLsh;
    resetStickyKeys();
    stickyLsh = tmp + 1;
  }
  if (keyPressed(K_RSH) && !keyActive(K_MIC)) { // RSH(2,3) (inactive while CTRL(0,6) is held, because of brightness down)
    byte tmp = stickyRsh;
    resetStickyKeys();
    stickyRsh = tmp + 1;
  }
  if (stickySym==STICKY_STATUS_OPEN && keyPressed(K_MIC)) { // CTRL(0,6)
    byte tmp = stickyCtrl;
    resetStickyKeys();
    stickyCtrl = tmp + 1;
  }
  if (stickySym==STICKY_STATUS_OPEN && keyPressed(K_ALT)) { // ALT(0,4)
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
  changeKeyboardBacklight(0, 0, true);
  idleTimeout = millis() + IDLE_TIMEOUT;
  backlightTimeout = millis() + BACKLIGHT_TIMEOUT;
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
    if (connectionNeedsReinit1) {
      digitalWrite(LED1_PIN, 1);
      connectionNeedsReinit1 = false;
      connectionNeedsReinit2 = millis()+200;
      USBDevice.attach();
      ACTIVATE_KEYBOARD_CONNECTION_BUFFER();
    } else if (connectionNeedsReinit2!=0 && connectionNeedsReinit2<millis()) {
      KEYBOARD_BEGIN(KeyboardLayout_en_US);
      #ifdef SERIAL_DEBUG_LOG
        Serial.print(millis());
        Serial.println(": Activated connection buffer");
      #endif
      digitalWrite(LED1_PIN, 0);
      connectionNeedsReinit2 = 0;
    }
  #endif
  #ifdef POWERSAVE_ESP32_LIGHT_SLEEP
    if (connectionNeedsReinit1) {
      pinMode(RESET_PIN, OUTPUT);
      digitalWrite(RESET_PIN, LOW);
      
      /*connectionNeedsReinit1 = false;
      btStart();
      KEYBOARD_BEGIN(KeyboardLayout_en_US);*/
    }
  #endif
}

boolean anyKeyPressed = false;
/**
 * Returns whether any key was changed (true == at least one key changed state)
 */
boolean readMatrix(byte debouceMsSinceLast) {
  int delayTime = 0;
  anyKeyReleased = false;
  boolean anyKeyChanged = false;
  boolean anyKeyPressed = true;
  #ifdef FASTCHECK
    #ifdef FASTCOLCHECK_D
      DDRD |= FASTCOLCHECK_D;
      PORTD &= ~FASTCOLCHECK_D;
    #endif
    #ifdef FASTROWCHECK_B
      DDRB &= ~FASTROWCHECK_B;
      PORTB |= FASTROWCHECK_B;
    #endif
    #ifdef FASTROWCHECK_C
      DDRC &= ~FASTROWCHECK_C;
      PORTC |= FASTROWCHECK_C;
    #endif
    #ifdef FASTROWCHECK_D
      DDRD &= ~FASTROWCHECK_D;
      PORTD |= FASTROWCHECK_D;
    #endif
    #ifdef FASTROWCHECK_E
      DDRE &= ~FASTROWCHECK_E;
      PORTE |= FASTROWCHECK_E;
    #endif
    #ifdef FASTROWCHECK_F
      DDRF &= ~FASTROWCHECK_F;
      PORTF |= FASTROWCHECK_F;
    #endif
    delayMicroseconds(10);
    anyKeyPressed = false;
    #ifdef FASTROWCHECK_B
      anyKeyPressed |= ((~PINB) & FASTROWCHECK_B);
      PORTB &= ~FASTROWCHECK_B;
    #endif
    #ifdef FASTROWCHECK_C
      anyKeyPressed |= ((~PINC) & FASTROWCHECK_C);
      PORTC &= ~FASTROWCHECK_C;
    #endif
    #ifdef FASTROWCHECK_D
      anyKeyPressed |= ((~PIND) & FASTROWCHECK_D);
      PORTD &= ~FASTROWCHECK_D;
    #endif
    #ifdef FASTROWCHECK_E
      anyKeyPressed |= ((~PINE) & FASTROWCHECK_E);
      PORTE &= ~FASTROWCHECK_E;
    #endif
    #ifdef FASTROWCHECK_F
      anyKeyPressed |= ((~PINF) & FASTROWCHECK_F);
      PORTF &= ~FASTROWCHECK_F;
    #endif
    #ifdef FASTCOLCHECK_D
      DDRD &= ~FASTCOLCHECK_D;
    #endif
  #endif

  
  for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
    // iterate the columns
    byte curRow = rows[rowIndex];
    if (anyKeyPressed) {
      pinMode(curRow, INPUT_PULLUP);
    }
    
    for (int colIndex=0; colIndex < colCount; colIndex++) {
      // col: set to output to low
      byte curCol = cols[colIndex];
      if (anyKeyPressed) {
        #ifdef FASTCOLS
          FASTCOLS_PINCONFIG = FASTCOLS_PINCONFIG | (1 << curCol);
          FASTCOLS_PORT = FASTCOLS_PORT & !(1 << curCol);
        #else
          pinMode(curCol, OUTPUT);
          digitalWrite(curCol, LOW);
        #endif
      }

      if (debounceMs[colIndex][rowIndex]>0) { // If debouncing is still active for this key, ignore all input
        if (debounceMs[colIndex][rowIndex] >= debouceMsSinceLast) {
          debounceMs[colIndex][rowIndex] -= debouceMsSinceLast;
        } else {
          debounceMs[colIndex][rowIndex] = 0;
        }
        changedValue[colIndex][rowIndex] = false;
      } else { // If debouncing is not active, process the key press
        bool buttonPressed = (anyKeyPressed) && (digitalRead(curRow) == LOW);
        
        keys[colIndex][rowIndex] = buttonPressed;
        if ((lastValue[colIndex][rowIndex] != buttonPressed)) {
          changedValue[colIndex][rowIndex] = true;
          debounceMs[colIndex][rowIndex] = DEBOUNCE_MS;
          anyKeyReleased = anyKeyReleased | buttonPressed;
          anyKeyChanged = true;
        } else {
          changedValue[colIndex][rowIndex] = false;
        }
  
        lastValue[colIndex][rowIndex] = buttonPressed;
      }

      if (anyKeyPressed) {
        #ifdef FASTCOLS
          FASTCOLS_PINCONFIG = FASTCOLS_PINCONFIG & !(1 << curCol);
          FASTCOLS_PORT = FASTCOLS_PORT & !(1 << curCol);
        #else
          pinMode(curCol, INPUT);
        #endif
      }
    }
    if (anyKeyPressed) {
      pinMode(curRow, INPUT);
    }
  }

  #ifdef FASTROWCHECK_B
    DDRB &= ~FASTROWCHECK_B;
    PORTB |= FASTROWCHECK_B;
  #endif
  #ifdef FASTROWCHECK_C
    DDRC &= ~FASTROWCHECK_C;
    PORTC |= FASTROWCHECK_C;
  #endif
  #ifdef FASTROWCHECK_D
    DDRD &= ~FASTROWCHECK_D;
    PORTD |= FASTROWCHECK_D;
  #endif
  #ifdef FASTROWCHECK_E
    DDRE &= ~FASTROWCHECK_E;
    PORTE |= FASTROWCHECK_E;
  #endif
  #ifdef FASTROWCHECK_F
    DDRF &= ~FASTROWCHECK_F;
    PORTF |= FASTROWCHECK_F;
  #endif
  #ifdef FASTCOLCHECK_D
    DDRD &= ~FASTCOLCHECK_D;
    PORTD |= FASTCOLCHECK_D;
  #endif

  updateStickyKeyStates();
  rolloverStickyKeyStates();

  if (anyKeyReleased) {
    wakeEverythingUp();
  }
  return anyKeyChanged;
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

void changeKeyboardBacklight(int change1, int change2, bool on) {
  setKeyboardBacklight(keyboardLight1 + change1, keyboardLight2 + change2, on);
}

void setKeyboardBacklight(int pwmValue1, int pwmValue2, bool on) {
  if (pwmValue1 > 255) {
    pwmValue1 = 255;
  }
  if (pwmValue1 < 0) {
    pwmValue1 = 0;
  }
  if (pwmValue2 > 255) {
    pwmValue2 = 255;
  }
  if (pwmValue2 < 0) {
    pwmValue2 = 0;
  }
  keyboardLight1 = pwmValue1;
  keyboardLight2 = pwmValue2;
  if (on) {
    #ifdef KEYBOARD_LIGHT_PIN_1
      analogWrite(KEYBOARD_LIGHT_PIN_1, keyboardLight1);
    #endif
    #ifdef KEYBOARD_LIGHT_PIN_2
      analogWrite(KEYBOARD_LIGHT_PIN_2, keyboardLight2);
    #endif
  } else {
    #ifdef KEYBOARD_LIGHT_PIN_1
      analogWrite(KEYBOARD_LIGHT_PIN_1, 0);
    #endif
    #ifdef KEYBOARD_LIGHT_PIN_2
      analogWrite(KEYBOARD_LIGHT_PIN_2, 0);
    #endif
  }
}

void unstickKeys() {
  if (anyKeyReleased) {
    stickySym = (stickySym==STICKY_STATUS_STICKY) ? keyPressed(K_SYM) : stickySym;
    stickyLsh = (stickyLsh==STICKY_STATUS_STICKY) ? keyPressed(K_LSH) : stickyLsh;
    stickyRsh = (stickyRsh==STICKY_STATUS_STICKY) ? keyPressed(K_RSH) : stickyRsh;
    stickyCtrl= (stickyCtrl==STICKY_STATUS_STICKY)? keyPressed(K_MIC) : stickyCtrl;
    stickyAlt = (stickyAlt==STICKY_STATUS_STICKY) ? keyPressed(K_ALT) : stickyAlt;
  }
}

void resetStickyKeys() {
  stickySym = STICKY_STATUS_OPEN;
  stickyLsh = STICKY_STATUS_OPEN;
  stickyRsh = STICKY_STATUS_OPEN;
  stickyCtrl= STICKY_STATUS_OPEN;
  stickyAlt = STICKY_STATUS_OPEN;
}

char keyPressBuffer[32]; //0=End of presses in buffer 
uint32_t keyPressBufferType; //1=press, 0=release

void flushKeyboardBuffer() {
  #ifdef SERIAL_DEBUG_LOG
    Serial.print(millis());
    Serial.println(": Flushing buffer");
  #endif
  for (uint8_t i=0;i<32;i++) {
    if (keyPressBuffer[i]!=0) {
      bool pressed = !!(keyPressBufferType & (1ull<<i));
      if (pressed) {
        KEYBOARD_PRESS(keyPressBuffer[i]);
        #ifdef SERIAL_DEBUG_LOG
          Serial.print("Pressed slot ");
          Serial.print(i);
          Serial.print("/");
          Serial.println(keyPressBuffer[i]);
        #endif
        bool foundRelease = false;
        for (uint8_t j=i+1;j<32;j++) {
          if (keyPressBuffer[j]==keyPressBuffer[i] && !(keyPressBufferType & (1ull<<i))) {
            foundRelease = true;
          }
        }
        if (!foundRelease) {
          #ifdef SERIAL_DEBUG_LOG
            Serial.println("Sending missing release");
          #endif
          KEYBOARD_RELEASE(keyPressBuffer[i]);
        }
      } else {
        KEYBOARD_RELEASE(keyPressBuffer[i]);
        #ifdef SERIAL_DEBUG_LOG
          Serial.print("Released slot ");
          Serial.print(i);
          Serial.print("/");
          Serial.println(keyPressBuffer[i]);
        #endif
        delay(30);
      }
      keyPressBuffer[i]=0;
    }
  }
}

void keyboardEventBuffered(char key, bool pressed) {
  #ifdef SERIAL_DEBUG_LOG
    Serial.print("Key event buffered: ");
    Serial.print(key);
    Serial.print("/");
    Serial.print((uint8_t)key);
    Serial.print("/");
    Serial.print(pressed);
    Serial.print("/");
    Serial.println(KEYBOARD_CONNECTION_READY);
  #endif
  if (KEYBOARD_CONNECTION_READY) {
    #ifdef SERIAL_DEBUG_LOG
      Serial.println("Forwarding");
    #endif
    if (keyPressBuffer[0]!=0) {
      flushKeyboardBuffer();
    }
    if (pressed) {
      KEYBOARD_PRESS(key);
    } else {
      KEYBOARD_RELEASE(key);
    }
  } else {
    #ifdef SERIAL_DEBUG_LOG
      Serial.println("Buffering");
    #endif
    boolean foundPress = pressed; // Buffer all presses, buffer only releases with matching presses
    for (uint8_t i=0;i<32;i++) {
      #ifdef SERIAL_DEBUG_LOG
        Serial.print("Buffer slot ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println((uint8_t)keyPressBuffer[i]);
      #endif
      if (!foundPress && keyPressBuffer[i]==key && (!(keyPressBufferType&(1ull<<(i-1))))==pressed) {
        foundPress = true;
      }
      if (keyPressBuffer[i]==0) {
        if (!foundPress) {
          #ifdef SERIAL_DEBUG_LOG
            Serial.println("Skipped buffering due to no press found");
          #endif
          break;
        }
        if (i>0 && keyPressBuffer[i-1]==key && (!!(keyPressBufferType&(1ull<<(i-1))))==pressed) {
          #ifdef SERIAL_DEBUG_LOG
            Serial.print("Skipped buffering: ");
            Serial.print((!!(keyPressBufferType&(1ull<<(i-1)))));
            Serial.print("/");
            Serial.println(pressed);
          #endif
          break;
        }
        keyPressBuffer[i] = key;
        keyPressBufferType &= ~(0ull<<i);
        keyPressBufferType |= ((uint32_t)pressed)<<i;
        #ifdef SERIAL_DEBUG_LOG
          Serial.print("Buffered to slot ");
          Serial.println(i);
        #endif
        break;
      }
    }
  }
}

void printMatrix() {
  for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
    for (int colIndex=0; colIndex < colCount; colIndex++) {
      if (keyChanged(colIndex, rowIndex) && isPrintableKey(colIndex, rowIndex)) {
        char toPrint;
        char other1;
        char other2;
        if (cursorMode) {
          toPrint = keyboard_cursor[colIndex][rowIndex];
          other1  = keyboard[colIndex][rowIndex];
          other2  = keyboard_symbol[colIndex][rowIndex];
        } else if (stickySym != STICKY_STATUS_OPEN || keyActive(K_SYM)) {
          toPrint = keyboard_symbol[colIndex][rowIndex];
          other1  = keyboard[colIndex][rowIndex];
          other2  = keyboard_cursor[colIndex][rowIndex];
        } else {
          toPrint = keyboard[colIndex][rowIndex];
          other1  = keyboard_symbol[colIndex][rowIndex];
          other2  = keyboard_cursor[colIndex][rowIndex];
        }
  
        // Workaround for left shift key dropping while being pressed
        if (keyActive(K_LSH) && rowIndex!=1 && colIndex!=6) {
          keyboardEventBuffered(KEY_LEFT_SHIFT, KEY_RELEASED);
          keyboardEventBuffered(KEY_LEFT_SHIFT, KEY_PRESSED);
        }
        if (keyPressed(colIndex, rowIndex)) {
          if (toPrint!=NULL) {
            if (stickyLsh != STICKY_STATUS_OPEN && !keyActive(K_LSH)) { keyboardEventBuffered(KEY_LEFT_SHIFT, KEY_PRESSED); }
            if (stickyRsh != STICKY_STATUS_OPEN && !keyActive(K_RSH)) { keyboardEventBuffered(KEY_RIGHT_SHIFT, KEY_PRESSED); }
            if (stickyCtrl!= STICKY_STATUS_OPEN && !keyActive(K_MIC)) { keyboardEventBuffered(KEY_LEFT_CTRL, KEY_PRESSED); }
            if (stickyAlt != STICKY_STATUS_OPEN && !keyActive(K_ALT)) { keyboardEventBuffered(KEY_LEFT_ALT, KEY_PRESSED); }
            keyboardEventBuffered(toPrint, KEY_PRESSED);
          }
        } else {
          if (toPrint!=NULL) {
            keyboardEventBuffered(toPrint, KEY_RELEASED);
          }
          if (other1!=NULL) {
            keyboardEventBuffered(other1, KEY_RELEASED);
          }
          if (other2!=NULL) {
            keyboardEventBuffered(other2, KEY_RELEASED);
          }
          if (!keyActive(K_LSH)) { keyboardEventBuffered(KEY_LEFT_SHIFT, KEY_RELEASED); }
          if (!keyActive(K_RSH)) { keyboardEventBuffered(KEY_RIGHT_SHIFT, KEY_RELEASED); }
          if (!keyActive(K_MIC)) { keyboardEventBuffered(KEY_LEFT_CTRL, KEY_RELEASED); }
          if (!keyActive(K_ALT)) { keyboardEventBuffered(KEY_LEFT_ALT, KEY_RELEASED); }
        }
      }
    }
  }
  sei();
}

void loop() {
  unsigned long startms = millis();
  #if BOARD_TYPE == ESP32
  if (bleKeyboard.isConnected()) {
  #endif
    if (readMatrix(startms-lastDebounceMs)) { // some key changed
      printMatrix();
      unstickKeys();

      // increase backlight if mic key + sym key is pressed
      if (keyActive(K_MIC) && keyPressed(K_SYM)) {
        changeKeyboardBacklight(keyboardLightSteps, keyboardLightSteps, true);
      }
      // decrease backlight if mic key + right shift key is pressed
      if (keyActive(K_MIC) && keyPressed(K_RSH)) {
        changeKeyboardBacklight(-keyboardLightSteps, -keyboardLightSteps, true);
      }

      // instant powersave mode if SYM + Enter is pressed
      if (keyActive(K_SYM) && keyPressed(K_ENTER)) {
        idleTimeout = 0;
        backlightTimeout = 0;
      }
    
      if ((keyPressed(K_LSH) && keyActive(K_RSH)) || (keyActive(K_LSH) && keyPressed(K_RSH))) {
        cursorMode = !cursorMode;
        resetStickyKeys();
      }
    } else {
      if (KEYBOARD_CONNECTION_READY && keyPressBuffer[0]!=0) {
        flushKeyboardBuffer();
      }
    }
    lastDebounceMs = startms;
    #ifdef SERIAL_DEBUG_LOG
      //Serial.print("matrix: ");
      //Serial.println(millis()-startms);
    #endif
  #if BOARD_TYPE == ESP32
  }
  #endif

  #ifdef BLINK_IN_CURSOR_MODE
    if (backlightTimeout>=millis()) {
      if (cursorMode && blinkTimeout<millis()) {
        blinkTimeout = millis() + BLINK_TIMEOUT;
        blinkState = !blinkState;
      } else if (!cursorMode) {
        blinkState = true;
      }
      changeKeyboardBacklight(0, 0, blinkState);
    }
  #endif
  #ifdef LED0_IN_CURSOR_MODE
    digitalWrite(LED0_PIN, cursorMode);
  #endif

  if (backlightTimeout<millis()) {
    #ifdef SERIAL_DEBUG_LOG
      Serial.println("Backlight off");
    #endif
    changeKeyboardBacklight(0, 0, false);
  }

  if (idleTimeout<millis()) {
    #ifdef SERIAL_DEBUG_LOG
      Serial.println("Sleep");
    #endif
    resetStickyKeys();
    firstSleep = false;
    #ifdef POWERSAVE_ARDUINO_POWERDOWN
      if (!connectionNeedsReinit1) {
        Keyboard.end();
        USBDevice.detach();
        
        //USBCON &= ~(1 << USBE);
        //USBCON |= (1 << FRZCLK);
        
        delay(300);
        connectionNeedsReinit1 = true;
      }
      LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);
    #endif
    #ifdef POWERSAVE_ARDUINO_CLOCKDOWN
      CLKPR = 0b10000000;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      CLKPR = ARDUINO_CLOCKDOWN_DIVISION_VALUE;
      isClockedDown = true;
    #endif
    #ifdef POWERSAVE_ARDUINO_IDLE
      power_twi_disable();
      power_adc_disable();
      power_spi_disable();
      power_usart0_disable();
      power_usart1_disable();
      if (!connectionNeedsReinit1) {
        Keyboard.end();
        USBDevice.detach();
        USBCON &= ~(1 << USBE);

        delay(300);
        while (USBDevice.isSuspended()) {}
        USBCON |= (1 << USBE);
        USBDevice.attach();
        connectionNeedsReinit1 = true;
      }
      LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON);
    #endif
    #ifdef POWERSAVE_ESP32_LIGHT_SLEEP
      if (!connectionNeedsReinit1) {
        btStop();
        connectionNeedsReinit1 = true;
      }
      esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
      esp_light_sleep_start();
    #endif
  }
  #ifdef POWERSAVE_ARDUINO_IDLE_DURING_ACTIVE
    if (!firstSleep) {
      LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON);
    }
  #endif
  #ifdef SERIAL_DEBUG_LOG
    //Serial.print("total:  ");
    //Serial.println(millis()-startms);
  #endif
}
