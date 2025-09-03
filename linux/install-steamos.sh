#!/bin/bash

# SudoVDA SteamOS Installation Script
# Automatically detects SteamOS version and installs SudoVDA

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}SudoVDA SteamOS Installation${NC}"
echo "=============================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This script must be run as root${NC}"
    echo "Please run: sudo $0"
    exit 1
fi

# Detect SteamOS version
echo -e "${YELLOW}Detecting SteamOS version...${NC}"
if [ -f "/etc/os-release" ]; then
    if grep -q "SteamOS" /etc/os-release; then
        STEAMOS_VERSION=$(grep "VERSION_ID" /etc/os-release | cut -d'"' -f2)
        echo -e "${GREEN}Detected SteamOS $STEAMOS_VERSION${NC}"
        
        if [[ "$STEAMOS_VERSION" == "3"* ]]; then
            STEAMOS_3=true
            echo -e "${BLUE}SteamOS 3.x (Holo) detected${NC}"
        else
            STEAMOS_3=false
            echo -e "${BLUE}SteamOS 2.x (Alchemist) detected${NC}"
        fi
    else
        echo -e "${YELLOW}Not running SteamOS, but continuing with installation...${NC}"
        STEAMOS_3=false
    fi
else
    echo -e "${YELLOW}Could not detect OS version, assuming SteamOS 2.x${NC}"
    STEAMOS_3=false
fi

# Handle SteamOS 3.x read-only filesystem
if [ "$STEAMOS_3" = true ]; then
    echo -e "${YELLOW}SteamOS 3.x detected - disabling read-only filesystem...${NC}"
    if steamos-readonly status | grep -q "enabled"; then
        echo "Disabling read-only filesystem..."
        steamos-readonly disable
        READONLY_DISABLED=true
    else
        echo -e "${GREEN}Read-only filesystem already disabled${NC}"
        READONLY_DISABLED=false
    fi
fi

# Update package database
echo -e "${YELLOW}Updating package database...${NC}"
pacman -Sy

# Install dependencies
echo -e "${YELLOW}Installing build dependencies...${NC}"
pacman -S --noconfirm base-devel linux-headers dkms

echo -e "${GREEN}🎉 SudoVDA installation completed successfully!${NC}"
echo ""
echo -e "${BLUE}Next steps:${NC}"
echo "1. Build the driver: ./build-package.sh"
echo "2. Install: sudo pacman -U sudovda-dkms-*.pkg.tar.zst"
echo "3. Create virtual displays for gaming"
echo "4. Configure Sunshine to use virtual displays"
echo ""
echo -e "${YELLOW}Note: This is a placeholder script. Full implementation coming soon!${NC}"
