#!/bin/bash

# SudoVDA Build Test Script
# Tests compilation without requiring a full Linux environment

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}SudoVDA Build Test${NC}"
echo "=================="

# Check if we're on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo -e "${YELLOW}Running on macOS - testing syntax only${NC}"
    
    # Test C syntax with clang
    echo -e "${YELLOW}Testing C syntax with clang...${NC}"
    
    # Test main driver file
    if clang -fsyntax-only -I./include sudovda_drv.c 2>/dev/null; then
        echo -e "${GREEN}✓ sudovda_drv.c syntax OK${NC}"
    else
        echo -e "${RED}✗ sudovda_drv.c has syntax errors${NC}"
        clang -fsyntax-only -I./include sudovda_drv.c
    fi
    
    # Test simple driver file
    if clang -fsyntax-only -I./include sudovda_drv_simple.c 2>/dev/null; then
        echo -e "${GREEN}✓ sudovda_drv_simple.c syntax OK${NC}"
    else
        echo -e "${RED}✗ sudovda_drv_simple.c has syntax errors${NC}"
        clang -fsyntax-only -I./include sudovda_drv_simple.c
    fi
    
    # Test display file
    if clang -fsyntax-only -I./include sudovda_display.c 2>/dev/null; then
        echo -e "${GREEN}✓ sudovda_display.c syntax OK${NC}"
    else
        echo -e "${RED}✗ sudovda_display.c has syntax errors${NC}"
        clang -fsyntax-only -I./include sudovda_display.c
    fi
    
    # Test EDID file
    if clang -fsyntax-only -I./include sudovda_edid.c 2>/dev/null; then
        echo -e "${GREEN}✓ sudovda_edid.c syntax OK${NC}"
    else
        echo -e "${RED}✗ sudovda_edid.c has syntax errors${NC}"
        clang -fsyntax-only -I./include sudovda_edid.c
    fi
    
    # Test IOCTL file
    if clang -fsyntax-only -I./include sudovda_ioctl.c 2>/dev/null; then
        echo -e "${GREEN}✓ sudovda_ioctl.c syntax OK${NC}"
    else
        echo -e "${RED}✗ sudovda_ioctl.c has syntax errors${NC}"
        clang -fsyntax-only -I./include sudovda_ioctl.c
    fi
    
    # Test user-space tool
    if gcc -fsyntax-only -std=c99 tools/sudovda-ctl.c 2>/dev/null; then
        echo -e "${GREEN}✓ sudovda-ctl.c syntax OK${NC}"
    else
        echo -e "${RED}✗ sudovda-ctl.c has syntax errors${NC}"
        gcc -fsyntax-only -std=c99 tools/sudovda-ctl.c
    fi
    
    echo ""
    echo -e "${BLUE}Build Test Summary${NC}"
    echo "=================="
    echo -e "${GREEN}Syntax tests completed!${NC}"
    echo ""
    echo -e "${YELLOW}Note: This only tests C syntax, not full compilation.${NC}"
    echo -e "${YELLOW}For full testing, use Docker with Linux kernel headers.${NC}"
    
else
    echo -e "${YELLOW}Running on Linux - attempting full build${NC}"
    
    # Try to build with make
    if make -f Makefile.simple all; then
        echo -e "${GREEN}✓ Simple driver build successful${NC}"
    else
        echo -e "${RED}✗ Simple driver build failed${NC}"
    fi
fi

echo ""
echo -e "${BLUE}Next Steps:${NC}"
echo "1. Fix any syntax errors shown above"
echo "2. Test with Docker: ./docker-build.sh"
echo "3. Test on actual Linux system with kernel headers"
