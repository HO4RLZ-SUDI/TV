#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT="${1:-${PORT:-}}"
BAUD="${BAUD:-115200}"

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
  echo "Usage: ./mo.sh /dev/ttyUSB0"
  echo "Or:    PORT=/dev/ttyUSB0 ./mo.sh"
  exit 1
fi

echo "Port: $PORT"
echo "Baud: $BAUD"
echo "Press Ctrl+C to stop."

"$ARDUINO_CLI" monitor -p "$PORT" -c "baudrate=$BAUD"
