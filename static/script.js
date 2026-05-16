const statusPill = document.querySelector("#statusPill");
const statusText = document.querySelector("#statusText");
const reconnectButton = document.querySelector("#reconnectButton");
const toastStack = document.querySelector("#toastStack");
const keyButtons = document.querySelectorAll("[data-key]");

const keyLabels = {
  KEY_POWER: "Power",
  KEY_HOME: "Home",
  KEY_MENU: "Menu",
  KEY_MUTE: "Mute",
  KEY_VOLUP: "Volume Up",
  KEY_VOLDOWN: "Volume Down",
  KEY_CHUP: "Channel Up",
  KEY_CHDOWN: "Channel Down",
  KEY_SOURCE: "Source",
  KEY_UP: "Up",
  KEY_DOWN: "Down",
  KEY_LEFT: "Left",
  KEY_RIGHT: "Right",
  KEY_ENTER: "OK",
};

let reconnectTimer;
let isCheckingStatus = false;

function setStatus(state) {
  statusPill.classList.remove("is-online", "is-offline");

  if (state === "online") {
    statusPill.classList.add("is-online");
    statusText.textContent = "Online";
    return;
  }

  if (state === "offline") {
    statusPill.classList.add("is-offline");
    statusText.textContent = "Offline";
    return;
  }

  statusText.textContent = "Checking";
}

function showToast(title, message, type = "success") {
  const toast = document.createElement("div");
  toast.className = `toast ${type === "error" ? "toast--error" : ""}`;
  toast.innerHTML = `<strong>${title}</strong><span>${message}</span>`;
  toastStack.appendChild(toast);

  window.setTimeout(() => {
    toast.style.opacity = "0";
    toast.style.transform = "translateY(8px) scale(0.98)";
  }, 2600);

  window.setTimeout(() => toast.remove(), 3000);
}

async function checkStatus({ notify = false } = {}) {
  if (isCheckingStatus) return;
  isCheckingStatus = true;
  setStatus("checking");

  try {
    const response = await fetch("/status", { cache: "no-store" });
    const data = await response.json();
    setStatus(data.connected ? "online" : "offline");

    if (notify) {
      showToast(
        data.connected ? "TV connected" : "TV offline",
        data.connected ? "Remote is ready." : "Check that the TV is on and reachable.",
        data.connected ? "success" : "error",
      );
    }
  } catch (error) {
    setStatus("offline");
    if (notify) {
      showToast("Connection failed", "The Flask server could not check the TV.", "error");
    }
  } finally {
    isCheckingStatus = false;
  }
}

async function sendKey(key, button) {
  const label = keyLabels[key] || key;
  button.classList.add("is-pressing");

  try {
    const response = await fetch(`/key/${encodeURIComponent(key)}`, { cache: "no-store" });
    const data = await response.json().catch(() => ({}));

    if (!response.ok || !data.ok) {
      throw new Error(data.error || "The TV did not accept the command.");
    }

    setStatus("online");
    showToast(`${label} sent`, "Command delivered to the TV.");
  } catch (error) {
    setStatus("offline");
    showToast(`${label} failed`, error.message || "Reconnect and try again.", "error");
    scheduleReconnect();
  } finally {
    window.setTimeout(() => button.classList.remove("is-pressing"), 160);
  }
}

function scheduleReconnect() {
  window.clearTimeout(reconnectTimer);
  reconnectTimer = window.setTimeout(() => checkStatus(), 3500);
}

keyButtons.forEach((button) => {
  button.addEventListener("click", () => sendKey(button.dataset.key, button));
});

reconnectButton.addEventListener("click", () => checkStatus({ notify: true }));

checkStatus();
window.setInterval(() => checkStatus(), 15000);
