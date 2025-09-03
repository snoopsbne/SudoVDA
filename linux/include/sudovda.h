#ifndef SUDOVDA_H
#define SUDOVDA_H

#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/drm.h>

#define SUDOVDA_DRIVER_NAME "sudovda"
#define SUDOVDA_DRIVER_DESC "SudoVDA Virtual Display Driver"
#define SUDOVDA_DRIVER_DATE "2024-01-01"
#define SUDOVDA_DRIVER_MAJOR 1
#define SUDOVDA_DRIVER_MINOR 0
#define SUDOVDA_DRIVER_PATCHLEVEL 0

#define SUDOVDA_MAX_DISPLAYS 10
#define SUDOVDA_MAX_EDID_SIZE 256

/* IOCTL definitions */
#define SUDOVDA_IOCTL_BASE 'S'
#define SUDOVDA_IOCTL_ADD_DISPLAY    _IOWR(SUDOVDA_IOCTL_BASE, 0x01, struct sudovda_add_display)
#define SUDOVDA_IOCTL_REMOVE_DISPLAY _IOW(SUDOVDA_IOCTL_BASE, 0x02, struct sudovda_remove_display)
#define SUDOVDA_IOCTL_LIST_DISPLAYS  _IOR(SUDOVDA_IOCTL_BASE, 0x03, struct sudovda_list_displays)
#define SUDOVDA_IOCTL_GET_VERSION    _IOR(SUDOVDA_IOCTL_BASE, 0x04, struct sudovda_version)
#define SUDOVDA_IOCTL_PING           _IO(SUDOVDA_IOCTL_BASE, 0x05)

/* Display modes */
struct sudovda_mode {
	__u32 width;
	__u32 height;
	__u32 refresh_rate; /* in mHz (millihertz) */
	__u32 flags;
};

/* EDID structure */
struct sudovda_edid {
	__u8 data[SUDOVDA_MAX_EDID_SIZE];
	__u32 size;
};

/* Display information */
struct sudovda_display_info {
	__u32 id;
	__u32 connector_id;
	char name[32];
	char serial[32];
	struct sudovda_mode preferred_mode;
	struct sudovda_edid edid;
	__u32 status; /* 0 = disconnected, 1 = connected */
};

/* IOCTL structures */
struct sudovda_add_display {
	__u32 width;
	__u32 height;
	__u32 refresh_rate;
	char name[32];
	char serial[32];
	__u32 display_id; /* output */
	__u32 connector_id; /* output */
};

struct sudovda_remove_display {
	__u32 display_id;
};

struct sudovda_list_displays {
	__u32 count;
	struct sudovda_display_info displays[SUDOVDA_MAX_DISPLAYS];
};

struct sudovda_version {
	__u32 major;
	__u32 minor;
	__u32 patch;
	char build_date[32];
};

/* Driver internal structures */
struct sudovda_display {
	struct list_head list;
	struct drm_connector connector;
	struct drm_encoder encoder;
	struct drm_crtc crtc;
	struct drm_plane primary_plane;
	
	__u32 id;
	__u32 connector_id;
	char name[32];
	char serial[32];
	struct sudovda_mode preferred_mode;
	struct sudovda_edid edid;
	
	bool connected;
	bool enabled;
	
	struct drm_framebuffer *fb;
	struct drm_display_mode *mode;
	
	struct mutex lock;
};

struct sudovda_device {
	struct drm_device *drm_dev;
	struct pci_dev *pdev;
	
	struct list_head displays;
	struct mutex displays_lock;
	
	__u32 next_display_id;
	__u32 next_connector_id;
	
	bool initialized;
};

#endif /* SUDOVDA_H */