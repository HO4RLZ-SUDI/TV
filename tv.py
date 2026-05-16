num = input()
import samsungctl

config = {
    "name": "Mac Remote",
    "description": "Mac Remote",
    "id": "mac",
    "host": "192.168.10.111",
    "port": 55000,
    "method": "legacy",
    "timeout": 30,
}

for i in (num):
    with samsungctl.Remote(config) as remote:
        remote.control("KEY_VOLUP")