FROM ubuntu:22.04 AS builder
LABEL authors="kyler"

RUN apt-get update && apt-get install -y  \
    cmake \
    g++ \
    make \
    git \
    libssl-dev \
    curl \
    libasio-dev \
    nlohmann-json3-dev \
    libmosquitto-dev \
    mosquitto-clients \
    pkg-config  \
    libboost-all-dev && \
    rm -rf /var/lib/apt/lists/*

RUN git clone --depth=1 https://github.com/CrowCpp/Crow.git /tmp/crow && \
    mkdir -p /usr/local/include && \
    cp -r /tmp/crow/include/* /usr/local/include/ && \
    rm -rf /tmp/crow

WORKDIR /app
COPY . /app

RUN mkdir -p external/gtest && \
    git clone --depth=1 https://github.com/google/googletest.git external/gtest

RUN rm -rf build && \
    mkdir build && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --target Weather_Station_Dashboard -- -j$(nproc)


FROM ubuntu:22.04 AS app

RUN apt-get update && apt-get install -y \
    git \
    cmake \
    build-essential \
    libssl-dev \
    libasio-dev \
    nlohmann-json3-dev && \
    rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY --from=builder /app/build/Weather_Station_Dashboard .
COPY --from=builder /app/src/templates ./templates

EXPOSE 18080

CMD ["./Weather_Station_Dashboard"]


# --- tests ---
FROM builder AS tests
WORKDIR /app

# Rebuild with test flag enabled
RUN rm -rf build && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON && \
    cmake --build build --target runTests -- -j$(nproc)

CMD ["./build/runTests"]
