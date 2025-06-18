# 第一阶段：构建环境
FROM registry.cn-hangzhou.aliyuncs.com/acs/ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# 安装明确版本的 zlib 和构建依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    autoconf \
    libtool \
    pkg-config \
    zlib1g-dev \
    libsystemd-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
COPY grpc_lib /usr/local/grpc
COPY lib /usr/local/lib/

# 设置环境变量
ENV LD_LIBRARY_PATH=/usr/local/grpc/lib
ENV PATH=/usr/local/grpc/bin:$PATH
ENV PKG_CONFIG_PATH=/usr/local/grpc/lib/pkgconfig

# 构建项目
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_CXX_STANDARD=17 .. && \
    make -j16

# 第二阶段：运行时环境
FROM registry.cn-hangzhou.aliyuncs.com/acs/ubuntu:22.04

# 安装运行时依赖（明确版本）
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/MonitorWallGRPCServer .
COPY --from=builder /usr/local/grpc/lib/ /usr/local/lib/
COPY --from=builder /usr/local/lib/ /usr/local/lib/

# 设置运行时环境
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
EXPOSE 50051
CMD ["./MonitorWallGRPCServer"]