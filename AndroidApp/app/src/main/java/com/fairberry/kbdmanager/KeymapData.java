package com.fairberry.kbdmanager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/** Keymap model mirroring the firmware layout: 3 modes x 5 columns x 7 rows. */
public class KeymapData {
    public static final int MODES = 3;
    public static final int COLS = 5;
    public static final int ROWS = 7;
    public static final int[] MODE_IDS = {0, 1, 2};
    public static final String[] MODE_NAMES = {"Normal", "Symbol", "Cursor"};

    public static class Entry {
        public final int value; // Keyboard.h value domain (see KeyLabels), 0 == none
        public final int mods;  // bitmask bit0=LCtrl .. bit7=RGui

        public Entry(int value, int mods) {
            this.value = value;
            this.mods = mods;
        }

        public boolean isEmpty() {
            return value == 0 && mods == 0;
        }

        @Override
        public boolean equals(Object o) {
            if (!(o instanceof Entry)) {
                return false;
            }
            Entry other = (Entry) o;
            return value == other.value && mods == other.mods;
        }

        @Override
        public int hashCode() {
            return Objects.hash(value, mods);
        }
    }

    public static final Entry EMPTY = new Entry(0, 0);

    private final Entry[][][] entries = new Entry[MODES][COLS][ROWS];

    public KeymapData() {
        for (int m = 0; m < MODES; m++) {
            for (int c = 0; c < COLS; c++) {
                for (int r = 0; r < ROWS; r++) {
                    entries[m][c][r] = EMPTY;
                }
            }
        }
    }

    public Entry get(int mode, int col, int row) {
        return entries[mode][col][row];
    }

    public void set(int mode, int col, int row, Entry entry) {
        entries[mode][col][row] = entry;
    }

    public KeymapData copy() {
        KeymapData copy = new KeymapData();
        for (int m = 0; m < MODES; m++) {
            for (int c = 0; c < COLS; c++) {
                for (int r = 0; r < ROWS; r++) {
                    copy.entries[m][c][r] = entries[m][c][r];
                }
            }
        }
        return copy;
    }

    public static class Change {
        public final int mode;
        public final int col;
        public final int row;
        public final Entry entry;

        Change(int mode, int col, int row, Entry entry) {
            this.mode = mode;
            this.col = col;
            this.row = row;
            this.entry = entry;
        }
    }

    public List<Change> changesAgainst(KeymapData baseline) {
        List<Change> changes = new ArrayList<>();
        for (int m = 0; m < MODES; m++) {
            for (int c = 0; c < COLS; c++) {
                for (int r = 0; r < ROWS; r++) {
                    if (!entries[m][c][r].equals(baseline.get(m, c, r))) {
                        changes.add(new Change(m, c, r, entries[m][c][r]));
                    }
                }
            }
        }
        return changes;
    }
}
