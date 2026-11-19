/*
 * Keymap storage for the Fairberry keyboard.
 *
 * Each key is a KeyEntry: a keymap value plus a modifier bitmask. The value
 * domain is the one the Arduino Keyboard library uses (Keyboard.h,
 * github.com/arduino-libraries/Keyboard): a printable character is stored as
 * its ASCII code, everything else as a Keyboard.h constant, where 128..135 are
 * the modifiers and 136..255 are HID usage page 0x07 ids as value - 136.
 * Sending a raw usage id instead of the value makes the layout lookup treat it
 * as ASCII, so e.g. Backspace (42) prints "*" because 42 is the ASCII code of
 * '*'. The Android app uses the same domain.
 *
 * Modifier values (128..135) are normalized into the modifier bitmask
 * (bit 0 = left ctrl .. bit 7 = right gui) and never stored as values.
 *
 * Values 136..175 name no Keyboard.h constant, so that block is reserved for HID
 * consumer page (0x0C) keys, whose usages do not fit in one byte; see
 * consumerUsageMap. Values 252..255 are the four constants Keyboard.h leaves
 * undefined above that block, and they name actions the firmware performs itself
 * rather than a usage sent to the host. The storage format is unchanged.
 *
 * Three modes exist per key position: normal, symbol and cursor.
 * The active keymap is kept in RAM and can be updated over the serial
 * protocol (see protocol.h). On AVR boards it is persisted to EEPROM.
 */

#ifndef FAIRBERRY_KEYMAPS_H
#define FAIRBERRY_KEYMAPS_H

#include <Arduino.h>

#define rowCount 7
#define colCount 5

#define MODE_NORMAL 0
#define MODE_SYMBOL 1
#define MODE_CURSOR 2
#define MODE_COUNT 3

// Modifier bitmask bits, matching the HID keyboard modifier usage order
#define MOD_LCTRL 0x01
#define MOD_LSHIFT 0x02
#define MOD_LALT 0x04
#define MOD_LGUI 0x08
#define MOD_RCTRL 0x10
#define MOD_RSHIFT 0x20
#define MOD_RALT 0x40
#define MOD_RGUI 0x80

// Keyboard.h constants, under a KV_ prefix so they cannot collide with the
// identically named KeyboardKeycode members of the HID-Project layout enum.
#define KV_LCTRL 0x80
#define KV_LSHIFT 0x81
#define KV_LALT 0x82
#define KV_LGUI 0x83
#define KV_RCTRL 0x84
#define KV_RSHIFT 0x85
#define KV_RALT 0x86
#define KV_RGUI 0x87

#define KV_RETURN 0xB0
#define KV_ESC 0xB1
#define KV_BACKSPACE 0xB2
#define KV_TAB 0xB3
#define KV_PAUSE 0xD0
#define KV_INSERT 0xD1
#define KV_HOME 0xD2
#define KV_PAGE_UP 0xD3
#define KV_DELETE 0xD4
#define KV_END 0xD5
#define KV_PAGE_DOWN 0xD6
#define KV_RIGHT_ARROW 0xD7
#define KV_LEFT_ARROW 0xD8
#define KV_DOWN_ARROW 0xD9
#define KV_UP_ARROW 0xDA
#define KV_NUM_LOCK 0xDB
#define KV_KP_SLASH 0xDC
#define KV_KP_ASTERISK 0xDD
#define KV_KP_MINUS 0xDE
#define KV_KP_PLUS 0xDF
#define KV_KP_ENTER 0xE0
#define KV_MENU 0xED
#define KV_CAPS_LOCK 0xC1
#define KV_PRINT_SCREEN 0xCE
#define KV_SCROLL_LOCK 0xCF
#define KV_F1 0xC2
#define KV_F2 0xC3
#define KV_F3 0xC4
#define KV_F4 0xC5
#define KV_F5 0xC6
#define KV_F6 0xC7
#define KV_F7 0xC8
#define KV_F8 0xC9
#define KV_F9 0xCA
#define KV_F10 0xCB
#define KV_F11 0xCC
#define KV_F12 0xCD
#define KV_F13 0xF0
#define KV_F14 0xF1
#define KV_F15 0xF2
#define KV_F16 0xF3
#define KV_F17 0xF4
#define KV_F18 0xF5
#define KV_F19 0xF6
#define KV_F20 0xF7
#define KV_F21 0xF8
#define KV_F22 0xF9
#define KV_F23 0xFA
#define KV_F24 0xFB

// Consumer page keys have no Keyboard.h constant and their usage is 16 bit, so
// they use the value block Keyboard.h leaves undefined (136..175). Android maps
// these usages to the home screen, back and the volume controls.
#define KV_CONSUMER_BASE 136
#define KV_CONSUMER_COUNT 40
#define KV_ANDROID_HOME 136
#define KV_ANDROID_BACK 137
#define KV_VOLUME_MUTE 138
#define KV_VOLUME_UP 139
#define KV_VOLUME_DOWN 140
#define KV_APP_SWITCH 141     // usage 0x01A2, which hid-input reports as Linux

// Device actions are not HID usages: they name things the firmware does itself,
// and they are only reachable as a combination output.
#define KV_ACTION_BASE 252
#define KV_ACTION_BACKLIGHT_DOWN 252
#define KV_ACTION_BACKLIGHT_UP 253

struct KeyEntry {
  uint8_t value; // ASCII code for text, otherwise a Keyboard.h constant; 0 == none
  uint8_t mods;  // modifier bitmask (MOD_* values)
};

#define KEYMAP_ENTRIES (MODE_COUNT * colCount * rowCount)

// HID usage page 0x07 for printable ASCII (US layout), bit 7 marks the
// characters that need shift; identical to KeyboardLayout_en_US, which is what
// the Keyboard library uses to send an ASCII value.
static const uint8_t asciiUsageMap[128] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2a, 0x2b, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x2c, 0x1e | 0x80, 0x34 | 0x80, 0x20 | 0x80, 0x21 | 0x80, 0x22 | 0x80, 0x24 | 0x80, 0x34,
  0x26 | 0x80, 0x27 | 0x80, 0x25 | 0x80, 0x2e | 0x80, 0x36, 0x2d, 0x37, 0x38,
  0x27, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
  0x33 | 0x80, 0x33, 0x36 | 0x80, 0x2e, 0x37 | 0x80, 0x38 | 0x80, 0x1f | 0x80,
  0x04 | 0x80, 0x05 | 0x80, 0x06 | 0x80, 0x07 | 0x80, 0x08 | 0x80, 0x09 | 0x80, 0x0a | 0x80,
  0x0b | 0x80, 0x0c | 0x80, 0x0d | 0x80, 0x0e | 0x80, 0x0f | 0x80, 0x10 | 0x80, 0x11 | 0x80,
  0x12 | 0x80, 0x13 | 0x80, 0x14 | 0x80, 0x15 | 0x80, 0x16 | 0x80, 0x17 | 0x80, 0x18 | 0x80,
  0x19 | 0x80, 0x1a | 0x80, 0x1b | 0x80, 0x1c | 0x80, 0x1d | 0x80,
  0x2f, 0x31, 0x30, 0x23 | 0x80, 0x2d | 0x80, 0x35,
  0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
  0x2f | 0x80, 0x31 | 0x80, 0x30 | 0x80, 0x35 | 0x80, 0x00
};

// Value - KV_CONSUMER_BASE -> consumer page usage, 0 for the unassigned slots.
static const uint16_t consumerUsageMap[KV_CONSUMER_COUNT] PROGMEM = {
  0x0223, 0x0224, 0x00E2, 0x00E9, 0x00EA, 0x01A2,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000
};

inline bool isConsumerValue(uint8_t value) {
  return value >= KV_CONSUMER_BASE && value < KV_CONSUMER_BASE + KV_CONSUMER_COUNT;
}

inline uint16_t consumerUsage(uint8_t value) {
  return pgm_read_word(&consumerUsageMap[value - KV_CONSUMER_BASE]);
}

inline bool isDeviceAction(uint8_t value) {
  return value >= KV_ACTION_BASE && value <= 255;
}

// HID usage to send for an entry; *needsShift reports the shift modifier the
// character itself requires, on top of entry.mods.
inline uint8_t keyEntryUsage(const KeyEntry& entry, bool& needsShift) {
  needsShift = false;
  if (isConsumerValue(entry.value)) {
    return 0; // consumer keys are sent on the consumer report, see consumerUsage
  }
  if (isDeviceAction(entry.value)) {
    return 0; // device actions are performed by the firmware, not sent to the host
  }
  if (entry.value < 128) {
    uint8_t mapped = pgm_read_byte(&asciiUsageMap[entry.value]);
    if (mapped & 0x80) {
      needsShift = true;
    }
    return mapped & 0x7F;
  }
  if (entry.value < 136) {
    return 0; // modifier values live in the mods bitmask
  }
  return (uint8_t)(entry.value - 136);
}

#define KC(ch) KeyEntry{(uint8_t)(ch), 0}
#define KV(kv) KeyEntry{(uint8_t)(kv), 0}
#define KM(modbits) KeyEntry{0, modbits}
#define KNUL KeyEntry{0, 0}

const KeyEntry keymap_defaults[MODE_COUNT][colCount][rowCount] PROGMEM = {
  // normal mode
  {
    KC('q'), KC('w'), KNUL, KC('a'), KM(MOD_LALT), KC(' '), KM(MOD_LCTRL),            // sym, ALT, mic
    KC('e'), KC('s'), KC('d'), KC('p'), KC('x'), KC('z'), KM(MOD_LSHIFT),             // left shift
    KC('r'), KC('g'), KC('t'), KM(MOD_RSHIFT), KC('v'), KC('c'), KC('f'),             // right shift
    KC('u'), KC('h'), KC('y'), KV(KV_RETURN), KC('b'), KC('n'), KC('j'),              // Enter
    KC('o'), KC('l'), KC('i'), KV(KV_BACKSPACE), KC('$'), KC('m'), KC('k')            // Backspace
  },
  // symbol mode
  {
    KC('#'), KC('1'), KNUL, KC('*'), KV(KV_TAB), KC(' '), KC('0'),                   // sym, ALT, SPACE
    KC('2'), KC('4'), KC('5'), KC('@'), KC('8'), KC('7'), KM(MOD_LSHIFT),
    KC('3'), KC('/'), KC('('), KM(MOD_RSHIFT), KC('?'), KC('9'), KC('6'),
    KC('_'), KC(':'), KC(')'), KNUL, KC('!'), KC(','), KC(';'),
    KC('+'), KC('"'), KC('-'), KV(KV_BACKSPACE), KNUL, KC('.'), KC('\'')
  },
  // cursor mode
  {
    KV(KV_ESC), KV(KV_UP_ARROW), KNUL, KV(KV_LEFT_ARROW), KV(KV_TAB), KC(' '), KC('0'),
    KC('2'), KV(KV_DOWN_ARROW), KV(KV_RIGHT_ARROW), KV(KV_F15), KC('8'), KC('7'), KM(MOD_LSHIFT),
    KC('3'), KC('/'), KC('('), KM(MOD_RSHIFT), KC('?'), KC('9'), KC('6'),
    KV(KV_F7), KC(':'), KC(')'), KV(KV_F4), KC('!'), KC(','), KC(';'),
    KV(KV_F6), KC('"'), KV(KV_F8), KV(KV_F3), KV(KV_F5), KC('.'), KC('\'')
  }
};

// Active keymap in RAM: keymap[mode][col][row]
KeyEntry keymap[MODE_COUNT][colCount][rowCount];

inline bool keyEntryEmpty(const KeyEntry& e) {
  return e.value == 0 && e.mods == 0;
}

void loadFactoryDefaults() {
  memcpy_P(&keymap[0][0][0], &keymap_defaults[0][0][0], sizeof(keymap));
}

#ifdef __AVR__
#include <EEPROM.h>

#define KEYMAP_MAGIC1 'F'
#define KEYMAP_MAGIC2 'K'
// Storage version 2 switched the entry values from raw HID usages to the
// Keyboard.h value domain, so older stored keymaps are not accepted.
#define KEYMAP_STORAGE_VERSION 2
#define KEYMAP_EEPROM_ADDR 0
// layout: 'F' 'K' version reserved | 2 bytes per entry | CRC16 big-endian
#define KEYMAP_EEPROM_SIZE (4 + KEYMAP_ENTRIES * 2 + 2)

static uint16_t crc16Update(uint16_t crc, const uint8_t* data, int len) {
  for (int i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (int b = 0; b < 8; b++) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
  }
  return crc;
}

static uint16_t keymapStorageCrc(uint8_t version, const KeyEntry* entries) {
  uint8_t header[2] = {version, 0};
  uint16_t crc = crc16Update(0xFFFF, header, 2);
  crc = crc16Update(crc, (const uint8_t*)entries, KEYMAP_ENTRIES * 2);
  return crc;
}

// A corrupt or empty EEPROM leaves the current RAM keymap untouched.
bool loadKeymapFromEEPROM() {
  if (EEPROM.read(KEYMAP_EEPROM_ADDR) != KEYMAP_MAGIC1 ||
      EEPROM.read(KEYMAP_EEPROM_ADDR + 1) != KEYMAP_MAGIC2 ||
      EEPROM.read(KEYMAP_EEPROM_ADDR + 2) != KEYMAP_STORAGE_VERSION) {
    return false;
  }

  KeyEntry loaded[KEYMAP_ENTRIES];
  for (int i = 0; i < KEYMAP_ENTRIES; i++) {
    loaded[i].value = EEPROM.read(KEYMAP_EEPROM_ADDR + 4 + i * 2);
    loaded[i].mods = EEPROM.read(KEYMAP_EEPROM_ADDR + 4 + i * 2 + 1);
  }

  uint16_t stored = ((uint16_t)EEPROM.read(KEYMAP_EEPROM_ADDR + KEYMAP_EEPROM_SIZE - 2) << 8) |
                    EEPROM.read(KEYMAP_EEPROM_ADDR + KEYMAP_EEPROM_SIZE - 1);
  if (keymapStorageCrc(KEYMAP_STORAGE_VERSION, loaded) != stored) {
    return false;
  }

  memcpy(&keymap[0][0][0], loaded, sizeof(keymap));
  return true;
}

void writeKeymapToEEPROM() {
  KeyEntry current[KEYMAP_ENTRIES];
  memcpy(current, &keymap[0][0][0], sizeof(keymap));

  EEPROM.update(KEYMAP_EEPROM_ADDR, KEYMAP_MAGIC1);
  EEPROM.update(KEYMAP_EEPROM_ADDR + 1, KEYMAP_MAGIC2);
  EEPROM.update(KEYMAP_EEPROM_ADDR + 2, KEYMAP_STORAGE_VERSION);
  EEPROM.update(KEYMAP_EEPROM_ADDR + 3, 0);

  for (int i = 0; i < KEYMAP_ENTRIES; i++) {
    EEPROM.update(KEYMAP_EEPROM_ADDR + 4 + i * 2, current[i].value);
    EEPROM.update(KEYMAP_EEPROM_ADDR + 4 + i * 2 + 1, current[i].mods);
  }

  uint16_t crc = keymapStorageCrc(KEYMAP_STORAGE_VERSION, current);
  EEPROM.update(KEYMAP_EEPROM_ADDR + KEYMAP_EEPROM_SIZE - 2, crc >> 8);
  EEPROM.update(KEYMAP_EEPROM_ADDR + KEYMAP_EEPROM_SIZE - 1, crc & 0xFF);
}
#else
bool loadKeymapFromEEPROM() {
  return false;
}

void writeKeymapToEEPROM() {
}
#endif

#endif
