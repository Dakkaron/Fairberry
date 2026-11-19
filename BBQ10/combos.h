/*
 * Key combinations: modifier bit + physical key position -> up to 3 output keys.
 * A combination fires when its key is pressed while its modifier is already being sent: the
 * modifier stops reaching the host and the outputs are sent in its place until either key is
 * released. The key half is a physical matrix cell, so combinations apply in every mode, and
 * the modifier half is the bit the keyboard currently sends, so remapping the keycap that
 * acts as the modifier does not break the combination.
 * Outputs are held together as a chord, pressed in slot order and released in reverse.
 * Combos live in their own EEPROM blob so the keymap blob is not affected.
 */

#ifndef FAIRBERRY_COMBOS_H
#define FAIRBERRY_COMBOS_H

#include "constants.h"
#include "keymaps.h"

#define COMBO_MAX 40
#define COMBO_KEYS (colCount * rowCount)
#define COMBO_OUTPUTS 3

struct ComboEntry {
  uint8_t mod;   // exactly one modifier bitmask bit, 0 = empty slot
  uint8_t key;
  KeyEntry outputs[COMBO_OUTPUTS];
};

static ComboEntry combos[COMBO_MAX];

inline int keyIndex(int col, int row) {
  return col * rowCount + row;
}

inline int comboModifierBit(uint8_t mod) {
  int bit = 0;
  while (bit < 8 && ((mod >> bit) & 1) == 0) {
    bit++;
  }
  return bit;
}

inline bool comboHasOutput(const ComboEntry& combo) {
  for (int i = 0; i < COMBO_OUTPUTS; i++) {
    if (!keyEntryEmpty(combo.outputs[i])) {
      return true;
    }
  }
  return false;
}

// Outputs can need shift on top of the combination's own modifier bit.
inline uint8_t comboOutputModBits(int slot) {
  uint8_t bits = 0;
  for (int i = 0; i < COMBO_OUTPUTS; i++) {
    bool needsShift;
    keyEntryUsage(combos[slot].outputs[i], needsShift);
    bits |= combos[slot].outputs[i].mods | (needsShift ? MOD_LSHIFT : 0);
  }
  return bits;
}

// Implemented in the sketch, where the live scan state lives.
int comboFiring(int col, int row);

void clearComboSlot(int slot) {
  for (int i = 0; i < COMBO_OUTPUTS; i++) {
    combos[slot].outputs[i] = KNUL;
  }
  combos[slot].mod = 0;
  combos[slot].key = 0;
}

int findCombo(int mod, int keyPos) {
  for (int i = 0; i < COMBO_MAX; i++) {
    if (combos[i].mod == mod && combos[i].key == keyPos) {
      return i;
    }
  }
  return -1;
}

// An existing modifier + key pair updates in place because the pair must stay unique.
bool addCombo(int mod, int keyPos, const KeyEntry* outputs) {
  if (mod == 0 || (mod & (mod - 1)) != 0 || keyPos < 0 || keyPos >= COMBO_KEYS) {
    return false;
  }

  ComboEntry entry;
  entry.mod = (uint8_t)mod;
  entry.key = (uint8_t)keyPos;
  for (int i = 0; i < COMBO_OUTPUTS; i++) {
    if (outputs[i].value > 255 || outputs[i].mods > 255) {
      return false;
    }
    // Keyboard.h modifier constants (128..135) are stored as modifier bitmask
    // bits instead of as values, exactly as in setKeymapEntry.
    entry.outputs[i] = outputs[i];
    if (entry.outputs[i].value >= 128 && entry.outputs[i].value <= 135) {
      entry.outputs[i].mods |= (uint8_t)(1 << (entry.outputs[i].value - 128));
      entry.outputs[i].value = 0;
    }
  }
  if (!comboHasOutput(entry)) {
    return false;
  }

  int slot = findCombo(mod, keyPos);
  if (slot < 0) {
    for (int i = 0; i < COMBO_MAX; i++) {
      if (combos[i].mod == 0) {
        slot = i;
        break;
      }
    }
    if (slot < 0) {
      return false;
    }
  }

  combos[slot] = entry;
  return true;
}

bool removeCombo(int mod, int keyPos) {
  int slot = findCombo(mod, keyPos);
  if (slot < 0) {
    return false;
  }
  clearComboSlot(slot);
  return true;
}

void clearCombos() {
  for (int i = 0; i < COMBO_MAX; i++) {
    clearComboSlot(i);
  }
}

// Factory defaults are the stock keyboard's hardcoded brightness combos.
void loadComboDefaults() {
  clearCombos();

  KeyEntry down[COMBO_OUTPUTS] = {KNUL, KNUL, KNUL};
  down[0] = KV(KV_ACTION_BACKLIGHT_DOWN);
  addCombo(MOD_LCTRL, keyIndex(K_SYM_COL, K_SYM_ROW), down);

  KeyEntry up[COMBO_OUTPUTS] = {KNUL, KNUL, KNUL};
  up[0] = KV(KV_ACTION_BACKLIGHT_UP);
  addCombo(MOD_LCTRL, keyIndex(K_RSH_COL, K_RSH_ROW), up);
}

#ifdef __AVR__

#define COMBO_MAGIC1 'F'
#define COMBO_MAGIC2 'C'
// Storage version 1 stored one output value per combo; version 2 stores three,
// so a version 1 blob is not read as a version 2 record.
#define COMBO_STORAGE_VERSION 2
#define COMBO_EEPROM_ADDR (KEYMAP_EEPROM_ADDR + KEYMAP_EEPROM_SIZE)
// layout: 'F' 'C' version reserved | 8 bytes per slot | CRC16 big-endian
#define COMBO_EEPROM_SIZE (4 + COMBO_MAX * (2 + COMBO_OUTPUTS * 2) + 2)

static uint16_t comboStorageCrc(uint8_t version, const ComboEntry* slots) {
  uint8_t header[2] = {version, 0};
  uint16_t crc = crc16Update(0xFFFF, header, 2);
  for (int i = 0; i < COMBO_MAX; i++) {
    uint8_t record[8] = {slots[i].mod, slots[i].key};
    for (int out = 0; out < COMBO_OUTPUTS; out++) {
      record[2 + out * 2] = slots[i].outputs[out].value;
      record[3 + out * 2] = slots[i].outputs[out].mods;
    }
    crc = crc16Update(crc, record, 8);
  }
  return crc;
}

// A corrupt or empty EEPROM leaves the current combo table untouched.
bool loadCombosFromEEPROM() {
  if (EEPROM.read(COMBO_EEPROM_ADDR) != COMBO_MAGIC1 ||
      EEPROM.read(COMBO_EEPROM_ADDR + 1) != COMBO_MAGIC2 ||
      EEPROM.read(COMBO_EEPROM_ADDR + 2) != COMBO_STORAGE_VERSION) {
    return false;
  }

  ComboEntry loaded[COMBO_MAX];
  for (int i = 0; i < COMBO_MAX; i++) {
    loaded[i].mod = EEPROM.read(COMBO_EEPROM_ADDR + 4 + i * 8);
    loaded[i].key = EEPROM.read(COMBO_EEPROM_ADDR + 4 + i * 8 + 1);
    for (int out = 0; out < COMBO_OUTPUTS; out++) {
      loaded[i].outputs[out].value = EEPROM.read(COMBO_EEPROM_ADDR + 4 + i * 8 + 2 + out * 2);
      loaded[i].outputs[out].mods = EEPROM.read(COMBO_EEPROM_ADDR + 4 + i * 8 + 3 + out * 2);
    }
  }

  uint16_t stored = ((uint16_t)EEPROM.read(COMBO_EEPROM_ADDR + COMBO_EEPROM_SIZE - 2) << 8) |
                    EEPROM.read(COMBO_EEPROM_ADDR + COMBO_EEPROM_SIZE - 1);
  if (comboStorageCrc(COMBO_STORAGE_VERSION, loaded) != stored) {
    return false;
  }

  memcpy(&combos[0], loaded, sizeof(combos));
  return true;
}

void writeCombosToEEPROM() {
  ComboEntry current[COMBO_MAX];
  memcpy(&current[0], &combos[0], sizeof(combos));

  EEPROM.update(COMBO_EEPROM_ADDR, COMBO_MAGIC1);
  EEPROM.update(COMBO_EEPROM_ADDR + 1, COMBO_MAGIC2);
  EEPROM.update(COMBO_EEPROM_ADDR + 2, COMBO_STORAGE_VERSION);
  EEPROM.update(COMBO_EEPROM_ADDR + 3, 0);

  for (int i = 0; i < COMBO_MAX; i++) {
    EEPROM.update(COMBO_EEPROM_ADDR + 4 + i * 8, current[i].mod);
    EEPROM.update(COMBO_EEPROM_ADDR + 4 + i * 8 + 1, current[i].key);
    for (int out = 0; out < COMBO_OUTPUTS; out++) {
      EEPROM.update(COMBO_EEPROM_ADDR + 4 + i * 8 + 2 + out * 2, current[i].outputs[out].value);
      EEPROM.update(COMBO_EEPROM_ADDR + 4 + i * 8 + 3 + out * 2, current[i].outputs[out].mods);
    }
  }

  uint16_t crc = comboStorageCrc(COMBO_STORAGE_VERSION, current);
  EEPROM.update(COMBO_EEPROM_ADDR + COMBO_EEPROM_SIZE - 2, crc >> 8);
  EEPROM.update(COMBO_EEPROM_ADDR + COMBO_EEPROM_SIZE - 1, crc & 0xFF);
}

#else
bool loadCombosFromEEPROM() {
  return false;
}

void writeCombosToEEPROM() {
}
#endif

#endif
