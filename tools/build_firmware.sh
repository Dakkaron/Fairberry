#!/usr/bin/env bash
# Compile the Fairberry keyboard firmware.
#
# Requires arduino-cli with the arduino:avr core installed. The needed
# libraries (HID-Project, Low-Power) are vendored under BBQ10/libraries and
# passed to the compiler via --libraries.
#
# Output: BBQ10/build/arduino.avr.leonardo/BBQ10.ino.hex
# The hex is also exported to /storage/emulated/0/builtAPKs/ for transfer to
# the phone, where the Android app's flasher picks it up via SAF.
set -e
cd "$(dirname "$0")/.."

FQBN="${FQBN:-arduino:avr:leonardo}"

if ! command -v arduino-cli >/dev/null; then
  echo "arduino-cli not found. Install it from https://github.com/arduino/arduino-cli" >&2
  exit 1
fi

if ! arduino-cli core list 2>/dev/null | grep -q "arduino:avr"; then
  echo "Installing arduino:avr core..."
  arduino-cli core install arduino:avr
fi

# proot/link2symlink environments can break arduino-cli's hardlinked tool
# installs; this repairs them from the cached tool tarballs if needed.
if [ ! -x "$(find /root/.arduino15/packages/arduino/tools/avr-gcc -name avr-g++ -type f -print -quit 2>/dev/null)" ]; then
  python3 tools/fix_arduino15_links.py || true
fi

arduino-cli compile --fqbn "$FQBN" --libraries BBQ10/libraries --export-binaries BBQ10

BUILD_HEX="BBQ10/build/$(echo "$FQBN" | sed 's/:/./g')/BBQ10.ino.hex"
OUT_DIR=/storage/emulated/0/builtAPKs
mkdir -p "$OUT_DIR"
cp "$BUILD_HEX" "$OUT_DIR/fairberry-keyboard.hex"

echo
echo "Firmware hex: $BUILD_HEX"
echo "Exported:     $OUT_DIR/fairberry-keyboard.hex"
