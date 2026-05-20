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

def make_client(port):
    s = socket.socket()
    s.connect(('localhost', port))
    return s

s = make_client(6000)

# PUT first
send(s, "PUT key0 valuekey0")
print(f"PUT key0: {recv(s)}")

send(s, "PUT key10 valuekey10")
print(f"PUT key10: {recv(s)}")

# Then GET
send(s, "GET key0")
print(f"GET key0: {recv(s)}")

send(s, "GET key10")
print(f"GET key10: {recv(s)}")

s.close()