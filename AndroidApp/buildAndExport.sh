#!/bin/sh
cd "$(dirname "$0")"
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-arm64
OUT_DIR=/storage/emulated/0/builtAPKs
mkdir -p "$OUT_DIR"
./gradlew --no-daemon assembleDebug && cp app/build/outputs/apk/debug/app-debug.apk "$OUT_DIR/fairberry-kbdmanager.apk"
