#include <WebServer.h>
#include <WiFi.h>
#include <mbedtls/base64.h>
#include <vector>

const char *WIFI_SSID = "Jirayu_2.4G";
const char *WIFI_PASSWORD = "0806498701";

const char *TV_HOST = "192.168.10.111";
const uint16_t TV_PORT = 55000;

const char *REMOTE_NAME = "Web Remote";
const char *REMOTE_DESCRIPTION = "Samsung TV Web Remote";
const char *REMOTE_ID = "webremote";

WebServer server(80);

using Bytes = std::vector<uint8_t>;

const char *ALLOWED_KEYS[] = {
  "KEY_POWER",
  "KEY_POWEROFF",
  "KEY_HOME",
  "KEY_CONTENTS",
  "KEY_MENU",
  "KEY_MUTE",
  "KEY_VOLUP",
  "KEY_VOLDOWN",
  "KEY_CHUP",
  "KEY_CHDOWN",
  "KEY_SOURCE",
  "KEY_UP",
  "KEY_DOWN",
  "KEY_LEFT",
  "KEY_RIGHT",
  "KEY_ENTER",
  "KEY_YT_BROWSER",
  "KEY_YOUTUBE",
  "KEY_NETFLIX",
  "KEY_WWW",
  "KEY_SEARCH",
  "KEY_GOOGLE",
};

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Samsung TV Remote</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #090d12;
      --panel: rgba(18, 25, 34, .78);
      --button: rgba(35, 47, 61, .86);
      --line: rgba(255,255,255,.14);
      --text: #f5f7fb;
      --muted: #a3adba;
      --green: #46d98c;
      --red: #ff5f6d;
      --blue: #64b5ff;
    }
    * { box-sizing: border-box; }
    body {
      min-height: 100vh;
      margin: 0;
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at 15% 12%, rgba(100,181,255,.22), transparent 25rem),
        radial-gradient(circle at 88% 10%, rgba(70,217,140,.13), transparent 23rem),
        linear-gradient(145deg, #080b10, #121821 55%, #07090c);
      display: grid;
      place-items: center;
      padding: 18px;
    }
    .remote {
      width: min(430px, 100%);
      padding: 22px;
      border: 1px solid var(--line);
      border-radius: 28px;
      background: var(--panel);
      box-shadow: 0 24px 70px rgba(0,0,0,.42);
      backdrop-filter: blur(22px);
    }
    header, .screen {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
    }
    h1 { margin: 0; font-size: clamp(2rem, 8vw, 3rem); line-height: 1; }
    .eyebrow {
      margin: 0 0 4px;
      color: var(--blue);
      font-size: .76rem;
      font-weight: 800;
      letter-spacing: .12em;
      text-transform: uppercase;
    }
    .status, .screen {
      border: 1px solid var(--line);
      background: rgba(255,255,255,.06);
    }
    .status {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      min-width: 112px;
      justify-content: center;
      padding: 9px 12px;
      border-radius: 999px;
      color: var(--muted);
      font-size: .84rem;
      font-weight: 800;
    }
    .dot {
      width: 9px;
      height: 9px;
      border-radius: 999px;
      background: #f4c76b;
      box-shadow: 0 0 16px #f4c76b;
    }
    .online { color: var(--green); }
    .online .dot { background: var(--green); box-shadow: 0 0 18px var(--green); }
    .offline { color: var(--red); }
    .offline .dot { background: var(--red); box-shadow: 0 0 18px var(--red); }
    .screen {
      margin: 18px 0 20px;
      padding: 15px;
      border-radius: 18px;
    }
    .screen span { display: block; color: var(--muted); font-size: .78rem; font-weight: 800; text-transform: uppercase; }
    button {
      border: 1px solid var(--line);
      color: var(--text);
      background: var(--button);
      box-shadow: inset 0 1px 0 rgba(255,255,255,.1), 0 12px 28px rgba(0,0,0,.22);
      cursor: pointer;
      transition: transform .16s ease, border-color .16s ease, background .16s ease;
      -webkit-tap-highlight-color: transparent;
      font: inherit;
      font-weight: 850;
    }
    button:hover { transform: translateY(-2px); border-color: rgba(100,181,255,.55); background: rgba(47,61,78,.9); }
    button:active, .pressing { transform: scale(.95); }
    .top { display: grid; grid-template-columns: repeat(4, 1fr); gap: 12px; }
    .shortcuts { margin-top: 12px; }
    .tile { min-height: 76px; border-radius: 18px; display: grid; place-items: center; gap: 4px; padding: 10px; }
    .power { background: rgba(99,32,42,.86); }
    .pad {
      display: grid;
      grid-template-columns: repeat(3, 82px);
      grid-template-rows: repeat(3, 82px);
      justify-content: center;
      gap: 10px;
      margin: 24px 0;
    }
    .round, .ok { width: 82px; height: 82px; border-radius: 999px; font-size: 2rem; }
    .ok { grid-column: 2; grid-row: 2; color: #071014; background: linear-gradient(135deg, #8ed7ff, #4ce7a1); }
    .up { grid-column: 2; grid-row: 1; }
    .left { grid-column: 1; grid-row: 2; }
    .right { grid-column: 3; grid-row: 2; }
    .down { grid-column: 2; grid-row: 3; }
    .rockers { display: grid; grid-template-columns: 1fr .82fr 1fr; gap: 12px; }
    .rocker {
      display: grid;
      grid-template-rows: 70px auto 70px;
      gap: 8px;
      align-items: center;
      justify-items: center;
      padding: 10px;
      border: 1px solid var(--line);
      border-radius: 22px;
      background: rgba(255,255,255,.05);
    }
    .rocker span { color: var(--muted); font-size: .8rem; font-weight: 850; text-transform: uppercase; }
    .rocker button { width: 100%; height: 70px; border-radius: 18px; font-size: 2rem; }
    .mute { border-radius: 22px; min-height: 100%; }
    .toast {
      position: fixed;
      right: 16px;
      bottom: 16px;
      max-width: min(330px, calc(100vw - 32px));
      padding: 13px 15px;
      border: 1px solid var(--line);
      border-radius: 16px;
      background: rgba(18,25,34,.95);
      box-shadow: 0 20px 60px rgba(0,0,0,.35);
      opacity: 0;
      transform: translateY(12px);
      transition: .2s ease;
      pointer-events: none;
    }
    .toast.show { opacity: 1; transform: translateY(0); }
    @media (max-width: 430px) {
      .remote { padding: 18px; }
      header, .screen { align-items: stretch; flex-direction: column; }
      .status { width: 100%; }
      .top { grid-template-columns: repeat(2, 1fr); }
      .pad { grid-template-columns: repeat(3, minmax(66px, 78px)); grid-template-rows: repeat(3, minmax(66px, 78px)); }
      .round, .ok { width: 100%; height: 100%; }
      .rockers { grid-template-columns: 1fr 1fr; }
      .mute { grid-column: 1 / -1; min-height: 76px; }
    }
  </style>
</head>
<body>
  <main class="remote">
    <header>
      <div><p class="eyebrow">Samsung Smart TV</p><h1>Remote</h1></div>
      <div class="status" id="status"><span class="dot"></span><span id="statusText">Checking</span></div>
    </header>
    <section class="screen">
      <div><span>Legacy mode</span><strong>192.168.10.111:55000</strong></div>
      <button class="tile" onclick="checkStatus(true)">Reconnect</button>
    </section>
    <section class="top">
      <button class="tile power" data-key="KEY_POWEROFF">Power</button>
      <button class="tile" data-key="KEY_CONTENTS">Home</button>
      <button class="tile" data-key="KEY_MENU">Menu</button>
      <button class="tile" data-key="KEY_SOURCE">Source</button>
    </section>
    <section class="top shortcuts">
      <button class="tile" data-key="KEY_YT_BROWSER">YouTube</button>
      <button class="tile" data-key="KEY_NETFLIX">Netflix</button>
      <button class="tile" data-key="KEY_WWW">Browser</button>
      <button class="tile" data-key="KEY_SEARCH">Search</button>
    </section>
    <section class="pad">
      <button class="round up" data-key="KEY_UP">⌃</button>
      <button class="round left" data-key="KEY_LEFT">‹</button>
      <button class="ok" data-key="KEY_ENTER">OK</button>
      <button class="round right" data-key="KEY_RIGHT">›</button>
      <button class="round down" data-key="KEY_DOWN">⌄</button>
    </section>
    <section class="rockers">
      <div class="rocker"><button data-key="KEY_VOLUP">+</button><span>Volume</span><button data-key="KEY_VOLDOWN">−</button></div>
      <button class="mute" data-key="KEY_MUTE">Mute</button>
      <div class="rocker"><button data-key="KEY_CHUP">+</button><span>Channel</span><button data-key="KEY_CHDOWN">−</button></div>
    </section>
  </main>
  <div class="toast" id="toast"></div>
  <script>
    const statusBox = document.getElementById("status");
    const statusText = document.getElementById("statusText");
    const toast = document.getElementById("toast");
    const labels = {
      KEY_POWER: "Power", KEY_POWEROFF: "Power", KEY_HOME: "Home", KEY_CONTENTS: "Home",
      KEY_MENU: "Menu", KEY_MUTE: "Mute",
      KEY_VOLUP: "Volume Up", KEY_VOLDOWN: "Volume Down", KEY_CHUP: "Channel Up",
      KEY_CHDOWN: "Channel Down", KEY_SOURCE: "Source", KEY_UP: "Up",
      KEY_DOWN: "Down", KEY_LEFT: "Left", KEY_RIGHT: "Right", KEY_ENTER: "OK",
      KEY_YT_BROWSER: "YouTube", KEY_YOUTUBE: "YouTube App", KEY_NETFLIX: "Netflix", KEY_WWW: "Browser",
      KEY_SEARCH: "Search", KEY_GOOGLE: "Google"
    };
    function setStatus(state) {
      statusBox.className = "status " + state;
      statusText.textContent = state === "online" ? "Online" : state === "offline" ? "Offline" : "Checking";
    }
    function notify(message) {
      toast.textContent = message;
      toast.classList.add("show");
      clearTimeout(window.toastTimer);
      window.toastTimer = setTimeout(() => toast.classList.remove("show"), 2200);
    }
    async function checkStatus(showToast = false) {
      setStatus("");
      try {
        const res = await fetch("/status", { cache: "no-store" });
        const data = await res.json();
        setStatus(data.connected ? "online" : "offline");
        if (showToast) notify(data.connected ? "TV reachable" : "TV offline");
      } catch {
        setStatus("offline");
        if (showToast) notify("Status check failed");
      }
    }
    async function sendKey(key, button) {
      button.classList.add("pressing");
      try {
        const res = await fetch("/key/" + encodeURIComponent(key), { cache: "no-store" });
        const data = await res.json();
        if (!res.ok || !data.ok) throw new Error(data.error || "Command failed");
        setStatus("online");
        notify((labels[key] || key) + " sent");
      } catch (error) {
        setStatus("offline");
        notify(error.message || "TV offline");
      } finally {
        setTimeout(() => button.classList.remove("pressing"), 150);
      }
    }
    document.querySelectorAll("[data-key]").forEach(button => {
      button.addEventListener("click", () => sendKey(button.dataset.key, button));
    });
    checkStatus();
    setInterval(checkStatus, 15000);
  </script>
</body>
</html>
)HTML";

void appendByte(Bytes &out, uint8_t value) {
  out.push_back(value);
}

String base64Encode(const String &input) {
  size_t outputLength = 0;
  mbedtls_base64_encode(
    nullptr,
    0,
    &outputLength,
    reinterpret_cast<const unsigned char *>(input.c_str()),
    input.length()
  );

  unsigned char output[256];
  if (outputLength >= sizeof(output)) {
    return "";
  }

  if (mbedtls_base64_encode(
        output,
        sizeof(output),
        &outputLength,
        reinterpret_cast<const unsigned char *>(input.c_str()),
        input.length()
      ) != 0) {
    return "";
  }

  String encoded;
  encoded.reserve(outputLength);
  for (size_t i = 0; i < outputLength; i++) {
    encoded += static_cast<char>(output[i]);
  }
  return encoded;
}

Bytes serializeString(const String &value) {
  String encoded = base64Encode(value);
  Bytes out;
  appendByte(out, static_cast<uint8_t>(encoded.length()));
  appendByte(out, 0x00);
  for (size_t i = 0; i < encoded.length(); i++) {
    appendByte(out, static_cast<uint8_t>(encoded[i]));
  }
  return out;
}

Bytes serializeRaw(const Bytes &value) {
  Bytes out;
  appendByte(out, static_cast<uint8_t>(value.size()));
  appendByte(out, 0x00);
  out.insert(out.end(), value.begin(), value.end());
  return out;
}

void appendBytes(Bytes &out, const Bytes &value) {
  out.insert(out.end(), value.begin(), value.end());
}

Bytes makeHandshakePacket() {
  Bytes payload;
  appendByte(payload, 0x64);
  appendByte(payload, 0x00);
  appendBytes(payload, serializeString(REMOTE_DESCRIPTION));
  appendBytes(payload, serializeString(REMOTE_ID));
  appendBytes(payload, serializeString(REMOTE_NAME));

  Bytes packet;
  appendByte(packet, 0x00);
  appendByte(packet, 0x00);
  appendByte(packet, 0x00);
  appendBytes(packet, serializeRaw(payload));
  return packet;
}

Bytes makeControlPacket(const String &key) {
  Bytes payload;
  appendByte(payload, 0x00);
  appendByte(payload, 0x00);
  appendByte(payload, 0x00);
  appendBytes(payload, serializeString(key));

  Bytes packet;
  appendByte(packet, 0x00);
  appendByte(packet, 0x00);
  appendByte(packet, 0x00);
  appendBytes(packet, serializeRaw(payload));
  return packet;
}

bool writePacket(WiFiClient &client, const Bytes &packet) {
  return client.write(packet.data(), packet.size()) == packet.size();
}

bool readExact(WiFiClient &client, uint8_t *buffer, size_t length, uint32_t timeoutMs = 5000) {
  size_t offset = 0;
  uint32_t startedAt = millis();

  while (offset < length && millis() - startedAt < timeoutMs) {
    if (client.available()) {
      offset += client.read(buffer + offset, length - offset);
    } else {
      delay(5);
    }
  }

  return offset == length;
}

bool readSamsungResponse(WiFiClient &client, bool firstTime, String &error) {
  uint8_t header[3];
  if (!readExact(client, header, sizeof(header))) {
    error = "No response header from TV";
    return false;
  }

  uint16_t tvNameLength = header[1] | (header[2] << 8);
  uint8_t ignoredName[128];
  while (tvNameLength > 0) {
    size_t chunk = min<size_t>(tvNameLength, sizeof(ignoredName));
    if (!readExact(client, ignoredName, chunk)) {
      error = "Incomplete TV name response";
      return false;
    }
    tvNameLength -= chunk;
  }

  uint8_t lengthBytes[2];
  if (!readExact(client, lengthBytes, sizeof(lengthBytes))) {
    error = "No response length from TV";
    return false;
  }

  uint16_t responseLength = lengthBytes[0] | (lengthBytes[1] << 8);
  if (responseLength == 0 || responseLength > 32) {
    error = "Invalid TV response";
    return false;
  }

  uint8_t response[32];
  if (!readExact(client, response, responseLength)) {
    error = "Incomplete TV response";
    return false;
  }

  if (responseLength == 4 && response[0] == 0x64 && response[1] == 0x00 && response[2] == 0x01 && response[3] == 0x00) {
    return true;
  }

  if (responseLength == 4 && response[0] == 0x00 && response[1] == 0x00 && response[2] == 0x00 && response[3] == 0x00) {
    return true;
  }

  if (response[0] == 0x0a && firstTime) {
    return readSamsungResponse(client, false, error);
  }

  if (responseLength == 4 && response[0] == 0x64 && response[1] == 0x00 && response[2] == 0x00 && response[3] == 0x00) {
    error = "TV denied remote access";
    return false;
  }

  if (response[0] == 0x65) {
    error = "TV authorization was cancelled";
    return false;
  }

  error = "Unhandled TV response";
  return false;
}

bool isAllowedKey(const String &key) {
  for (const char *allowedKey : ALLOWED_KEYS) {
    if (key == allowedKey) {
      return true;
    }
  }
  return false;
}

bool checkTvReachable() {
  WiFiClient client;
  client.setTimeout(3000);
  bool connected = client.connect(TV_HOST, TV_PORT);
  client.stop();
  return connected;
}

bool sendSamsungKey(const String &key, String &error) {
  WiFiClient client;
  client.setTimeout(5000);

  Serial.print("Sending key: ");
  Serial.println(key);

  if (!client.connect(TV_HOST, TV_PORT)) {
    error = "Could not connect to TV";
    Serial.println(error);
    return false;
  }

  Bytes handshake = makeHandshakePacket();
  if (!writePacket(client, handshake)) {
    error = "Could not send handshake";
    Serial.println(error);
    client.stop();
    return false;
  }

  if (!readSamsungResponse(client, true, error)) {
    Serial.print("Handshake failed: ");
    Serial.println(error);
    client.stop();
    return false;
  }

  Bytes command = makeControlPacket(key);
  if (!writePacket(client, command)) {
    error = "Could not send command";
    Serial.println(error);
    client.stop();
    return false;
  }

  bool ok = readSamsungResponse(client, false, error);
  if (ok) {
    Serial.println("Command accepted");
  } else {
    Serial.print("Command failed: ");
    Serial.println(error);
  }
  client.stop();
  delay(200);
  return ok;
}

void sendJson(int code, const String &json) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(code, "application/json", json);
}

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleYoutubeRedirect() {
  server.sendHeader("Location", "https://www.youtube.com/tv", true);
  server.send(302, "text/plain", "Opening YouTube...");
}

void handleStatus() {
  bool connected = checkTvReachable();
  sendJson(
    200,
    String("{\"connected\":") + (connected ? "true" : "false") +
      ",\"host\":\"" + TV_HOST +
      "\",\"port\":55000,\"method\":\"legacy\"}"
  );
}

void handleKey() {
  String path = server.uri();
  String key = path.substring(String("/key/").length());
  key.toUpperCase();

  Serial.print("HTTP key request: ");
  Serial.println(key);

  if (!isAllowedKey(key)) {
    sendJson(400, "{\"ok\":false,\"error\":\"Unsupported key\"}");
    return;
  }

  if (key == "KEY_YT_BROWSER") {
    String error;
    if (!sendSamsungKey("KEY_WWW", error)) {
      sendJson(503, String("{\"ok\":false,\"key\":\"KEY_YT_BROWSER\",\"error\":\"") + error + "\"}");
      return;
    }

    sendJson(200, "{\"ok\":true,\"key\":\"KEY_YT_BROWSER\",\"mode\":\"browser\",\"redirect\":\"/yt\"}");
    return;
  }

  String error;
  if (!sendSamsungKey(key, error)) {
    sendJson(503, String("{\"ok\":false,\"key\":\"") + key + "\",\"error\":\"" + error + "\"}");
    return;
  }

  sendJson(200, String("{\"ok\":true,\"key\":\"") + key + "\"}");
}

void handleNotFound() {
  if (server.uri().startsWith("/key/")) {
    handleKey();
    return;
  }
  sendJson(404, "{\"ok\":false,\"error\":\"Not found\"}");
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("ESP32 remote: http://");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  connectWifi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/yt", HTTP_GET, handleYoutubeRedirect);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}
