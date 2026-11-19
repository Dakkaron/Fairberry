/*
 * Combination arguments take the modifier as one bitmask bit (bit 0 = left ctrl .. bit 7 =
 * right gui), the key as its physical matrix cell, then three output value/mods pairs.
 */

#ifdef __AVR__

#include <avr/wdt.h>
#include "combos.h"

#define FIRMWARE_VERSION "3.2"
#define KEYMAP_PROTOCOL_VERSION 4

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

void printComboEntry(int slot) {
  int pos = combos[slot].key;
  Serial.print('G');
  Serial.print(' ');
  Serial.print(combos[slot].mod);
  Serial.print(' ');
  Serial.print(pos / rowCount);
  Serial.print(' ');
  Serial.print(pos % rowCount);
  for (int out = 0; out < COMBO_OUTPUTS; out++) {
    Serial.print(' ');
    Serial.print(combos[slot].outputs[out].value);
    Serial.print(' ');
    Serial.print(combos[slot].outputs[out].mods);
  }
  Serial.println();
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
      case 'C':
        for (int i = 0; i < COMBO_MAX; i++) {
          if (combos[i].mod != 0) {
            printComboEntry(i);
          }
        }
        printProtocolOK();
        break;
      case 'N': {
        int mod = readProtocolNumber();
        int col = readProtocolNumber();
        int row = readProtocolNumber();
        KeyEntry outputs[COMBO_OUTPUTS];
        bool valid = col >= 0 && col < colCount && row >= 0 && row < rowCount;
        for (int out = 0; out < COMBO_OUTPUTS && valid; out++) {
          int value = readProtocolNumber();
          int mods = readProtocolNumber();
          if (value < 0 || mods < 0) {
            valid = false;
            break;
          }
          outputs[out] = KeyEntry{(uint8_t)value, (uint8_t)mods};
        }
        if (valid && addCombo(mod, keyIndex(col, row), outputs)) {
          printProtocolOK();
        } else {
          printProtocolERR();
        }
        break;
      }
      case 'X': {
        int mod = readProtocolNumber();
        int col = readProtocolNumber();
        int row = readProtocolNumber();
        if (removeCombo(mod, keyIndex(col, row))) {
          printProtocolOK();
        } else {
          printProtocolERR();
        }
        break;
      }
      case 'T':
        writeCombosToEEPROM();
        printProtocolOK();
        break;
      case 'V':
        loadComboDefaults();
        printProtocolOK();
        break;
      case 'Y':
        if (loadCombosFromEEPROM()) {
          printProtocolOK();
        } else {
          printProtocolERR();
        }
        break;
      case 'P':
        // No response: the host sends this to keep the board awake while its app
        // is in the foreground, so a reply would be noise on the CDC link.
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
