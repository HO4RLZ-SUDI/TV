#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import socket
import sys
import textwrap
import time
import urllib.error
import urllib.request
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from typing import Any


DEFAULT_HOST = "192.168.10.111"

PORT_HINTS = {
    55000: "Legacy Samsung remote, common on older Orsay/pre-Tizen TVs",
    8000: "Samsung HTTP/WebSocket control, older smart TV variants",
    8001: "Tizen remote WebSocket/API v2 over HTTP",
    8002: "Tizen remote WebSocket/API v2 over HTTPS",
    7676: "Samsung device service / multiscreen on some models",
    9197: "Samsung remote/accessory service on some models",
}


@dataclass
class PortResult:
    port: int
    open: bool
    hint: str


def tcp_open(host: str, port: int, timeout: float) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


def http_get(url: str, timeout: float) -> tuple[int | None, str]:
    request = urllib.request.Request(
        url,
        headers={
            "User-Agent": "Samsung-TV-Checker/1.0",
            "Accept": "application/json,text/xml,text/plain,*/*",
        },
    )
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            body = response.read(65536).decode("utf-8", errors="replace")
            return response.status, body
    except urllib.error.HTTPError as exc:
        body = exc.read(65536).decode("utf-8", errors="replace")
        return exc.code, body
    except Exception as exc:
        return None, str(exc)


def try_api_v2(host: str, timeout: float) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for port, scheme in ((8001, "http"), (8002, "https"), (8000, "http")):
        url = f"{scheme}://{host}:{port}/api/v2/"
        status, body = http_get(url, timeout)
        item: dict[str, Any] = {"url": url, "status": status}

        if status and body:
            try:
                item["json"] = json.loads(body)
            except json.JSONDecodeError:
                item["body"] = body[:500]
        elif body:
            item["error"] = body

        results.append(item)
    return results


def ssdp_locations(host: str, timeout: float) -> list[str]:
    message = "\r\n".join(
        [
            "M-SEARCH * HTTP/1.1",
            "HOST: 239.255.255.250:1900",
            'MAN: "ssdp:discover"',
            "MX: 2",
            "ST: ssdp:all",
            "",
            "",
        ]
    ).encode()

    locations: set[str] = set()
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.settimeout(0.35)

    try:
        sock.sendto(message, ("239.255.255.250", 1900))
        deadline = time.time() + timeout

        while time.time() < deadline:
            try:
                data, address = sock.recvfrom(8192)
            except socket.timeout:
                continue

            if address[0] != host:
                continue

            text = data.decode("utf-8", errors="replace")
            for line in text.splitlines():
                if line.lower().startswith("location:"):
                    locations.add(line.split(":", 1)[1].strip())
    finally:
        sock.close()

    return sorted(locations)


def parse_device_xml(xml_text: str) -> dict[str, str]:
    try:
        root = ET.fromstring(xml_text)
    except ET.ParseError:
        return {}

    fields = {
        "friendlyName",
        "manufacturer",
        "modelName",
        "modelNumber",
        "modelDescription",
        "serialNumber",
        "UDN",
    }
    out: dict[str, str] = {}

    for element in root.iter():
        tag = element.tag.rsplit("}", 1)[-1]
        if tag in fields and element.text:
            out[tag] = element.text.strip()

    return out


def print_json_block(title: str, data: Any) -> None:
    print(f"\n{title}")
    print("-" * len(title))
    print(json.dumps(data, indent=2, ensure_ascii=False))


def infer_platform(ports: list[PortResult], api_results: list[dict[str, Any]]) -> str:
    open_ports = {result.port for result in ports if result.open}

    api_v2_ok = any(
        result.get("status") == 200 and isinstance(result.get("json"), dict)
        for result in api_results
    )

    if api_v2_ok or 8001 in open_ports or 8002 in open_ports:
        if 55000 in open_ports:
            return "Likely Samsung smart TV with Tizen/newer remote API plus legacy compatibility."
        return "Likely Tizen/newer Samsung TV remote API."

    if 55000 in open_ports:
        return "Likely legacy Samsung Smart TV remote API, often Orsay/pre-Tizen era."

    if 8000 in open_ports or 7676 in open_ports or 9197 in open_ports:
        return "Samsung smart TV services found, but platform is unclear from ports alone."

    return "No common Samsung remote ports found. TV may be offline, blocked by network, or using another control mode."


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Check Samsung TV network services and infer legacy/Tizen support."
    )
    parser.add_argument("host", nargs="?", default=DEFAULT_HOST)
    parser.add_argument("--timeout", type=float, default=3.0)
    args = parser.parse_args()

    host = args.host
    timeout = args.timeout

    print(f"Checking Samsung TV at {host}\n")

    ports = [
        PortResult(port=port, open=tcp_open(host, port, timeout), hint=hint)
        for port, hint in PORT_HINTS.items()
    ]

    print("Ports")
    print("-----")
    for result in ports:
        state = "OPEN " if result.open else "closed"
        print(f"{result.port:<5} {state}  {result.hint}")

    api_results = try_api_v2(host, timeout)
    print_json_block("API v2 probes", api_results)

    locations = ssdp_locations(host, timeout)
    print_json_block("SSDP locations", locations)

    device_infos = []
    for location in locations:
        status, body = http_get(location, timeout)
        info = {"location": location, "status": status}
        if status == 200:
            parsed = parse_device_xml(body)
            if parsed:
                info["device"] = parsed
            else:
                info["body"] = body[:500]
        elif body:
            info["error"] = body[:500]
        device_infos.append(info)

    print_json_block("UPnP device info", device_infos)

    print("\nInference")
    print("---------")
    print(textwrap.fill(infer_platform(ports, api_results), width=88))

    print("\nNotes")
    print("-----")
    print("- Port 55000 open usually means legacy Samsung remote mode works.")
    print("- Port 8001/8002 or /api/v2/ usually points to Tizen/newer Samsung remote API.")
    print("- Model info from SSDP/UPnP is the best clue if the TV exposes it.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
