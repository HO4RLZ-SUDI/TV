#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKETCH_DIR="${SKETCH_DIR:-$ROOT_DIR/esp32_samsung_remote}"
FQBN="${FQBN:-esp32:esp32:esp32}"
PORT="${1:-${PORT:-}}"

if [[ -x "$ROOT_DIR/bin/arduino-cli" ]]; then
  ARDUINO_CLI="$ROOT_DIR/bin/arduino-cli"
elif command -v arduino-cli >/dev/null 2>&1; then
  ARDUINO_CLI="arduino-cli"
else
  echo "arduino-cli not found."
  echo "Install it or place it at: $ROOT_DIR/bin/arduino-cli"
  exit 1
fi

if [[ -z "$PORT" ]]; then
  PORT="$("$ARDUINO_CLI" board list | awk '/(ttyUSB|ttyACM|cu\.|COM[0-9]+)/ {print $1; exit}')"
fi

if [[ -z "$PORT" ]]; then
  echo "No board port found."
  echo "Usage: ./up.sh /dev/ttyUSB0"
  echo "Or:    PORT=/dev/ttyUSB0 ./up.sh"
  exit 1
fi

echo "Sketch: $SKETCH_DIR"
echo "Board:  $FQBN"
echo "Port:   $PORT"

"$ARDUINO_CLI" compile --fqbn "$FQBN" "$SKETCH_DIR"
"$ARDUINO_CLI" upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"

echo "Upload complete."
