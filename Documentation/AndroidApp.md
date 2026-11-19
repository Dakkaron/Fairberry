# Fairberry Android companion app

The Android app (`AndroidApp/`) manages the Fairberry keyboard over its USB
connection: it reads and edits the keymap live through the keyboard's CDC
serial interface, persists it to the keyboard's EEPROM, and can flash new
firmware images through the Caterina USB bootloader — all from the phone, no
computer needed.

- Package: `com.fairberry.kbdmanager` (Java, minSdk 27)
- Requires a phone with USB host support (OTG).

## Building

```sh
sh AndroidApp/buildAndExport.sh
```

The APK is written to `/storage/emulated/0/builtAPKs/fairberry-kbdmanager.apk`.
The script uses the Android SDK at `/root/android-sdk` and the system aapt2
(see the `aapt2FromMavenOverride` setting in `gradle.properties`).

## Connecting

The keyboard presents two USB faces, and the app detects which one it is:

| USB interfaces | Meaning | App behaviour |
|---|---|---|
| CDC-ACM + HID | normal sketch mode | keymap editor is enabled |
| CDC-ACM + HID, protocol silent | Leonardo running stock firmware | flash-only mode |
| CDC-ACM only | Caterina bootloader | flashing is enabled |

**Kernel driver caveat:** many Android builds bind CDC-ACM devices to the
in-kernel `cdc-acm` driver, which claims the interfaces before any app can.
The app requests the interfaces with `claimInterface(force=true)`; on a
non-rooted device this fails when `cdc-acm` holds them. The device this
project was tested on (non-rooted Samsung) does not bind `cdc-acm` to the
keyboard, so the claim succeeds there. If you see "Could not claim USB
interface", either run on a rooted device or blacklist `cdc-acm`.

The app auto-connects when a supported keyboard is plugged in (device filter
VIDs `0x2310`, `0x2341`, `0x2A03`), keeps the keyboard awake with an `I` ping
every few seconds, and shows a command log. Permissions are per device
instance, so every Caterina re-enumeration needs its permission dialog
confirmed once within the 8 s bootloader window.

## Editing the keymap

The keymap has 5 columns × 7 rows in three modes (Normal, Symbol, Cursor).
The app renders the grid in the physical Q10 keycap shape, not as the scan
matrix (which does not match the keycaps), with each key labelled by its
keycap name; `PhysicalLayout.java` holds the keycap → matrix cell mapping:

```
row 1:  Q W E R T Y U I O P
row 2:  A S D F G H J K L Backspace
row 3:  Alt Z X C V B N M $ Enter
row 4:  LeftShift Ctrl Space Sym RightShift
```

Tap a key to pick a new binding from a preset list (letters, digits, shifted
symbols, arrows/F-keys, pure modifiers, or empty); the dialog title shows the
keycap name and its matrix cell. Edits are local until **Apply** writes them
to the keyboard RAM, **Save EEPROM** persists them (survives reboot),
**Factory** loads the firmware defaults, and **Reload** re-reads the keyboard
state.

Modifier bindings (the Ctrl/Shift/Alt keys on the keyboard) are stored as
modifier bitmask entries, not as values — see the protocol below.

## Serial keymap protocol

Plain text lines over the USB CDC interface (the same port the Arduino IDE
would use). Implemented in `BBQ10/protocol.h`, mirrored by `KeymapClient.java`.

Each keymap entry is `value mods`. The value domain is the one the Arduino
Keyboard library uses ([Keyboard.h](https://github.com/arduino-libraries/Keyboard/blob/master/src/Keyboard.h)):
a printable character is its ASCII code (`a` = 97, `$` = 36, …), everything
else is a Keyboard.h constant — modifiers 128–135, and usage-page-0x07 keys as
`value − 136` (Enter 176/0xB0, Esc 0xB1, Backspace 0xB2, Tab 0xB3, F1–F12
0xC2–0xCD, arrows 0xD7–0xDA, F15 0xF2, …). `0 0` means "no key". This matters
because the firmware sends ASCII values through the US layout lookup: storing
the raw usage 42 for Backspace would print `*` (42 is `*` in ASCII), which is
why protocol version 1's raw-usage domain was replaced. Sending a modifier
value 128–135 in a command is automatically folded into the modifier bitmask
(bit0 LCtrl, bit1 LShift, bit2 LAlt, bit3 LGui, bit4 RCtrl, bit5 RShift,
bit6 RAlt, bit7 RGui); characters that need shift (e.g. `$`) carry the shift
themselves through the layout, and additionally honor the entry's own bitmask.

| Command | Effect | Response |
|---|---|---|
| `I` | query info | `INFO <fw> <proto> <storage>` |
| `L` | dump all modes | 105 × `K <mode> <col> <row> <value> <mods>` |
| `R <mode>` | dump one mode | 35 × `K ...` |
| `M <mode> <col> <row> <value> <mods>` | set one entry in RAM | `OK` / `ERR` |
| `D` | reload keymap from EEPROM | `OK` / `ERR` |
| `F` | load factory defaults into RAM | `OK` |
| `W` | write RAM keymap to EEPROM | `OK` |
| `T` | toggle cursor mode | `OK` |
| `Z` | soft reset (restart sketch) | `OK` |
| `B` | reboot into Caterina bootloader | `OK`, then device re-enumerates |

Commands are newline terminated; several may be sent in one write. All
arguments are decimal. Protocol version is 2 (value domain). The EEPROM
storage (AVR boards) is versioned with a magic header (`FK`) and a
CRC16-CCITT so corrupt data is ignored at boot; storage version 2 corresponds
to the value domain, and boot falls back to factory defaults when the header,
version, or CRC check fails.

Any command resets the keyboard's 30 s idle timeout, so an open app
connection keeps the keyboard responsive.

## Flashing firmware

1. Build the firmware with arduino-cli (`compile --fqbn arduino:avr:leonardo
   --libraries BBQ10/libraries --export-binaries`), which produces
   `BBQ10/build/arduino.avr.leonardo/BBQ10.ino.hex`
   (HID-Project is vendored in `BBQ10/libraries/`).
2. In the app: **Select .hex** (system file picker), then **Flash firmware**.
   A single press completes the whole flow: if no device is open yet the app
   connects first (the pending flash fires as soon as the device opens), then
   either flashes directly (bootloader mode) or reboots into the bootloader.
3. If the keyboard is running normally it first reboots itself into the
   Caterina bootloader (`B` command: writes the magic value `0x7777` to RAM
   address `0x0800` and triggers a watchdog reset — Caterina keeps the
   bootloader alive when it sees that boot key). The app reconnects to the
   bootloader automatically (confirm the permission dialog for the new device
   instance within the 8 s window) and starts flashing.
4. If the board is a Leonardo running **stock firmware** (it answers CDC but
   not the keymap protocol), the app keeps the connection in flash-only mode.
   Pressing **Flash** attempts the 1200-baud touch (`SET_LINE_CODING` 1200 +
   DTR release), which only works when the running firmware implements the
   stock Arduino core's `Serial` CDC hook — standalone Caterina checks no baud
   rate at all. Otherwise double-press the reset button: an external reset
   keeps the bootloader alive for 8 seconds. Either way, when the Caterina
   device re-appears the app reconnects and flashes automatically.
6. The app speaks the AVR910-over-CDC protocol the Caterina bootloader
   implements: handshake (`V`), page size (`b`), full chip erase of the
   application region (`e`), then per-page address set (`A`, word address) +
   block write (`B`, 256-byte pages), and finish (`E`). Every step waits for
   its acknowledgement byte.
7. After "Flash complete", unplug and replug the keyboard to run the new
   firmware.

App-region only: records at address `0x7000` and above (the bootloader itself
in `*.with_bootloader.hex`) are skipped, since Caterina protects its own
region. Flashing the plain `BBQ10.ino.hex` is the normal path.

## Source layout

```
AndroidApp/app/src/main/java/com/fairberry/kbdmanager/
  MainActivity.java      UI, connection lifecycle, USB permissions
  UsbCdc.java            CDC-ACM over the Android USB host API
  KeymapClient.java      serial keymap protocol client
  KeymapData.java        keymap model + change diffing
  KeyLabels.java         keymap value -> display label, edit presets
  PhysicalLayout.java    physical Q10 keycap positions -> matrix cells
  IntelHex.java          Intel HEX parser (skips bootloader region)
  CaterinaFlasher.java   AVR910-over-CDC flashing
```
