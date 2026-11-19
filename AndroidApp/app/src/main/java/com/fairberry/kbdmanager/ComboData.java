package com.fairberry.kbdmanager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

/**
 * Key combinations, mirroring the firmware table in BBQ10/combos.h: modifier bit plus
 * physical keycap, up to three output keys fired while the modifier is already held.
 */
public class ComboData {

    public static final int MAX = 40;
    public static final int OUTPUTS = 3;

    public static class Combo {
        public final int mod; // one modifier bitmask bit, 0 == empty row
        public final int col;
        public final int row;
        public final List<KeymapData.Entry> outputs;

        public Combo(int mod, int col, int row, List<KeymapData.Entry> outputs) {
            this.mod = mod;
            this.col = col;
            this.row = row;
            this.outputs = new ArrayList<>(outputs);
        }

        public boolean isEmpty() {
            return mod == 0;
        }

        public int modBit() {
            return KeyLabels.modifierBit(mod);
        }

        public KeymapData.Entry output(int index) {
            return outputs.get(index);
        }

        @Override
        public boolean equals(Object o) {
            if (!(o instanceof Combo)) {
                return false;
            }
            Combo other = (Combo) o;
            return mod == other.mod && col == other.col && row == other.row
                    && outputs.equals(other.outputs);
        }

        @Override
        public int hashCode() {
            return Objects.hash(mod, col, row, outputs);
        }
    }

    public static final Combo EMPTY = new Combo(0, 0, 0,
            Arrays.asList(KeymapData.EMPTY, KeymapData.EMPTY, KeymapData.EMPTY));

    private final Combo[] slots = new Combo[MAX];

    public ComboData() {
        for (int i = 0; i < MAX; i++) {
            slots[i] = EMPTY;
        }
    }

    public Combo get(int index) {
        return slots[index];
    }

    public void set(int index, Combo combo) {
        slots[index] = combo;
    }

    public List<Combo> rows() {
        List<Combo> rows = new ArrayList<>();
        for (Combo combo : slots) {
            if (!combo.isEmpty()) {
                rows.add(combo);
            }
        }
        return rows;
    }

    /**
     * The modifier + keycap pair is unique, so a row is identified by that pair
     * instead of by its slot: a row keeps its identity while other rows change.
     */
    public int indexOf(int mod, int col, int row) {
        for (int i = 0; i < MAX; i++) {
            Combo combo = slots[i];
            if (!combo.isEmpty() && combo.mod == mod && combo.col == col && combo.row == row) {
                return i;
            }
        }
        return -1;
    }

    public boolean add(Combo combo) {
        if (combo.isEmpty() || !comboHasOutput(combo)) {
            return false; // a combination needs at least one output key
        }
        int index = indexOf(combo.mod, combo.col, combo.row);
        if (index < 0) {
            for (int i = 0; i < MAX; i++) {
                if (slots[i].isEmpty()) {
                    index = i;
                    break;
                }
            }
            if (index < 0) {
                return false;
            }
        }
        slots[index] = combo;
        return true;
    }

    public boolean remove(int mod, int col, int row) {
        int index = indexOf(mod, col, row);
        if (index < 0) {
            return false;
        }
        slots[index] = EMPTY;
        return true;
    }

    public ComboData copy() {
        ComboData copy = new ComboData();
        for (int i = 0; i < MAX; i++) {
            copy.slots[i] = slots[i];
        }
        return copy;
    }

    public List<Combo> additionsAgainst(ComboData baseline) {
        List<Combo> additions = new ArrayList<>();
        for (Combo combo : rows()) {
            int index = baseline.indexOf(combo.mod, combo.col, combo.row);
            if (index < 0 || !combo.equals(baseline.get(index))) {
                additions.add(combo);
            }
        }
        return additions;
    }

    public List<Combo> removalsAgainst(ComboData baseline) {
        List<Combo> removals = new ArrayList<>();
        for (Combo combo : baseline.rows()) {
            if (indexOf(combo.mod, combo.col, combo.row) < 0) {
                removals.add(combo);
            }
        }
        return removals;
    }

    private static boolean comboHasOutput(Combo combo) {
        for (KeymapData.Entry output : combo.outputs) {
            if (!output.isEmpty()) {
                return true;
            }
        }
        return false;
    }
}
