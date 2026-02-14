cmake_minimum_required(VERSION 3.10)
project(constella)

set(CMAKE_CXX_STANDARD 17)

add_executable(constella-node
    src/main.cpp
    src/storage.cpp
    src/server.cpp
    src/hash_ring.cpp
)