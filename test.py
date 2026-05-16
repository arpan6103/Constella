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

# Write to node1
n1 = make_client(6000)
send(n1, "PUT user1 arpan")
print(f"PUT via node1: {recv(n1)}")      # OK

# Read from node2 — should still find it (replication)
n2 = make_client(6001)
send(n2, "GET user1")
print(f"GET via node2: {recv(n2)}")      # VALUE: arpan

# Read from node3
n3 = make_client(6002)
send(n3, "GET user1")
print(f"GET via node3: {recv(n3)}")      # VALUE: arpan or ERROR depending on hash ring

# Test cross-node write — write to node2, read from node1
send(n2, "PUT user2 testvalue")
print(f"PUT via node2: {recv(n2)}")      # OK

send(n1, "GET user2")
print(f"GET via node1: {recv(n1)}")      # VALUE: testvalue

n1.close()
n2.close()
n3.close()