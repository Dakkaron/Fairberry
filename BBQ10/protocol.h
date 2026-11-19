/*
 * Text line protocol for updating the keymap over USB CDC (Serial).
 *
 * Commands (uppercase letter, optional space-separated decimal arguments):
 *   I                 -> INFO <firmware> <protocol> <storage>
 *   L                 -> one K line per keymap entry (all modes)
 *   R <mode>          -> K lines for one mode
 *   M <mode> <col> <row> <value> <mods>  -> set one entry in RAM
 *   D                 -> reload keymap from EEPROM
 *   F                 -> load factory defaults into RAM
 *   W                 -> write RAM keymap to EEPROM
 *   T                 -> toggle cursor mode
 *   Z                 -> soft reset (restart the sketch)
 *   B                 -> reboot into the USB bootloader (Caterina) for reflashing
 * Responses: OK, ERR, K <mode> <col> <row> <value> <mods>, INFO ...
 *
 * Values follow the Arduino Keyboard library (Keyboard.h): printable characters
 * are their ASCII code, everything else is a Keyboard.h constant (128..135 are
 * the modifiers, 136..255 map to HID usage page 0x07 as value - 136).
 *
 * Only available on AVR boards: the protocol lives on the same USB CDC
 * interface as the keyboard HID interface and the Caterina bootloader.
 */

#ifdef __AVR__

#include <avr/wdt.h>

#define FIRMWARE_VERSION "3.1"
#define KEYMAP_PROTOCOL_VERSION 2

extern bool cursorMode;
extern volatile unsigned long idleTimeout;
extern void(* resetFunc)(void);

void printKeyEntry(int mode, int col, int row) {
  KeyEntry e = keymap[mode][col][row];
  Serial.print('K');
  Serial.print(' ');
  Serial.print(mode);
  Serial.print(' ');
  Serial.print(col);
  Serial.print(' ');
  Serial.print(row);
  Serial.print(' ');
  Serial.print(e.value);
  Serial.print(' ');
  Serial.println(e.mods);
}

void printKeymapMode(int mode) {
  for (int col = 0; col < colCount; col++) {
    for (int row = 0; row < rowCount; row++) {
      printKeyEntry(mode, col, row);
    }
  }
}

int readProtocolNumber() {
  int value = -1;
  while (Serial.available()) {
    char c = Serial.read();
    if (value < 0 && c == ' ') {
      continue;
    }
    if (c == ' ' || c == '\r' || c == '\n') {
      break;
    }
    if (c < '0' || c > '9') {
      return -1;
    }
    int digit = c - '0';
    if (value < 0) {
      value = digit;
    } else {
      value = value * 10 + digit;
    }
    if (value > 255) {
      return -1;
    }
  }
  return value;
}

void printProtocolOK() {
  Serial.println("OK");
}

void printProtocolERR() {
  Serial.println("ERR");
}

bool setKeymapEntry(int mode, int col, int row, int value, int mods) {
  if (mode < 0 || mode >= MODE_COUNT || col < 0 || col >= colCount ||
      row < 0 || row >= rowCount || value < 0 || value > 255 || mods < 0 || mods > 255) {
    return false;
  }

  // Keyboard.h modifier constants (128..135) are stored as modifier bitmask
  // bits instead of as values.
  if (value >= 128 && value <= 135) {
    mods |= (1 << (value - 128));
    value = 0;
  }

  keymap[mode][col][row].value = (uint8_t)value;
  keymap[mode][col][row].mods = (uint8_t)mods;
  return true;
}

void printProtocolInfo() {
  Serial.print("INFO ");
  Serial.print(FIRMWARE_VERSION);
  Serial.print(' ');
  Serial.print(KEYMAP_PROTOCOL_VERSION);
  Serial.print(' ');
  Serial.println("eeprom");
}

// Caterina keeps its bootloader active after a watchdog reset only if the
// magic boot key is present in RAM at 0x0800 (see Caterina.c).
void enterBootloaderMode() {
  printProtocolOK();
  Serial.flush();
  delay(50); // give the CDC transmit buffer time to drain

  *(volatile uint16_t*)0x0800 = 0x7777;

  cli();
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDE);
  while (1) {
  }
}

void handleSerial() {
  while (Serial.available()) {
    char command = Serial.read();
    switch (command) {
      case 'I':
        printProtocolInfo();
        break;
      case 'L':
        for (int mode = 0; mode < MODE_COUNT; mode++) {
          printKeymapMode(mode);
        }
        break;
      case 'R': {
        int mode = readProtocolNumber();
        if (mode >= 0 && mode < MODE_COUNT) {
          printKeymapMode(mode);
        } else {
          printProtocolERR();
        }
        break;
      }
      case 'M': {
        int mode = readProtocolNumber();
        int col = readProtocolNumber();
        int row = readProtocolNumber();
        int value = readProtocolNumber();
        int mods = readProtocolNumber();
        if (setKeymapEntry(mode, col, row, value, mods)) {
          printProtocolOK();
        } else {
          printProtocolERR();
        }
        break;
      }
      case 'D':
        if (loadKeymapFromEEPROM()) {
          printProtocolOK();
        } else {
          printProtocolERR();
        }
        break;
      case 'F':
        loadFactoryDefaults();
        printProtocolOK();
        break;
      case 'W':
        writeKeymapToEEPROM();
        printProtocolOK();
        break;
      case 'T':
        cursorMode = !cursorMode;
        printProtocolOK();
        break;
      case 'Z':
        printProtocolOK();
        Serial.flush();
        delay(50);
        resetFunc();
        break;
      case 'B':
        enterBootloaderMode();
        break;
      case '\r':
      case '\n':
        break;
      default:
        printProtocolERR();
        break;
    }

    idleTimeout = millis() + IDLE_TIMEOUT;
  }
}

#else
#define handleSerial() ;
#endif
