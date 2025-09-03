# SudoVDA Linux Kernel Driver Makefile
# Virtual Display Driver for SteamOS/Arch Linux

obj-m += sudovda.o

# Source files
sudovda-y := \
	sudovda_drv.o \
	sudovda_display.o \
	sudovda_edid.o \
	sudovda_ioctl.o

# Kernel build directory
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

# Build directory
PWD := $(shell pwd)

# Build flags
ccflags-y += -I$(src)/include
ccflags-y += -DDEBUG

# Default target
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

# Clean target
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

# Install target
install: all
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a

# Uninstall target
uninstall:
	rm -f /lib/modules/$(shell uname -r)/extra/sudovda.ko
	depmod -a

# Load module
load: all
	sudo modprobe drm
	sudo insmod sudovda.ko

# Unload module
unload:
	sudo rmmod sudovda

# Reload module
reload: unload load

.PHONY: all clean install uninstall load unload reload