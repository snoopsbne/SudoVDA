FROM --platform=linux/amd64 archlinux:latest

# Install build dependencies
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
    base-devel \
    linux-headers \
    git \
    cmake \
    make \
    gcc \
    pkg-config \
    libdrm \
    mesa \
    xorg-server-devel \
    libxrandr \
    libxinerama \
    libxcursor \
    libxi \
    libgl \
    vulkan-devel

# Create build user
RUN useradd -m -s /bin/bash builder && \
    echo "builder ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set working directory
WORKDIR /workspace

# Copy source code
COPY . /workspace/

# Change ownership
RUN chown -R builder:builder /workspace

# Switch to builder user
USER builder

# Build the driver
CMD ["make", "all"]
