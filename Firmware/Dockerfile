# Use the latest Ubuntu image as a base
FROM ubuntu:latest

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    PATH=$PATH:/root/.cargo/bin \
    CARGO_HOME=/root/.cargo \
    RUSTUP_HOME=/root/.rustup

# Install necessary packages
RUN apt-get update && apt-get install -y \
    clang \
    curl \
    git \
    libssl-dev \
    pkg-config \
    python3-pip \
    libudev-dev \
    usb-creator-gtk \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Rust and Cargo
RUN curl --proto '=https' --tlsv1.2 --fail --show-error --silent https://sh.rustup.rs | sh -s -- -y && \
    /bin/bash -c "source $HOME/.cargo/env" && \
    rustup toolchain install nightly --component rust-src

# Install additional Cargo modules
RUN cargo install ldproxy cargo-generate

# Install espflash separately
RUN cargo install espflash --version 2.0.0

# Install Python dependencies
RUN python3 -m pip install packaging

# Add VSCodium repository
RUN curl -fSsL https://gitlab.com/paulcarroty/vscodium-deb-rpm-repo/raw/master/pub.gpg | gpg --dearmor | tee /usr/share/keyrings/vscodium.gpg > /dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/vscodium.gpg] https://download.vscodium.com/debs vscodium main' | tee /etc/apt/sources.list.d/vscodium.list

# Update package list again and install VSCodium
RUN apt-get update && \
    apt-get install -y codium

# Set the working directory
WORKDIR /workspace
