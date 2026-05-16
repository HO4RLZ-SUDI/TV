from __future__ import annotations

from contextlib import suppress
from typing import Any

from flask import Flask, jsonify, render_template

try:
    import samsungctl
except ImportError:  # pragma: no cover - handled at runtime with clear API errors
    samsungctl = None


app = Flask(__name__)

TV_CONFIG: dict[str, Any] = {
    "name": "Web Remote",
    "description": "Samsung TV Web Remote",
    "id": "webremote",
    "host": "192.168.10.111",
    "port": 55000,
    "method": "legacy",
    "timeout": 5,
}

ALLOWED_KEYS = {
    "KEY_POWER",
    "KEY_HOME",
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
}


def send_tv_key(key: str) -> None:
    if samsungctl is None:
        raise RuntimeError("samsungctl is not installed. Run: pip install samsungctl")

    with samsungctl.Remote(TV_CONFIG) as remote:
        remote.control(key)


def can_connect_to_tv() -> bool:
    if samsungctl is None:
        return False

    with suppress(Exception):
        with samsungctl.Remote(TV_CONFIG):
            return True
    return False


@app.get("/")
def index():
    return render_template("index.html", tv_host=TV_CONFIG["host"])


@app.get("/status")
def status():
    connected = can_connect_to_tv()
    return jsonify(
        {
            "connected": connected,
            "host": TV_CONFIG["host"],
            "port": TV_CONFIG["port"],
            "method": TV_CONFIG["method"],
        }
    )


@app.get("/key/<key>")
def key(key: str):
    key = key.upper()

    if key not in ALLOWED_KEYS:
        return jsonify({"ok": False, "error": f"Unsupported key: {key}"}), 400

    try:
        send_tv_key(key)
    except Exception as exc:
        return (
            jsonify(
                {
                    "ok": False,
                    "key": key,
                    "connected": False,
                    "error": str(exc) or exc.__class__.__name__,
                }
            ),
            503,
        )

    return jsonify({"ok": True, "key": key, "connected": True})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
