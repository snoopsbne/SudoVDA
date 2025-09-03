#ifndef SUDOVDA_EDID_H
#define SUDOVDA_EDID_H

#include <linux/types.h>

#define EDID_SIZE 128
#define EDID_EXTENSION_SIZE 128
#define EDID_MAX_SIZE (EDID_SIZE + 127 * EDID_EXTENSION_SIZE)

/* EDID offsets */
#define EDID_OFFSET_HEADER 0x00
#define EDID_OFFSET_MANUFACTURER 0x08
#define EDID_OFFSET_PRODUCT_CODE 0x0A
#define EDID_OFFSET_SERIAL 0x0C
#define EDID_OFFSET_MANUFACTURE_DATE 0x10
#define EDID_OFFSET_EDID_VERSION 0x12
#define EDID_OFFSET_BASIC_DISPLAY 0x14
#define EDID_OFFSET_COLOR_CHARACTERISTICS 0x15
#define EDID_OFFSET_ESTABLISHED_TIMINGS 0x16
#define EDID_OFFSET_STANDARD_TIMINGS 0x26
#define EDID_OFFSET_DESCRIPTORS 0x36
#define EDID_OFFSET_EXTENSIONS 0x7E
#define EDID_OFFSET_CHECKSUM 0x7F

/* Descriptor types */
#define EDID_DESCRIPTOR_DETAILED_TIMING 0x00
#define EDID_DESCRIPTOR_MONITOR_NAME 0xFC
#define EDID_DESCRIPTOR_MONITOR_LIMITS 0xFD
#define EDID_DESCRIPTOR_MONITOR_SERIAL 0xFF
#define EDID_DESCRIPTOR_ADDITIONAL_COLOR_POINT 0xF9
#define EDID_DESCRIPTOR_ADDITIONAL_STANDARD_TIMING 0xFA
#define EDID_DESCRIPTOR_ADDITIONAL_WHITE_POINT 0xFB
#define EDID_DESCRIPTOR_DUMMY 0x10

/* Basic display parameters */
#define EDID_BASIC_DISPLAY_ANALOG 0x00
#define EDID_BASIC_DISPLAY_DIGITAL 0x01

/* Color characteristics */
#define EDID_COLOR_RED_X_LOW 0x00
#define EDID_COLOR_RED_X_HIGH 0x01
#define EDID_COLOR_RED_Y_LOW 0x02
#define EDID_COLOR_RED_Y_HIGH 0x03
#define EDID_COLOR_GREEN_X_LOW 0x04
#define EDID_COLOR_GREEN_X_HIGH 0x05
#define EDID_COLOR_GREEN_Y_LOW 0x06
#define EDID_COLOR_GREEN_Y_HIGH 0x07
#define EDID_COLOR_BLUE_X_LOW 0x08
#define EDID_COLOR_BLUE_X_HIGH 0x09
#define EDID_COLOR_BLUE_Y_LOW 0x0A
#define EDID_COLOR_BLUE_Y_HIGH 0x0B
#define EDID_COLOR_WHITE_X_LOW 0x0C
#define EDID_COLOR_WHITE_X_HIGH 0x0D
#define EDID_COLOR_WHITE_Y_LOW 0x0E
#define EDID_COLOR_WHITE_Y_HIGH 0x0F

/* Established timings */
#define EDID_ESTABLISHED_TIMING_720x400_70 0x01
#define EDID_ESTABLISHED_TIMING_720x400_88 0x02
#define EDID_ESTABLISHED_TIMING_640x480_60 0x04
#define EDID_ESTABLISHED_TIMING_640x480_67 0x08
#define EDID_ESTABLISHED_TIMING_640x480_72 0x10
#define EDID_ESTABLISHED_TIMING_640x480_75 0x20
#define EDID_ESTABLISHED_TIMING_800x600_56 0x40
#define EDID_ESTABLISHED_TIMING_800x600_60 0x80

/* Standard timing structure */
struct edid_standard_timing {
	__u8 h_resolution;
	__u8 v_resolution;
	__u8 refresh_rate;
};

/* Detailed timing structure */
struct edid_detailed_timing {
	__u16 pixel_clock; /* in 10kHz units */
	__u8 h_active_low;
	__u8 h_active_high;
	__u8 h_blanking_low;
	__u8 h_blanking_high;
	__u8 h_sync_offset_low;
	__u8 h_sync_offset_high;
	__u8 h_sync_width_low;
	__u8 h_sync_width_high;
	__u8 v_active_low;
	__u8 v_active_high;
	__u8 v_blanking_low;
	__u8 v_blanking_high;
	__u8 v_sync_offset_low;
	__u8 v_sync_offset_high;
	__u8 v_sync_width_low;
	__u8 v_sync_width_high;
	__u8 h_size_low;
	__u8 h_size_high;
	__u8 v_size_low;
	__u8 v_size_high;
	__u8 h_border;
	__u8 v_border;
	__u8 features;
};

/* EDID descriptor */
struct edid_descriptor {
	__u8 type;
	__u8 data[18];
};

/* Main EDID structure */
struct edid {
	__u8 header[8];
	__u8 manufacturer[2];
	__u8 product_code[2];
	__u8 serial[4];
	__u8 manufacture_date[2];
	__u8 edid_version[2];
	__u8 basic_display;
	__u8 color_characteristics[10];
	__u8 established_timings[3];
	__u8 standard_timings[16];
	struct edid_descriptor descriptors[4];
	__u8 extensions;
	__u8 checksum;
};

/* Function prototypes */
int sudovda_edid_generate(struct sudovda_edid *edid, 
			  __u32 width, __u32 height, __u32 refresh_rate,
			  const char *name, const char *serial);
int sudovda_edid_validate(const struct sudovda_edid *edid);
void sudovda_edid_print(const struct sudovda_edid *edid);

#endif /* SUDOVDA_EDID_H */