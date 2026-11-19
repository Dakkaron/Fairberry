package com.fairberry.kbdmanager;

import java.util.ArrayList;
import java.util.List;

/**
 * Keymap value labels and presets. Values follow the Arduino Keyboard library domain
 * (Keyboard.h): 0 = nothing, 1..127 = the ASCII code of a printable character, 128..135 =
 * the modifier constants, 136..255 = Keyboard.h constants. Plain text keys store their ASCII
 * character, and modifier constants are normalized into the modifier bitmask, so a stored
 * entry is either a value or modifiers.
 */
public final class KeyLabels {

    // Modifier bitmask bits, matching the Keyboard.h modifier value order
    static final int MOD_LCTRL = 0x01;
    static final int MOD_LSHIFT = 0x02;
    static final int MOD_LALT = 0x04;
    static final int MOD_LGUI = 0x08;
    static final int MOD_RCTRL = 0x10;
    static final int MOD_RSHIFT = 0x20;
    static final int MOD_RALT = 0x40;
    static final int MOD_RGUI = 0x80;

    static final int KEY_LEFT_CTRL = 0x80;
    static final int KEY_LEFT_SHIFT = 0x81;
    static final int KEY_LEFT_ALT = 0x82;
    static final int KEY_LEFT_GUI = 0x83;
    static final int KEY_RIGHT_CTRL = 0x84;
    static final int KEY_RIGHT_SHIFT = 0x85;
    static final int KEY_RIGHT_ALT = 0x86;
    static final int KEY_RIGHT_GUI = 0x87;
    static final int KEY_SPACE = 32;
    static final int KEY_RETURN = 0xB0;
    static final int KEY_ESC = 0xB1;
    static final int KEY_BACKSPACE = 0xB2;
    static final int KEY_TAB = 0xB3;
    static final int KEY_PAUSE = 0xD0;
    static final int KEY_INSERT = 0xD1;
    static final int KEY_HOME = 0xD2;
    static final int KEY_PAGE_UP = 0xD3;
    static final int KEY_DELETE = 0xD4;
    static final int KEY_END = 0xD5;
    static final int KEY_PAGE_DOWN = 0xD6;
    static final int KEY_RIGHT_ARROW = 0xD7;
    static final int KEY_LEFT_ARROW = 0xD8;
    static final int KEY_DOWN_ARROW = 0xD9;
    static final int KEY_UP_ARROW = 0xDA;
    static final int KEY_NUM_LOCK = 0xDB;
    static final int KEY_KP_SLASH = 0xDC;
    static final int KEY_KP_ASTERISK = 0xDD;
    static final int KEY_KP_MINUS = 0xDE;
    static final int KEY_KP_PLUS = 0xDF;
    static final int KEY_KP_ENTER = 0xE0;
    static final int KEY_MENU = 0xED;
    static final int KEY_CAPS_LOCK = 0xC1;
    static final int KEY_PRINT_SCREEN = 0xCE;
    static final int KEY_SCROLL_LOCK = 0xCF;
    static final int KEY_F1 = 0xC2;
    static final int KEY_F13 = 0xF0;

    // Values 136..175 are the block the firmware reserves for HID consumer page
    // (0x0C) keys, which Keyboard.h has no constants for. Android maps these
    // usages to the home screen, back and the volume controls.
    static final int ANDROID_HOME = 136;  // usage 0x0223
    static final int ANDROID_BACK = 137;  // usage 0x0224
    static final int VOLUME_MUTE = 138;   // usage 0x00E2
    static final int VOLUME_UP = 139;     // usage 0x00E9
    static final int VOLUME_DOWN = 140;   // usage 0x00EA
    static final int APP_SWITCH = 141;    // usage 0x01A2

    // Values 252..255 are the device actions the firmware performs itself instead
    // of sending a usage to the host.
    static final int ACTION_BACKLIGHT_DOWN = 252;
    static final int ACTION_BACKLIGHT_UP = 253;

    private static final String[] CONSTANTS = new String[0x100];
    // Key-grid cells are one row tall and often narrower than the name, so the same value
    // carries a short spelling for the grid; anything without a short form falls back to
    // its full label. Pickers and the combination table always use the full label.
    private static final String[] GRID_NAMES = new String[0x100];
    static final String[] MODIFIER_NAMES = {
            "LCtrl", "LShift", "LAlt", "LGui", "RCtrl", "RShift", "RAlt", "RGui"
    };

    static {
        for (int value = 0; value < 8; value++) {
            CONSTANTS[KEY_LEFT_CTRL + value] = MODIFIER_NAMES[value];
        }
        for (int f = 1; f <= 12; f++) {
            CONSTANTS[KEY_F1 + f - 1] = "F" + f;
        }
        for (int f = 13; f <= 24; f++) {
            CONSTANTS[KEY_F13 + f - 13] = "F" + f;
        }
        for (int d = 0; d <= 9; d++) {
            // Keyboard.h maps the numpad digits to 0xE1..0xEA (0 = 0xEA).
            CONSTANTS[KEY_KP_ENTER + 1 + (d == 0 ? 9 : d - 1)] = "Kp" + d;
        }
        CONSTANTS[KEY_SPACE] = "Space";
        CONSTANTS[KEY_RETURN] = "Enter";
        CONSTANTS[KEY_ESC] = "Esc";
        CONSTANTS[KEY_BACKSPACE] = "Bksp";
        CONSTANTS[KEY_TAB] = "Tab";
        CONSTANTS[KEY_PAUSE] = "Pause";
        CONSTANTS[KEY_INSERT] = "Insert";
        CONSTANTS[KEY_HOME] = "Home";
        CONSTANTS[KEY_PAGE_UP] = "PgUp";
        CONSTANTS[KEY_DELETE] = "Delete";
        CONSTANTS[KEY_END] = "End";
        CONSTANTS[KEY_PAGE_DOWN] = "PgDn";
        CONSTANTS[KEY_RIGHT_ARROW] = "Right";
        CONSTANTS[KEY_LEFT_ARROW] = "Left";
        CONSTANTS[KEY_DOWN_ARROW] = "Down";
        CONSTANTS[KEY_UP_ARROW] = "Up";
        CONSTANTS[KEY_NUM_LOCK] = "NumLock";
        CONSTANTS[KEY_KP_SLASH] = "Kp/";
        CONSTANTS[KEY_KP_ASTERISK] = "Kp*";
        CONSTANTS[KEY_KP_MINUS] = "Kp-";
        CONSTANTS[KEY_KP_PLUS] = "Kp+";
        CONSTANTS[KEY_KP_ENTER] = "KpEnter";
        CONSTANTS[0xEB] = "Kp.";
        CONSTANTS[KEY_CAPS_LOCK] = "CapsLock";
        CONSTANTS[KEY_PRINT_SCREEN] = "PrntScr";
        CONSTANTS[KEY_SCROLL_LOCK] = "ScrLock";
        CONSTANTS[KEY_MENU] = "Menu";
        CONSTANTS[ANDROID_HOME] = "Navigation Home";
        CONSTANTS[ANDROID_BACK] = "Navigation Back";
        CONSTANTS[VOLUME_MUTE] = "Volume Mute";
        CONSTANTS[VOLUME_UP] = "Volume Up";
        CONSTANTS[VOLUME_DOWN] = "Volume Down";
        CONSTANTS[APP_SWITCH] = "Navigation App Switch";
        CONSTANTS[ACTION_BACKLIGHT_DOWN] = "Backlight Down";
        CONSTANTS[ACTION_BACKLIGHT_UP] = "Backlight Up";
        gridName(ANDROID_HOME, "Nav Home");
        gridName(ANDROID_BACK, "Nav Back");
        gridName(VOLUME_MUTE, "\uD83D\uDD68");
        gridName(VOLUME_UP, "\uD83D\uDD6A\u2191");
        gridName(VOLUME_DOWN, "\uD83D\uDD69\u2193");
        gridName(APP_SWITCH, "Nav Switch");
        gridName(ACTION_BACKLIGHT_DOWN, "\uD83D\uDD05\u2193");
        gridName(ACTION_BACKLIGHT_UP, "\uD83D\uDD06\u2191");
    }

    private static void gridName(int value, String name) {
        GRID_NAMES[value] = name;
    }

    public static String entryLabel(int value, int mods) {
        return entryLabel(value, mods, false);
    }

    public static String gridLabel(int value, int mods) {
        return entryLabel(value, mods, true);
    }

    private static String entryLabel(int value, int mods, boolean grid) {
        if (value == 0 && mods == 0) {
            return "·";
        }
        if (value == 0) {
            return modifierOnlyLabel(mods);
        }
        StringBuilder prefix = new StringBuilder();
        for (int bit = 0; bit < 8; bit++) {
            if ((mods & (1 << bit)) != 0) {
                prefix.append(MODIFIER_NAMES[bit]).append('+');
            }
        }
        return prefix + valueLabel(value, grid);
    }

    private static String valueLabel(int value, boolean grid) {
        // The space key is stored as ASCII 32, but a bare space would render
        // as an empty cell like a "no action" entry, so show it as "Space".
        if (value == KEY_SPACE) {
            return "Space";
        }
        if (value < 128) {
            return String.valueOf((char) value);
        }
        String name = grid && GRID_NAMES[value] != null ? GRID_NAMES[value] : CONSTANTS[value];
        return name != null ? name : String.format("0x%02X", value);
    }

    private static String modifierOnlyLabel(int mods) {
        List<String> names = new ArrayList<>();
        for (int bit = 0; bit < 8; bit++) {
            if ((mods & (1 << bit)) != 0) {
                names.add(MODIFIER_NAMES[bit]);
            }
        }
        return String.join(" ", names);
    }

    static String modifierName(int bit) {
        return MODIFIER_NAMES[bit];
    }

    public static String comboLabel(ComboData.Combo combo) {
        List<String> outputs = new ArrayList<>();
        for (KeymapData.Entry output : combo.outputs) {
            if (!output.isEmpty()) {
                outputs.add(entryLabel(output.value, output.mods));
            }
        }
        return modifierName(combo.modBit()) + " + " + PhysicalLayout.nameFor(combo.col, combo.row)
                + " -> " + String.join(" + ", outputs);
    }

    /** Combos fire in every mode, so their outputs use one general choice list. */
    public static List<KeymapData.Entry> outputPresets() {
        List<KeymapData.Entry> presets = new ArrayList<>();
        presets.add(KeymapData.EMPTY);
        for (char c = 'a'; c <= 'z'; c++) {
            presets.add(entry(c, 0));
        }
        for (int d = 0; d <= 9; d++) {
            presets.add(entry('0' + d, 0));
        }
        for (char c : new char[]{'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_',
                '+', '[', ']', '{', '}', ';', ':', '\'', '"', ',', '.', '/', '\\', '`', '<', '>',
                '?', '|'}) {
            presets.add(entry(c, 0));
        }
        presets.add(entry(KEY_RETURN, 0));
        presets.add(entry(KEY_ESC, 0));
        presets.add(entry(KEY_BACKSPACE, 0));
        presets.add(entry(KEY_TAB, 0));
        presets.add(entry(KEY_SPACE, 0));
        presets.add(entry(KEY_CAPS_LOCK, 0));
        presets.add(entry(KEY_PRINT_SCREEN, 0));
        presets.add(entry(KEY_MENU, 0));
        for (int f = 1; f <= 15; f++) {
            presets.add(entry(KEY_F1 + f - 1, 0));
        }
        for (int value : new int[]{KEY_HOME, KEY_END, KEY_PAGE_UP, KEY_PAGE_DOWN, KEY_INSERT,
                KEY_DELETE, KEY_UP_ARROW, KEY_DOWN_ARROW, KEY_LEFT_ARROW, KEY_RIGHT_ARROW}) {
            presets.add(entry(value, 0));
        }
        for (int bit = 0; bit < 8; bit++) {
            presets.add(entry(0, 1 << bit));
        }
        for (KeymapData.Entry combo : new KeymapData.Entry[]{
                entry('a', MOD_LCTRL), entry('c', MOD_LCTRL), entry('v', MOD_LCTRL),
                entry('x', MOD_LCTRL), entry('z', MOD_LCTRL), entry(KEY_TAB, MOD_LALT),
                entry(KEY_ESC, MOD_LALT), entry(KEY_RETURN, MOD_LALT), entry(KEY_BACKSPACE, MOD_LALT)}) {
            presets.add(combo);
        }
        for (int value : new int[]{ANDROID_HOME, ANDROID_BACK, VOLUME_MUTE, VOLUME_UP, VOLUME_DOWN,
                APP_SWITCH, ACTION_BACKLIGHT_DOWN, ACTION_BACKLIGHT_UP}) {
            presets.add(entry(value, 0));
        }
        return presets;
    }

    /** Combos carry exactly one modifier bit. */
    public static int modifierBit(int mod) {
        int bit = 0;
        while (bit < 8 && ((mod >> bit) & 1) == 0) {
            bit++;
        }
        return bit;
    }

    public static List<KeymapData.Entry> presetsForMode(int mode) {
        List<KeymapData.Entry> presets = new ArrayList<>();
        presets.add(KeymapData.EMPTY);
        switch (mode) {
            case 0:
                for (char c = 'a'; c <= 'z'; c++) {
                    presets.add(entry(c, 0));
                }
                for (int d = 0; d <= 9; d++) {
                    presets.add(entry('0' + d, 0));
                }
                presets.add(entry(KEY_RETURN, 0));
                presets.add(entry(KEY_ESC, 0));
                presets.add(entry(KEY_BACKSPACE, 0));
                presets.add(entry(KEY_TAB, 0));
                presets.add(entry(KEY_SPACE, 0));
                presets.add(entry('$', 0));
                presets.add(entry(0, MOD_LCTRL));
                presets.add(entry(0, MOD_LSHIFT));
                presets.add(entry(0, MOD_RSHIFT));
                presets.add(entry(0, MOD_LALT));
                presets.add(entry('a', MOD_LCTRL));
                presets.add(entry('c', MOD_LCTRL));
                presets.add(entry('v', MOD_LCTRL));
                presets.add(entry('x', MOD_LCTRL));
                presets.add(entry(KEY_TAB, MOD_LALT));
                break;
            case 1:
                for (char c : new char[]{'#', '!', '@', '$', '%', '^', '&', '*', '(', ')',
                        '_', '+', '{', '}', '|', '~', ':', ';', '"', '\'', '<', '>', '?',
                        '/', ',', '.', '-', '[', ']', '`'}) {
                    presets.add(entry(c, 0));
                }
                for (int d = 0; d <= 9; d++) {
                    presets.add(entry('0' + d, 0));
                }
                for (char c = 'a'; c <= 'z'; c++) {
                    presets.add(entry(c, 0));
                }
                presets.add(entry(KEY_RETURN, 0));
                presets.add(entry(KEY_ESC, 0));
                presets.add(entry(KEY_BACKSPACE, 0));
                presets.add(entry(KEY_TAB, 0));
                presets.add(entry(KEY_SPACE, 0));
                presets.add(entry(0, MOD_LCTRL));
                presets.add(entry(0, MOD_LSHIFT));
                presets.add(entry(0, MOD_RSHIFT));
                presets.add(entry(0, MOD_LALT));
                break;
            default:
                presets.add(entry(KEY_UP_ARROW, 0));
                presets.add(entry(KEY_DOWN_ARROW, 0));
                presets.add(entry(KEY_RIGHT_ARROW, 0));
                presets.add(entry(KEY_LEFT_ARROW, 0));
                presets.add(entry(KEY_HOME, 0));
                presets.add(entry(KEY_END, 0));
                presets.add(entry(KEY_PAGE_UP, 0));
                presets.add(entry(KEY_PAGE_DOWN, 0));
                presets.add(entry(KEY_INSERT, 0));
                presets.add(entry(KEY_DELETE, 0));
                presets.add(entry(KEY_ESC, 0));
                presets.add(entry(KEY_TAB, 0));
                presets.add(entry(KEY_SPACE, 0));
                presets.add(entry(KEY_BACKSPACE, 0));
                presets.add(entry(KEY_RETURN, 0));
                for (int f = 1; f <= 12; f++) {
                    presets.add(entry(KEY_F1 + f - 1, 0));
                }
                presets.add(entry(KEY_F13 + 2, 0));
                presets.add(entry(0, MOD_LCTRL));
                presets.add(entry(0, MOD_LSHIFT));
                presets.add(entry(0, MOD_LALT));
                for (int d = 0; d <= 9; d++) {
                    presets.add(entry('0' + d, 0));
                }
                for (char c = 'a'; c <= 'z'; c++) {
                    presets.add(entry(c, 0));
                }
                break;
        }
        for (int value : new int[]{ANDROID_HOME, ANDROID_BACK, VOLUME_MUTE, VOLUME_UP, VOLUME_DOWN,
                APP_SWITCH, ACTION_BACKLIGHT_DOWN, ACTION_BACKLIGHT_UP}) {
            presets.add(entry(value, 0));
        }
        return presets;
    }

    private static KeymapData.Entry entry(int value, int mods) {
        return new KeymapData.Entry(value, mods);
    }

    private KeyLabels() {
    }
}
