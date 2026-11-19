package com.fairberry.kbdmanager;

import java.io.IOException;
import java.util.Arrays;
import java.util.Map;
import java.util.TreeMap;

/**
 * Parser for Intel HEX files. Records at or above 0x7000 are skipped: the Caterina bootloader
 * owns flash from there up.
 */
public class IntelHex {

    public static final int APP_REGION_END = 0x7000;

    private static final int REC_DATA = 0x00;
    private static final int REC_EOF = 0x01;
    private static final int REC_EXTENDED_SEGMENT = 0x02;
    private static final int REC_EXTENDED_LINEAR = 0x04;

    private final TreeMap<Integer, Integer> data = new TreeMap<>();

    public static IntelHex parse(String text) throws IOException {
        IntelHex hex = new IntelHex();
        int extendedAddress = 0;
        int lineNo = 0;
        for (String rawLine : text.split("\\R")) {
            lineNo++;
            String line = rawLine.trim();
            if (line.isEmpty()) {
                continue;
            }
            if (line.charAt(0) != ':') {
                throw new IOException("line " + lineNo + ": not an Intel HEX record");
            }
            if ((line.length() - 1) % 2 != 0) {
                throw new IOException("line " + lineNo + ": odd hex length");
            }
            int len = (line.length() - 1) / 2;
            byte[] bytes = new byte[len];
            for (int i = 0; i < len; i++) {
                bytes[i] = (byte) Integer.parseInt(line.substring(1 + i * 2, 3 + i * 2), 16);
            }
            byte sum = 0;
            for (byte b : bytes) {
                sum += b;
            }
            if (sum != 0) {
                throw new IOException("line " + lineNo + ": checksum mismatch");
            }

            int count = bytes[0] & 0xFF;
            int offset = ((bytes[1] & 0xFF) << 8) | (bytes[2] & 0xFF);
            int type = bytes[3] & 0xFF;
            if (count + 5 != len) {
                throw new IOException("line " + lineNo + ": record length mismatch");
            }

            switch (type) {
                case REC_DATA: {
                    int base = extendedAddress;
                    for (int i = 0; i < count; i++) {
                        int address = base + offset + i;
                        if (address >= APP_REGION_END) {
                            continue; // bootloader area, Caterina cannot flash it
                        }
                        hex.data.put(address, bytes[4 + i] & 0xFF);
                    }
                    break;
                }
                case REC_EOF:
                    return hex;
                case REC_EXTENDED_SEGMENT:
                    extendedAddress = (((bytes[4] & 0xFF) << 8 | (bytes[5] & 0xFF)) << 4);
                    break;
                case REC_EXTENDED_LINEAR:
                    extendedAddress = (((bytes[4] & 0xFF) << 8 | (bytes[5] & 0xFF)) << 16);
                    break;
                default:
                    // start/linear address records are not used by Arduino builds
                    break;
            }
        }
        throw new IOException("no EOF record in hex file");
    }

    /**
     * Blocks must be exactly one physical SPM page: Caterina commits the whole hardware page
     * buffer with a single boot_page_write per block, so a short or oversized block would commit
     * buffer contents that do not match the page. Blank pages are absent because 'e' has already
     * erased them.
     */
    public Map<Integer, byte[]> blankPagesFilled(int pageSize) {
        TreeMap<Integer, byte[]> blocks = new TreeMap<>();
        for (Map.Entry<Integer, Integer> e : data.entrySet()) {
            int blockStart = (e.getKey() / pageSize) * pageSize;
            byte[] block = blocks.get(blockStart);
            if (block == null) {
                block = new byte[pageSize];
                Arrays.fill(block, (byte) 0xFF);
                blocks.put(blockStart, block);
            }
            block[e.getKey() - blockStart] = (byte) (int) e.getValue();
        }

        TreeMap<Integer, byte[]> filled = new TreeMap<>();
        for (Map.Entry<Integer, byte[]> e : blocks.entrySet()) {
            boolean allBlank = true;
            for (byte b : e.getValue()) {
                if (b != (byte) 0xFF) {
                    allBlank = false;
                    break;
                }
            }
            if (!allBlank) {
                filled.put(e.getKey(), e.getValue());
            }
        }
        return filled;
    }

    public int highestAddress() {
        return data.isEmpty() ? -1 : data.lastKey();
    }

    public boolean isEmpty() {
        return data.isEmpty();
    }
}
