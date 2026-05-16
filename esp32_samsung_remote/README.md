# ESP32 Samsung Legacy Remote

ESP32 web remote for older Samsung TVs using the legacy TCP protocol on port `55000`.

This version does not use Flask or `samsungctl`. It sends the legacy packets directly from ESP32.

## Setup

1. Open `esp32_samsung_remote.ino` in Arduino IDE.
2. Install/select an ESP32 board package.
3. Edit:

```cpp
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

4. Upload to ESP32.
5. Open Serial Monitor at `115200`.
6. Visit the printed URL, for example:

```text
http://192.168.10.x
```

## TV Config

The sketch targets:

```cpp
const char *TV_HOST = "192.168.10.111";
const uint16_t TV_PORT = 55000;
```

## Endpoints

```text
GET /
GET /status
GET /key/KEY_VOLUP
GET /key/KEY_HOME
```

The first connection may show a permission prompt on the Samsung TV. Allow the remote named `Web Remote`.
