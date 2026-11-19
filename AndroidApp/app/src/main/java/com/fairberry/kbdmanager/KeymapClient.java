package com.fairberry.kbdmanager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Client for the firmware keymap text protocol (see BBQ10/protocol.h).
 * All methods must be called from a single background thread.
 */
public class KeymapClient {

    public interface ProtocolLog {
        void log(String line);
    }

    public static class Info {
        public String firmware;
        public int protocol;
        public String storage;
    }

    private final UsbCdc cdc;
    private final ProtocolLog protocolLog;

    public KeymapClient(UsbCdc cdc) {
        this(cdc, null);
    }

    public KeymapClient(UsbCdc cdc, ProtocolLog protocolLog) {
        this.cdc = cdc;
        this.protocolLog = protocolLog;
    }

    private void log(String line) {
        if (protocolLog != null) {
            protocolLog.log(line);
        }
    }

    private String readLine(int timeoutMs) throws IOException {
        long deadline = System.currentTimeMillis() + timeoutMs;
        while (true) {
            String line = readRawLine(deadline);
            if (line == null) {
                throw new IOException("keyboard did not respond (timeout)");
            }
            if (isDebugLine(line)) {
                log(line);
                continue;
            }
            return line;
        }
    }

    public void drainPending(int timeoutMs) throws IOException {
        String line = readRawLine(System.currentTimeMillis() + timeoutMs);
        while (line != null) {
            log("raw: " + line);
            line = readRawLine(System.currentTimeMillis() + timeoutMs);
        }
    }

    private static boolean isDebugLine(String line) {
        return line.startsWith("D ") || line.startsWith("E ");
    }

    private String readRawLine(long deadline) throws IOException {
        StringBuilder sb = new StringBuilder();
        while (true) {
            int remaining = (int) Math.max(0, deadline - System.currentTimeMillis());
            int b = cdc.readByte(remaining);
            if (b < 0) {
                return sb.length() == 0 ? null : sb.toString();
            }
            char c = (char) b;
            if (c == '\n') {
                return sb.toString();
            }
            if (c == '\r') {
                continue;
            }
            sb.append(c);
        }
    }

    private void send(String command) throws IOException {
        if (!cdc.write(command + "\n")) {
            throw new IOException("USB write failed");
        }
    }

    private void expectOk(String command) throws IOException {
        send(command);
        String response = readLine(2000);
        if (!response.equals("OK")) {
            throw new IOException("command '" + command + "' failed: " + response);
        }
    }

    public void sync() throws IOException {
        cdc.dropPendingInput();
        // the first INFO read doubles as a keep-alive and a protocol probe
        readInfo();
    }

    public Info readInfo() throws IOException {
        send("I");
        String line = readLine(2000);
        if (!line.startsWith("INFO")) {
            throw new IOException("unexpected INFO response: " + line);
        }
        Info info = new Info();
        String[] parts = line.split(" ");
        if (parts.length >= 4) {
            info.firmware = parts[1];
            info.protocol = Integer.parseInt(parts[2]);
            info.storage = parts[3];
        }
        return info;
    }

    public KeymapData readKeymap() throws IOException {
        KeymapData data = new KeymapData();
        send("L");
        int expected = KeymapData.MODES * KeymapData.COLS * KeymapData.ROWS;
        for (int i = 0; i < expected; i++) {
            String line = readLine(2000);
            log("raw K: " + line);
            if (!line.startsWith("K ")) {
                throw new IOException("unexpected keymap line: " + line);
            }
            String[] parts = line.split(" ");
            int mode = Integer.parseInt(parts[1]);
            int col = Integer.parseInt(parts[2]);
            int row = Integer.parseInt(parts[3]);
            int value = Integer.parseInt(parts[4]);
            int mods = Integer.parseInt(parts[5]);
            if (mode >= 0 && mode < KeymapData.MODES && col >= 0 && col < KeymapData.COLS
                    && row >= 0 && row < KeymapData.ROWS) {
                data.set(mode, col, row, new KeymapData.Entry(value, mods));
            }
        }
        return data;
    }

    public void applyChanges(List<KeymapData.Change> changes) throws IOException {
        StringBuilder batch = new StringBuilder();
        for (KeymapData.Change change : changes) {
            batch.append("M ")
                    .append(change.mode).append(' ')
                    .append(change.col).append(' ')
                    .append(change.row).append(' ')
                    .append(change.entry.value).append(' ')
                    .append(change.entry.mods).append('\n');
        }
        if (batch.length() == 0) {
            return;
        }
        log("raw M batch: " + batch.toString().trim().replace('\n', ';'));
        if (!cdc.write(batch.toString())) {
            throw new IOException("USB write failed");
        }
        for (int i = 0; i < changes.size(); i++) {
            String response = readLine(2000);
            log("raw response: " + response);
            if (!response.equals("OK")) {
                throw new IOException("keymap update rejected: " + response);
            }
        }
    }

    public void writeEeprom() throws IOException {
        expectOk("W");
    }

    public void reloadFromEeprom() throws IOException {
        expectOk("D");
    }

    public void loadFactoryDefaults() throws IOException {
        expectOk("F");
    }

    // ---------- combinations ----------

    public ComboData readCombos() throws IOException {
        ComboData data = new ComboData();
        send("C");
        while (true) {
            String line = readLine(2000);
            log("raw G: " + line);
            if (line.equals("OK")) {
                return data;
            }
            if (!line.startsWith("G ")) {
                throw new IOException("unexpected combo line: " + line);
            }
            String[] parts = line.split(" ");
            if (parts.length < 4 + ComboData.OUTPUTS * 2) {
                throw new IOException("unexpected combo line: " + line);
            }
            int mod = Integer.parseInt(parts[1]);
            int col = Integer.parseInt(parts[2]);
            int row = Integer.parseInt(parts[3]);
            List<KeymapData.Entry> outputs = new ArrayList<>();
            for (int out = 0; out < ComboData.OUTPUTS; out++) {
                int value = Integer.parseInt(parts[4 + out * 2]);
                int mods = Integer.parseInt(parts[5 + out * 2]);
                outputs.add(new KeymapData.Entry(value, mods));
            }
            data.add(new ComboData.Combo(mod, col, row, outputs));
        }
    }

    public void addCombo(ComboData.Combo combo) throws IOException {
        StringBuilder command = new StringBuilder("N ")
                .append(combo.mod).append(' ')
                .append(combo.col).append(' ')
                .append(combo.row);
        for (KeymapData.Entry output : combo.outputs) {
            command.append(' ').append(output.value).append(' ').append(output.mods);
        }
        expectOk(command.toString());
    }

    public void removeCombo(ComboData.Combo combo) throws IOException {
        expectOk("X " + combo.mod + " " + combo.col + " " + combo.row);
    }

    public void writeCombosToEEPROM() throws IOException {
        expectOk("T");
    }

    public void loadComboDefaults() throws IOException {
        expectOk("V");
    }

    public void reloadCombosFromEeprom() throws IOException {
        expectOk("Y");
    }

    /**
     * Refresh the keyboard's idle timeout. Fire and forget: the firmware sends no
     * response for 'P', so waiting for one would stall the worker and fill the
     * buffer with stale answers.
     */
    public void keepAlive() throws IOException {
        send("P");
    }

    public void softReset() throws IOException {
        expectOk("Z");
    }

    public void enterBootloader() {
        try {
            send("B");
        } catch (IOException ignored) {
        }
    }
}
