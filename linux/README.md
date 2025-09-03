# SudoVDA Linux Driver

A Linux kernel driver that creates virtual displays for use with streaming applications like Sunshine. This is the Linux port of the Windows SudoVDA virtual display driver.

## Features

- **Multiple Virtual Displays**: Create up to 10 virtual displays simultaneously
- **High Resolution Support**: From 640x480 to 8K (7680x4320)
- **High Refresh Rates**: Support for 60Hz, 120Hz, 144Hz, 240Hz, and up to 500Hz
- **HDR Support**: Full HDR10 and HDR12 support
- **EDID Generation**: Automatic EDID generation for each virtual display
- **DRM Integration**: Full DRM (Direct Rendering Manager) integration
- **User-space Tools**: Command-line tools for managing virtual displays

## Quick Installation

### SteamOS (Recommended)
```bash
cd linux
sudo ./install-steamos.sh
```

### Manual Installation
```bash
cd linux
./build-package.sh
sudo pacman -U sudovda-dkms-*.pkg.tar.zst
```

## Usage

```bash
# Add a virtual display
sudo sudovda-ctl add 1920 1080 60000 "Virtual Display" "VD001"

# List displays
sudo sudovda-ctl list

# Remove display
sudo sudovda-ctl remove 0
```

## Perfect for Sunshine Integration

Create virtual displays that Sunshine can capture and stream:

```bash
# Gaming displays
sudo sudovda-ctl add 1920 1080 240000 "Competitive Gaming" "COMP001"
sudo sudovda-ctl add 3840 2160 120000 "4K Gaming" "4K001"
```

For complete documentation, see the main README.md file.
