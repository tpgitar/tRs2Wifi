import base64
import select
import socket
import sys
import time


def main():
    host = sys.argv[1]
    port = int(sys.argv[2])
    timeout_ms = int(sys.argv[3])
    payload = base64.b64decode(sys.argv[4], validate=True)
    timeout = timeout_ms / 1000

    with socket.create_connection((host, port), timeout=timeout) as client:
        client.settimeout(timeout)
        client.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        client.sendall(payload)

        response = bytearray()
        deadline = time.monotonic() + timeout
        while len(response) < len(payload):
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            client.settimeout(remaining)
            chunk = client.recv(4096)
            if not chunk:
                break
            response.extend(chunk)

        if response:
            ready, _, _ = select.select([client], [], [], 0.1)
            while ready:
                chunk = client.recv(4096)
                if not chunk:
                    break
                response.extend(chunk)
                ready, _, _ = select.select([client], [], [], 0.01)

    print(base64.b64encode(response).decode("ascii"))


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)
