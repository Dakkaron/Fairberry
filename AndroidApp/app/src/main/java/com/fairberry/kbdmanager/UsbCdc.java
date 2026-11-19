package com.fairberry.kbdmanager;

import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

/**
 * Minimal USB CDC-ACM implementation over the Android host API.
 */
public class UsbCdc {
    public static final int USB_CLASS_CDC_DATA = 0x0A;
    public static final int USB_CLASS_COMM = 0x02;
    public static final int USB_CLASS_HID = 0x03;

    private static final int REQTYPE_HOST_TO_DEVICE = 0x21;
    private static final int SET_LINE_CODING = 0x20;
    private static final int SET_CONTROL_LINE_STATE = 0x22;

    private final UsbDevice device;
    private final UsbDeviceConnection connection;
    private final UsbInterface controlInterface;
    private final UsbInterface dataInterface;
    private final UsbEndpoint endpointIn;
    private final UsbEndpoint endpointOut;

    private final LinkedBlockingQueue<Integer> rxQueue = new LinkedBlockingQueue<>();
    private Thread readerThread;
    private volatile boolean readerRunning;

    public UsbCdc(UsbDevice device, UsbDeviceConnection connection) throws IOException {
        this.device = device;
        this.connection = connection;

        UsbInterface control = null;
        UsbInterface data = null;
        for (int i = 0; i < device.getInterfaceCount(); i++) {
            UsbInterface iface = device.getInterface(i);
            if (iface.getInterfaceClass() == USB_CLASS_COMM) {
                control = iface;
            } else if (iface.getInterfaceClass() == USB_CLASS_CDC_DATA) {
                data = iface;
            }
        }
        if (data == null) {
            throw new IOException("Device has no CDC data interface");
        }
        this.controlInterface = control;
        this.dataInterface = data;

        // Stock Android usually leaves CDC devices to the cdc-acm kernel driver;
        // claimInterface(force=true) asks the kernel to release it, which only
        // succeeds when the app (or the device, when rooted) has the rights.
        UsbInterface[] ifaces = control != null ? new UsbInterface[]{control, data} : new UsbInterface[]{data};
        for (UsbInterface iface : ifaces) {
            if (!connection.claimInterface(iface, true)) {
                throw new IOException("Could not claim USB interface " + iface.getId()
                        + " (the kernel cdc-acm driver may be holding it; needs root to unbind).");
            }
        }

        UsbEndpoint in = null;
        UsbEndpoint out = null;
        for (int i = 0; i < dataInterface.getEndpointCount(); i++) {
            UsbEndpoint ep = dataInterface.getEndpoint(i);
            if (ep.getType() != UsbConstants.USB_ENDPOINT_XFER_BULK) {
                continue;
            }
            if (ep.getDirection() == UsbConstants.USB_DIR_IN) {
                in = ep;
            } else {
                out = ep;
            }
        }
        if (in == null || out == null) {
            throw new IOException("CDC data interface has no bulk endpoints");
        }
        this.endpointIn = in;
        this.endpointOut = out;

        if (controlInterface != null) {
            // DTR + RTS: many CDC bootloaders only start their CDC machinery once the
            // control line state is asserted.
            connection.controlTransfer(REQTYPE_HOST_TO_DEVICE, SET_CONTROL_LINE_STATE,
                    0x03, controlInterface.getId(), null, 0, 1000);
        }

        readerRunning = true;
        readerThread = new Thread(this::readerLoop, "cdc-reader");
        readerThread.start();
    }

    public static boolean hasCdc(UsbDevice device) {
        for (int i = 0; i < device.getInterfaceCount(); i++) {
            int cls = device.getInterface(i).getInterfaceClass();
            if (cls == USB_CLASS_CDC_DATA || cls == USB_CLASS_COMM) {
                return true;
            }
        }
        return false;
    }

    public static boolean hasHid(UsbDevice device) {
        for (int i = 0; i < device.getInterfaceCount(); i++) {
            if (device.getInterface(i).getInterfaceClass() == USB_CLASS_HID) {
                return true;
            }
        }
        return false;
    }

    public static List<UsbDevice> cdcDevices(UsbManager manager) {
        List<UsbDevice> result = new ArrayList<>();
        for (UsbDevice device : manager.getDeviceList().values()) {
            if (hasCdc(device)) {
                result.add(device);
            }
        }
        return result;
    }

    private void readerLoop() {
        byte[] buffer = new byte[512];
        while (readerRunning) {
            int n = connection.bulkTransfer(endpointIn, buffer, buffer.length, 1000);
            if (n > 0) {
                for (int i = 0; i < n; i++) {
                    rxQueue.add((int) (buffer[i] & 0xFF));
                }
            }
        }
    }

    public boolean write(String text) {
        // Binary protocol bytes (block sizes, word addresses) reach 0x80+, which
        // UTF-8 encodes as two bytes and breaks the AVR910 framing; ISO-8859-1
        // maps each char to exactly one byte.
        return writeRaw(text.getBytes(StandardCharsets.ISO_8859_1));
    }

    public boolean writeRaw(byte[] bytes) {
        int offset = 0;
        byte[] buffer = new byte[Math.min(bytes.length, 1024)];
        while (offset < bytes.length) {
            int chunk = Math.min(buffer.length, bytes.length - offset);
            System.arraycopy(bytes, offset, buffer, 0, chunk);
            int sent = connection.bulkTransfer(endpointOut, buffer, chunk, 5000);
            if (sent < 0) {
                return false;
            }
            offset += chunk;
        }
        return true;
    }

    public int readByte(int timeoutMs) {
        try {
            Integer value = rxQueue.poll(timeoutMs, TimeUnit.MILLISECONDS);
            return value == null ? -1 : value;
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return -1;
        }
    }

    public int availableBytes() {
        return rxQueue.size();
    }

    public void dropPendingInput() {
        rxQueue.clear();
    }

    public UsbDevice getDevice() {
        return device;
    }

    public String describeDevice() {
        return String.format("%04x:%04x", device.getVendorId(), device.getProductId())
                + " " + device.getProductName();
    }

    /**
     * Stock Arduino sketches honour a 1200-baud line coding plus a DTR release
     * as the reboot-into-bootloader signal; Caterina itself checks no baud
     * rate, so this only works if the running firmware implements the core CDC
     * hook, otherwise the reset button must be double-pressed.
     */
    public boolean setLineCoding(int baud) {
        if (controlInterface == null) {
            return false;
        }
        byte[] encoding = {
                (byte) baud, (byte) (baud >> 8), (byte) (baud >> 16), (byte) (baud >> 24),
                0, 0, 8
        };
        return connection.controlTransfer(REQTYPE_HOST_TO_DEVICE, SET_LINE_CODING, 0,
                controlInterface.getId(), encoding, encoding.length, 1000) == encoding.length;
    }

    public boolean setControlLineState(boolean dtr, boolean rts) {
        if (controlInterface == null) {
            return false;
        }
        int value = (dtr ? 1 : 0) | (rts ? 2 : 0);
        return connection.controlTransfer(REQTYPE_HOST_TO_DEVICE, SET_CONTROL_LINE_STATE, value,
                controlInterface.getId(), null, 0, 1000) >= 0;
    }

    public void close() {
        readerRunning = false;
        try {
            if (readerThread != null) {
                readerThread.interrupt();
            }
        } catch (Exception ignored) {
        }
        if (controlInterface != null) {
            connection.releaseInterface(controlInterface);
        }
        connection.releaseInterface(dataInterface);
        connection.close();
    }
}
