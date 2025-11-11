FROM ubuntu:22.04 AS builder
LABEL authors="kyler"

RUN apt-get update && apt-get install -y cmake g++ make git libssl-dev curl \
    libasio-dev

WORKDIR /app

COPY . /app

RUN mkdir -p external && \
    git clone --depth=1 https://github.com/CrowCpp/Crow.git external/Crow

RUN mkdir -p external/gtest && \
    git clone --depth=1 https://github.com/google/googletest.git external/gtest

RUN rm -rf build && \
    mkdir build && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target Weather_Station_Dashboard -- -j$(nproc)


FROM ubuntu:22.04 AS app

RUN apt-get update && apt-get install -y \
    git cmake build-essential libssl-dev && \
    rm -rf /var/lib/apt/lists/*

# 1. Build and install Paho MQTT C (with async and CMake package export)
RUN git clone https://github.com/eclipse/paho.mqtt.c.git /tmp/paho-c && \
    cd /tmp/paho-c && \
    cmake -B build -S . \
      -DPAHO_WITH_SSL=OFF \
      -DPAHO_BUILD_STATIC=OFF \
      -DPAHO_BUILD_SHARED=ON \
      -DPAHO_BUILD_DOCUMENTATION=OFF \
      -DPAHO_BUILD_SAMPLES=OFF \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DPAHO_ENABLE_TESTING=OFF \
      -DPAHO_BUILD_DEPRECATED=OFF \
      -DPAHO_WITH_CMAKE=ON && \
    cmake --build build --target install && \
    ldconfig

# 2. Build and install Paho MQTT C++
RUN git clone https://github.com/eclipse/paho.mqtt.cpp.git /tmp/paho-cpp && \
    cd /tmp/paho-cpp && \
    cmake -B build -S . \
      -DPAHO_BUILD_STATIC=OFF \
      -DPAHO_BUILD_DOCUMENTATION=OFF \
      -DPAHO_WITH_SSL=OFF \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --build build --target install && \
    ldconfig


WORKDIR /app

COPY --from=builder /app/build/Weather_Station_Dashboard .
COPY --from=builder /app/src/templates ./templates

EXPOSE 18080

CMD ["./Weather_Station_Dashboard"]


# --- tests ---
FROM builder AS tests
WORKDIR /app

# Copy templates for Crow tests
COPY src/templates ./src/templates

# Rebuild with test flag enabled
RUN rm -rf build && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON && \
    cmake --build build --target runTests -- -j$(nproc)

CMD ["./build/runTests"]
