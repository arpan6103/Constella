# Constella

Constella is a distributed quorum-based key-value store inspired by Dynamo-style distributed systems.

It supports:

- Consistent hashing
- Replication
- Quorum-based consistency
- Coordinator-based request routing
- Idempotent distributed writes
- Heartbeat-based failure detection
- Dynamic node membership
- Multi-threaded TCP networking
- Dockerized distributed deployment

---

# Architecture Overview

```text
                ┌───────────────┐
                │    Client     │
                └──────┬────────┘
                       │
                 TCP Request
                       │
              ┌────────▼────────┐
              │  Coordinator    │
              │      Node       │
              └────────┬────────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
   Replica 1      Replica 2      Replica 3

```

Each node in the cluster:

- Accepts client requests
- Acts as coordinator for incoming requests
- Replicates data to responsible nodes
- Participates in quorum reads/writes
- Monitors node liveness using heartbeat messages

---

# Features

## 1. Consistent Hashing

Constella uses a consistent hashing ring to distribute keys across nodes.

Benefits:

- Deterministic key placement
- Minimal key movement during node changes
- Scalable partitioning

Replica nodes are selected by walking clockwise on the hash ring.

---

## 2. Replication

Each key is replicated across multiple nodes using a configurable replication factor.

Example:

```text
Replication Factor = 2
```

A key is stored on 2 different nodes.

---

## 3. Quorum Consistency

Constella implements Dynamo-style quorum logic.

Parameters:

```text
N = Replication Factor
W = Write Quorum
R = Read Quorum
```

Consistency rule:

```text
W + R > N
```

This ensures strong consistency guarantees under normal operation.

---

## 4. Coordinator-Based Routing

Any node can receive a client request.

That node becomes the coordinator and:

1. Computes responsible replicas
2. Forwards requests to replicas
3. Collects acknowledgements
4. Returns final response to client

---

## 5. Idempotent Distributed Writes

Constella uses request IDs to prevent:

- Infinite replication loops
- Duplicate writes
- Retry amplification

Each request is tagged with a unique request ID.

Processed requests are tracked to ensure idempotent behavior.

---

## 6. Heartbeat-Based Failure Detection

Nodes periodically exchange:

```text
PING
PONG
```

messages.

If a node becomes unreachable:

- It is marked dead
- Removed from active routing
- Excluded from replica selection

This enables fault-tolerant operation.

---

## 7. Multi-threaded Networking

Constella uses POSIX TCP sockets and std::thread to handle:

- Concurrent clients
- Inter-node communication
- Background heartbeat monitoring

---

## 8. Dockerized Deployment

The cluster runs as multiple Docker containers using Docker Compose.

Docker networking enables internal service discovery between nodes.

---

# Tech Stack

## Languages

- C++17

## Networking

- POSIX Sockets
- TCP/IP

## Concurrency

- std::thread
- std::mutex
- std::atomic

## Infrastructure

- Docker
- Docker Compose
- CMake

---

# Project Structure

```text
constella/
│
├── src/
│   ├── main.cpp
│   ├── server.cpp
│   ├── storage.cpp
│   └── hash_ring.cpp
│
├── include/
│   ├── server.h
│   ├── storage.h
│   └── hash_ring.h
│
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
└── README.md
```

---

# Build Instructions

## Local Build

```bash
mkdir build
cd build
cmake ..
make
```

---

# Running Locally

Run nodes in separate terminals:

## Node 1

```bash
./constella-node 6000 127.0.0.1:6000
```

## Node 2

```bash
./constella-node 6001 127.0.0.1:6001
```

## Node 3

```bash
./constella-node 6002 127.0.0.1:6002
```

---

# Docker Deployment

Start the distributed cluster:

```bash
docker compose up --build
```

This launches:

- node1
- node2
- node3

inside isolated Docker containers.

---

# Client Usage

Connect using netcat:

```bash
nc localhost 6000
```

## PUT

```text
PUT user1 arpan
```

Response:

```text
OK
```

## GET

```text
GET user1
```

Response:

```text
VALUE: arpan
```

---

# Failure Testing

Stop a node:

```bash
docker stop constella-node2
```

Heartbeat detection automatically removes the node from active routing.

Reads and writes continue using remaining replicas.

---

# Future Improvements

Potential future enhancements:

- Persistent storage (WAL)
- Read repair
- Virtual nodes
- Gossip protocol
- Data rebalancing
- Conflict resolution

---

# Distributed Systems Concepts Demonstrated

- Consistent Hashing
- Replication
- Quorum Consensus
- Eventual Consistency
- Fault Tolerance
- Failure Detection
- Idempotent APIs
- Distributed Request Routing
- Service Discovery
- Concurrent Networking

---

# Author

Arpan Chauhan

