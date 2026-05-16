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
GET /start
GET /open/youtube
GET /open/netflix
GET /open/google
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

The website buttons use this flow:

1. ESP32 saves the selected URL.
2. ESP32 sends `KEY_WWW` to open the TV browser.
3. The TV browser opens its homepage.
4. If that homepage is `http://<ESP32-IP>/start`, ESP32 redirects the TV to the selected site.

Set the TV browser homepage once to:

```text
http://<ESP32-IP>/start
```

Then the website buttons can open:

```text
YouTube -> https://www.youtube.com/tv
Netflix -> https://www.netflix.com
Google  -> https://www.google.com
Prime   -> https://www.primevideo.com
Disney+ -> https://www.disneyplus.com
```

Samsung legacy remote protocol cannot reliably type a URL into the browser address bar. The `/start` homepage redirect is the workaround.

App keys such as `KEY_YOUTUBE` and `KEY_NETFLIX` are model-dependent. Some Samsung legacy TVs ignore them even when normal remote keys work.
