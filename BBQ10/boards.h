#if BOARD_TYPE == FAIRBERRY_V0_1_1 || BOARD_TYPE == FAIRBERRY_V0_2_0 || BOARD_TYPE == FAIRBERRY_V0_3_0 || BOARD_TYPE == ARDUINO || BOARD_TYPE==BEETLE
  #define CHIP_TYPE CHIP_ATMEGA32U4
#elif BOARD_TYPE == ESP32
  #define CHIP_ESP32
#else
  #error "No valid BOARD_TYPE selected. Needs to be any of FAIRBERRY_V0_1_1, FAIRBERRY_V0_2_0, FAIRBERRY_V0_3_0, ARDUINO, BEETLE, ESP32"
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

#if BOARD_TYPE == FAIRBERRY_V0_1_1 || BOARD_TYPE == FAIRBERRY_V0_2_0 || BOARD_TYPE == FAIRBERRY_V0_3_0
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
  #include <avr/power.h>

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

#if BOARD_TYPE == FAIRBERRY_V0_3_0
  byte rows[] = {A1,7,8,9,10,5,A0};
  //byte cols[] = {4,30,1,12,6}; // Arduino pinout for reference only
  byte cols[] = {4,5,3,6,7}; // Using PORTD numbering instead of Arduino numbering

  #define KEYBOARD_LIGHT_PIN_1 13
  #define KEYBOARD_LIGHT_PIN_2 11

  #define FASTCHECK
  #define FASTROWCHECK_B 0b01110000
  #define FASTROWCHECK_C 0b01000000
  #define FASTROWCHECK_E 0b01000000
  #define FASTROWCHECK_F 0b11000000

  #define FASTCOLCHECK_D 0b11111000
  
  #define FASTCOLS
  #define FASTCOLS_PORT PORTD
  #define FASTCOLS_PINCONFIG DDRD
#elif BOARD_TYPE == FAIRBERRY_V0_2_0
  byte rows[] = {A1,7,8,9,10,5,A0};
  //byte cols[] = {4,30,1,12,6}; // Arduino pinout for reference only
  byte cols[] = {4,5,3,6,7}; // Using PORTD numbering instead of Arduino numbering

  #define KEYBOARD_LIGHT_PIN_1 13
  #define LED0_PIN 0
  #define LED1_PIN 11

  #define FASTCHECK
  #define FASTROWCHECK_B 0b01110000
  #define FASTROWCHECK_C 0b01000000
  #define FASTROWCHECK_E 0b01000000
  #define FASTROWCHECK_F 0b11000000

  #define FASTCOLCHECK_D 0b11111000
  
  #define FASTCOLS
  #define FASTCOLS_PORT PORTD
  #define FASTCOLS_PINCONFIG DDRD
#elif BOARD_TYPE == FAIRBERRY_V0_1_1
  byte rows[] = {A1,A2,8,9,10,5,A0};
  //byte cols[] = {4,30,1,12,6}; // Arduino pinout for reference only
  byte cols[] = {4,5,3,6,7}; // Using PORTD numbering instead of Arduino numbering

  #define FASTCHECK
  #define FASTROWCHECK_B 0b01110000
  #define FASTROWCHECK_C 0b01000000
  #define FASTROWCHECK_F 0b11100000

  #define FASTCOLCHECK_D 0b11111000

  #define KEYBOARD_LIGHT_PIN_1 13
  #define LED0_PIN 0
  #define LED1_PIN 11
  
  #define FASTCOLS
  #define FASTCOLS_PORT PORTD
  #define FASTCOLS_PINCONFIG DDRD
#elif BOARD_TYPE == ARDUINO
  byte rows[] = {9,8,7,6,5,4,A2};
  byte cols[] = {A1,A0,15,14,16};

  #define KEYBOARD_LIGHT_PIN_1 10
#elif BOARD_TYPE == BEETLE
  byte rows[] = {A5,11,10,9,18,19,20};
  byte cols[] = {3,2,0,14,15};

  #define KEYBOARD_LIGHT_PIN_1 10
#elif BOARD_TYPE == ESP32
  #define RESET_PIN 33
  byte rows[] = {27,25,32,4,0,2,22};
  byte cols[] = {5,23,19,18,26};

  #define KEYBOARD_LIGHT_PIN_1 35
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

#define STICKY_STATUS_OPEN 0
#define STICKY_STATUS_STICKY 1
#define STICKY_STATUS_LOCKED 2