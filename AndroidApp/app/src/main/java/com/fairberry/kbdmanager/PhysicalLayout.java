package com.fairberry.kbdmanager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Physical key positions of the BlackBerry Q10 keyboard, mapped to the firmware keymap
 * matrix indices. The firmware storage order is the electrical scan order, which does not
 * match the visual keycap rows.
 */
public final class PhysicalLayout {

    public static class Key {
        public final int col;
        public final int row;
        public final String name;
        public final int weight;  // share of the row when rendering

        Key(int col, int row, String name, int weight) {
            this.col = col;
            this.row = row;
            this.name = name;
            this.weight = weight;
        }
    }

    // Row 1: Q W E R T Y U I O P
    // Row 2: A S D F G H J K L Backspace
    // Row 3: Alt Z X C V B N M $ Enter
    // Row 4: LeftShift Ctrl Space Sym RightShift
    public static final List<List<Key>> ROWS = Arrays.asList(
            Arrays.asList(
                    key(0, 0, "Q", 1), key(0, 1, "W", 1), key(1, 0, "E", 1), key(2, 0, "R", 1),
                    key(2, 2, "T", 1), key(3, 2, "Y", 1), key(3, 0, "U", 1), key(4, 2, "I", 1),
                    key(4, 0, "O", 1), key(1, 3, "P", 1)),
            Arrays.asList(
                    key(0, 3, "A", 1), key(1, 1, "S", 1), key(1, 2, "D", 1), key(2, 6, "F", 1),
                    key(2, 1, "G", 1), key(3, 1, "H", 1), key(3, 6, "J", 1), key(4, 6, "K", 1),
                    key(4, 1, "L", 1), key(4, 3, "Backspace", 1)),
            Arrays.asList(
                    key(0, 4, "Alt", 1), key(1, 5, "Z", 1), key(1, 4, "X", 1), key(2, 5, "C", 1),
                    key(2, 4, "V", 1), key(3, 4, "B", 1), key(3, 5, "N", 1), key(4, 5, "M", 1),
                    key(4, 4, "$", 1), key(3, 3, "Enter", 1)),
            Arrays.asList(
                    key(1, 6, "LeftShift", 2), key(0, 6, "Ctrl", 1), key(0, 5, "Space", 4),
                    key(0, 2, "Sym", 1), key(2, 3, "RightShift", 2)));

    public static List<Key> allKeys() {
        List<Key> keys = new ArrayList<>();
        for (List<Key> row : ROWS) {
            keys.addAll(row);
        }
        return keys;
    }

    public static String nameFor(int col, int row) {
        for (Key key : allKeys()) {
            if (key.col == col && key.row == row) {
                return key.name;
            }
        }
        return "col " + col + " row " + row;
    }

    private static Key key(int col, int row, String name, int weight) {
        return new Key(col, row, name, weight);
    }

    private PhysicalLayout() {
    }
}
