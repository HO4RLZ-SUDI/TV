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
GET /key/KEY_YOUTUBE
GET /key/KEY_YT_BROWSER
GET /key/KEY_NETFLIX
GET /key/KEY_WWW
GET /key/KEY_SEARCH
GET /yt
```

The first connection may show a permission prompt on the Samsung TV. Allow the remote named `Web Remote`.

## Legacy Key Notes

Older Samsung legacy TVs often use:

```text
Power button -> KEY_POWEROFF
Home button  -> KEY_CONTENTS
```

`KEY_POWER` and `KEY_HOME` are still allowed by the firmware endpoint, but the web UI uses the legacy-compatible keys above by default.

Power-on usually does not work over legacy TCP if the TV turns off its network adapter while powered down.

## App Shortcuts

The web UI includes shortcut buttons for:

```text
YouTube -> KEY_YT_BROWSER
Netflix -> KEY_NETFLIX
Browser -> KEY_WWW
Search  -> KEY_SEARCH
```

The YouTube button opens the TV browser with `KEY_WWW`. The ESP32 also serves:

```text
http://<ESP32-IP>/yt
```

That route redirects to:

```text
https://www.youtube.com/tv
```

Samsung legacy remote protocol cannot reliably type a URL into the browser address bar. For one-button YouTube through the browser, set the TV browser homepage or a browser bookmark to `http://<ESP32-IP>/yt` once. After that, the YouTube button opens the browser and the browser can land on YouTube.

App keys such as `KEY_YOUTUBE` and `KEY_NETFLIX` are model-dependent. Some Samsung legacy TVs ignore them even when normal remote keys work.
