import socket, struct

def send(sock, msg):
    encoded = msg.encode()
    sock.sendall(struct.pack('>I', len(encoded)) + encoded)

def recv(sock):
    raw = sock.recv(4)
    if len(raw) < 4: return ""
    length = struct.unpack('>I', raw)[0]
    data = b""
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk: break
        data += chunk
    return data.decode()

s = socket.socket()
s.connect(('localhost', 6000))
send(s, "GET persistent_key")
print(recv(s))  # OK
s.close()