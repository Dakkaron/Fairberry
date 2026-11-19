package com.fairberry.kbdmanager;

import java.io.IOException;

/**
 * Flashes an application image through the Caterina USB bootloader using the
 * AVR910-over-CDC protocol (see caterina-bootloader Caterina.c).
 *
 * The protocol is strictly sequential: every command is followed by its
 * response bytes before the next command may be sent.
 */
public class CaterinaFlasher {

    public interface ProgressListener {
        void onProgress(int percent, String message);
    }

    private final UsbCdc cdc;

    public CaterinaFlasher(UsbCdc cdc) {
        this.cdc = cdc;
    }

    private int readByte(int timeoutMs) throws IOException {
        int b = cdc.readByte(timeoutMs);
        if (b < 0) {
            throw new IOException("no response from bootloader (is the device in bootloader mode?)");
        }
        return b;
    }

    private void expectCarriage(String command) throws IOException {
        int b = readByte(6000);
        if (b != '\r') {
            throw new IOException("command '" + command + "' returned 0x"
                    + Integer.toHexString(b) + " instead of acknowledgement");
        }
    }

    private void send(String command) throws IOException {
        if (!cdc.write(command)) {
            throw new IOException("USB write failed");
        }
    }

    public String handshake() throws IOException {
        cdc.dropPendingInput();
        send("V");
        int major = readByte(2000);
        int minor = readByte(2000);
        if (major < '0' || major > '9' || minor < '0' || minor > '9') {
            throw new IOException("unexpected bootloader version bytes: " + major + " " + minor);
        }
        return (char) major + "." + (char) minor;
    }

    public int queryPageSize() throws IOException {
        send("b");
        int ack = readByte(2000);
        if (ack != 'Y') {
            throw new IOException("block mode not supported (got 0x" + Integer.toHexString(ack) + ")");
        }
        int hi = readByte(2000);
        int lo = readByte(2000);
        int pageSize = (hi << 8) | lo;
        if (pageSize <= 0 || pageSize > 1024) {
            throw new IOException("nonsense page size " + pageSize);
        }
        return pageSize;
    }

    public void flash(IntelHex hex, ProgressListener listener) throws IOException {
        String version = handshake();
        listener.onProgress(0, "Caterina bootloader " + version);

        // Blocks must be exactly SPM_PAGESIZE as reported by 'b' (128 bytes on
        // the ATmega32U4): Caterina commits one hardware page buffer per 'B'
        // block, so a 256-byte block wraps the buffer and commits the wrong half.
        int pageSize = queryPageSize();

        java.util.Map<Integer, byte[]> pages = hex.blankPagesFilled(pageSize);
        if (pages.isEmpty()) {
            throw new IOException("hex file has no data in the application region");
        }

        listener.onProgress(2, "Erasing flash...");
        send("e");
        expectCarriage("e");

        int totalPages = pages.size();
        int done = 0;
        for (java.util.Map.Entry<Integer, byte[]> page : pages.entrySet()) {
            int byteAddress = page.getKey();
            byte[] data = page.getValue();
            int wordAddress = byteAddress >> 1;

            // 'A': set word address (hi, lo); byte address = word << 1
            send("A" + (char) ((wordAddress >> 8) & 0xFF) + (char) (wordAddress & 0xFF));
            expectCarriage("A");

            // 'B': write a memory block: size hi, size lo, type 'F', then data.
            // Data pairs arrive low-byte-first, as stored in raw IntelHex records.
            // Caterina erases the page at the current address before filling it.
            int size = data.length;
            cdc.write("B" + (char) ((size >> 8) & 0xFF) + (char) (size & 0xFF) + "F");
            if (!cdc.writeRaw(data)) {
                throw new IOException("USB data write failed");
            }
            expectCarriage("B");

            done++;
            int percent = 5 + (done * 93 / totalPages);
            listener.onProgress(percent, String.format("Wrote page 0x%04X (%d/%d)", byteAddress, done, totalPages));
        }

        // 'E': done, re-enable read-while-write and let the bootloader exit.
        send("E");
        expectCarriage("E");
        listener.onProgress(100, "Flash complete");
    }
}
