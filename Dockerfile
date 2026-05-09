FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make

WORKDIR /app

COPY . .

RUN rm -rf build

RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

WORKDIR /app/build

CMD ["./constella-node"]