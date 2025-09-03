#!/bin/bash

# SudoVDA Linux Driver Docker Build Script
# For building on M1 Mac with x86_64 Arch Linux

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}SudoVDA Linux Driver Docker Build${NC}"
echo "Building for x86_64 Arch Linux on M1 Mac..."

# Build the Docker image
echo -e "${YELLOW}Building Docker image...${NC}"
docker build --platform linux/amd64 -t sudovda-builder .

# Run the build
echo -e "${YELLOW}Building driver...${NC}"
docker run --platform linux/amd64 --rm -v "$(pwd):/workspace" sudovda-builder

echo -e "${GREEN}Build completed!${NC}"
echo "Driver files should be in the current directory."
