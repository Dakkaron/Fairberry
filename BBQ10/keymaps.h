/*
 * Keymap storage for the Fairberry keyboard.
 *
 * Each key is a KeyEntry: a raw HID usage id plus a modifier bitmask.
 * Modifier usages (0xE0..0xE7) are never stored in the usage field; they are
 * normalized into the modifier bitmask (bit 0 = left ctrl .. bit 7 = right gui)
 * so the HID-Project raw-keycode API can apply them directly.
 *
 * Three modes exist per key position: normal, symbol and cursor.
 * The active keymap is kept in RAM and can be updated over the serial
 * protocol (see protocol.h). On AVR boards it is persisted to EEPROM.
 */

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

struct KeyEntry {
  uint8_t usage; // raw HID usage id, 0 == none
  uint8_t mods;  // modifier bitmask (MOD_* values)
};

#define KEYMAP_ENTRIES (MODE_COUNT * colCount * rowCount)

// Raw HID usage ids (HID usage page 0x07). Letters a-z = 0x04..0x1D,
// digits 1-9 = 0x1E..0x26, 0 = 0x27, arrows = 0x4F..0x52, F1..F12 = 0x3A..0x45.
#define KEYMAP_F3 0x3C
#define KEYMAP_F4 0x3D
#define KEYMAP_F5 0x3E
#define KEYMAP_F6 0x3F
#define KEYMAP_F7 0x40
#define KEYMAP_F8 0x41
#define KEYMAP_F15 0x6A

#define KE(ch) KeyEntry{(uint8_t)(0x04 + (ch) - 'a'), 0}
#define KD(d) KeyEntry{(uint8_t)(0x1E + ((d) == 0 ? 9 : (d) - 1)), 0}
#define KS(hidUsage) KeyEntry{hidUsage, 0}
#define KSH(hidUsage) KeyEntry{hidUsage, MOD_LSHIFT}
#define KM(modbits) KeyEntry{0, modbits}
#define KNUL KeyEntry{0, 0}

const KeyEntry keymap_defaults[MODE_COUNT][colCount][rowCount] PROGMEM = {
  // normal mode
  {
    KE('q'), KE('w'), KNUL, KE('a'), KM(MOD_LALT), KS(0x2C), KM(MOD_LCTRL),   // sym, ALT, mic
    KE('e'), KE('s'), KE('d'), KE('p'), KE('x'), KE('z'), KM(MOD_LSHIFT),     // left shift
    KE('r'), KE('g'), KE('t'), KM(MOD_RSHIFT), KE('v'), KE('c'), KE('f'),     // right shift
    KE('u'), KE('h'), KE('y'), KS(0x28), KE('b'), KE('n'), KE('j'),           // Enter
    KE('o'), KE('l'), KE('i'), KS(0x2A), KSH(0x21), KE('m'), KE('k')          // Backspace, $
  },
  // symbol mode
  {
    KSH(0x20), KD(1), KNUL, KSH(0x25), KS(0x2B), KS(0x2C), KD(0),             // #, *, TAB, 0
    KD(2), KD(4), KD(5), KSH(0x1F), KD(8), KD(7), KM(MOD_LSHIFT),             // @
    KD(3), KS(0x38), KSH(0x26), KM(MOD_RSHIFT), KSH(0x38), KD(9), KD(6),      // / ( ?
    KSH(0x2D), KSH(0x33), KSH(0x27), KNUL, KSH(0x1E), KS(0x36), KS(0x33),     // _ : ) ! , ;
    KSH(0x2E), KSH(0x34), KS(0x2D), KS(0x2A), KNUL, KS(0x37), KS(0x34)        // + " - . '
  },
  // cursor mode
  {
    KS(0x29), KS(0x52), KNUL, KS(0x50), KS(0x2B), KS(0x2C), KD(0),            // ESC, up, left, TAB, 0
    KD(2), KS(0x51), KS(0x4F), KS(KEYMAP_F15), KD(8), KD(7), KM(MOD_LSHIFT),  // down, right, F15
    KD(3), KS(0x38), KSH(0x26), KM(MOD_RSHIFT), KSH(0x38), KD(9), KD(6),      // / ( ?
    KS(KEYMAP_F7), KSH(0x33), KSH(0x27), KS(KEYMAP_F4), KSH(0x1E), KS(0x36), KS(0x33),
    KS(KEYMAP_F6), KSH(0x34), KS(KEYMAP_F8), KS(KEYMAP_F3), KS(KEYMAP_F5), KS(0x37), KS(0x34)
  }
};

// Active keymap in RAM: keymap[mode][col][row]
KeyEntry keymap[MODE_COUNT][colCount][rowCount];

inline bool keyEntryEmpty(const KeyEntry& e) {
  return e.usage == 0 && e.mods == 0;
}

void loadFactoryDefaults() {
  memcpy_P(&keymap[0][0][0], &keymap_defaults[0][0][0], sizeof(keymap));
}

#ifdef __AVR__
#include <EEPROM.h>

#define KEYMAP_MAGIC1 'F'
#define KEYMAP_MAGIC2 'K'
#define KEYMAP_STORAGE_VERSION 1
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
    loaded[i].usage = EEPROM.read(KEYMAP_EEPROM_ADDR + 4 + i * 2);
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
    EEPROM.update(KEYMAP_EEPROM_ADDR + 4 + i * 2, current[i].usage);
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
