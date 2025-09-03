/*
 * SudoVDA EDID Generation and Management
 * Based on the Windows version but adapted for Linux
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/crc32.h>

#include "include/sudovda.h"
#include "include/sudovda_edid.h"

/* Base EDID template */
static const __u8 edid_base[] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x4d, 0xab, 0xce, 0xd1, 0xef, 0x2d, 0xbc, 0x1a,
	0x20, 0x22, 0x01, 0x03, 0x80, 0x46, 0x27, 0x78, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xa5, 0x6b, 0x80, 0xd1, 0xc0, 0xb3, 0x00, 0xa9, 0xc0, 0x81, 0x80, 0x81, 0x00,
	0x81, 0xc0, 0x01, 0x01, 0x01, 0x01, 0x4d, 0xd0, 0x00, 0xa0, 0xf0, 0x70, 0x3e, 0x80, 0x30, 0x20,
	0x35, 0x00, 0xba, 0x89, 0x21, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0xff, 0x00, 0x31, 0x32, 0x33,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x0f,
	0xff, 0x14, 0xff, 0xff, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
	0x00, 0x53, 0x75, 0x64, 0x6f, 0x4d, 0x61, 0x6b, 0x65, 0x72, 0x56, 0x44, 0x44, 0x0a, 0x01, 0xa4,
	0x02, 0x03, 0x44, 0xf0, 0x51, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x10, 0x1f, 0x22, 0x21, 0x20, 0x05,
	0x14, 0x04, 0x13, 0x12, 0x03, 0x01, 0x23, 0x0f, 0x56, 0x05, 0x83, 0x0f, 0x08, 0x00, 0x6d, 0x03,
	0x0c, 0x00, 0x10, 0x00, 0x38, 0x78, 0x20, 0x00, 0x60, 0x01, 0x02, 0x03, 0x67, 0xd8, 0x5d, 0xc4,
	0x01, 0x78, 0x80, 0x03, 0xe3, 0x05, 0xe0, 0x01, 0xe4, 0x0f, 0x18, 0x00, 0x00, 0xe6, 0x06, 0x0f,
	0x01, 0xc8, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e,
};

/* Helper function to calculate EDID checksum */
static __u8 edid_checksum(const __u8 *edid)
{
	__u8 sum = 0;
	int i;

	for (i = 0; i < EDID_SIZE - 1; i++)
		sum += edid[i];

	return (256 - sum) & 0xff;
}

/* Generate detailed timing descriptor */
static void edid_generate_detailed_timing(__u8 *desc, __u32 width, __u32 height, __u32 refresh_rate)
{
	__u32 pixel_clock = (width * height * refresh_rate) / 1000000; /* Convert to 10kHz */
	__u32 h_total = width + (width * 20) / 100; /* Add 20% blanking */
	__u32 v_total = height + (height * 5) / 100; /* Add 5% blanking */
	__u32 h_sync_start = width + (width * 8) / 100;
	__u32 h_sync_end = h_sync_start + (width * 3) / 100;
	__u32 v_sync_start = height + (height * 2) / 100;
	__u32 v_sync_end = v_sync_start + 5;

	/* Pixel clock (10kHz units) */
	desc[0] = pixel_clock & 0xff;
	desc[1] = (pixel_clock >> 8) & 0xff;

	/* Horizontal active */
	desc[2] = width & 0xff;
	desc[3] = (width >> 8) & 0x0f;
	desc[3] |= ((h_total - width) & 0x0f) << 4;

	/* Horizontal blanking */
	desc[4] = (h_total - width) & 0xff;

	/* Horizontal sync offset and width */
	desc[5] = (h_sync_start - width) & 0xff;
	desc[6] = ((h_sync_start - width) >> 8) & 0x0f;
	desc[6] |= ((h_sync_end - h_sync_start) & 0x0f) << 4;

	/* Horizontal sync width */
	desc[7] = (h_sync_end - h_sync_start) & 0xff;

	/* Vertical active */
	desc[8] = height & 0xff;
	desc[9] = (height >> 8) & 0x0f;
	desc[9] |= ((v_total - height) & 0x0f) << 4;

	/* Vertical blanking */
	desc[10] = (v_total - height) & 0xff;

	/* Vertical sync offset and width */
	desc[11] = (v_sync_start - height) & 0xff;
	desc[12] = ((v_sync_start - height) >> 8) & 0x0f;
	desc[12] |= ((v_sync_end - v_sync_start) & 0x0f) << 4;

	/* Vertical sync width */
	desc[13] = (v_sync_end - v_sync_start) & 0xff;

	/* Horizontal and vertical image size (mm) */
	desc[14] = (width * 254) / 10000; /* Convert to mm, assuming ~100 DPI */
	desc[15] = ((width * 254) / 10000) >> 8;
	desc[16] = (height * 254) / 10000;
	desc[17] = ((height * 254) / 10000) >> 8;

	/* Border pixels */
	desc[18] = 0;
	desc[19] = 0;

	/* Features */
	desc[20] = 0x1e; /* Digital, separate sync, positive polarity */
}

/* Generate monitor name descriptor */
static void edid_generate_monitor_name(__u8 *desc, const char *name)
{
	int len = strlen(name);
	int i;

	desc[0] = 0x00; /* Detailed timing descriptor */
	desc[1] = 0x00;
	desc[2] = 0x00;
	desc[3] = EDID_DESCRIPTOR_MONITOR_NAME;

	for (i = 0; i < 13 && i < len; i++)
		desc[4 + i] = name[i];

	/* Pad with spaces */
	for (; i < 13; i++)
		desc[4 + i] = ' ';

	desc[17] = 0x0a; /* Line feed */
}

/* Generate serial number descriptor */
static void edid_generate_serial(__u8 *desc, const char *serial)
{
	int len = strlen(serial);
	int i;

	desc[0] = 0x00; /* Detailed timing descriptor */
	desc[1] = 0x00;
	desc[2] = 0x00;
	desc[3] = EDID_DESCRIPTOR_MONITOR_SERIAL;

	for (i = 0; i < 13 && i < len; i++)
		desc[4 + i] = serial[i];

	/* Pad with spaces */
	for (; i < 13; i++)
		desc[4 + i] = ' ';

	desc[17] = 0x0a; /* Line feed */
}

/* Main EDID generation function */
int sudovda_edid_generate(struct sudovda_edid *edid, 
			  __u32 width, __u32 height, __u32 refresh_rate,
			  const char *name, const char *serial)
{
	__u8 *edid_data;
	int ret = 0;

	if (!edid || !name || !serial)
		return -EINVAL;

	edid_data = kmalloc(EDID_SIZE, GFP_KERNEL);
	if (!edid_data)
		return -ENOMEM;

	/* Copy base EDID */
	memcpy(edid_data, edid_base, EDID_SIZE);

	/* Set manufacturer ID (SudoMaker = 0x4DAB) */
	edid_data[EDID_OFFSET_MANUFACTURER] = 0xAB;
	edid_data[EDID_OFFSET_MANUFACTURER + 1] = 0x4D;

	/* Set product code */
	edid_data[EDID_OFFSET_PRODUCT_CODE] = 0xCE;
	edid_data[EDID_OFFSET_PRODUCT_CODE + 1] = 0xD1;

	/* Set serial number */
	edid_data[EDID_OFFSET_SERIAL] = 0xEF;
	edid_data[EDID_OFFSET_SERIAL + 1] = 0x2D;
	edid_data[EDID_OFFSET_SERIAL + 2] = 0xBC;
	edid_data[EDID_OFFSET_SERIAL + 3] = 0x1A;

	/* Set manufacture date (2024) */
	edid_data[EDID_OFFSET_MANUFACTURE_DATE] = 0x20; /* Week 32 */
	edid_data[EDID_OFFSET_MANUFACTURE_DATE + 1] = 0x24; /* Year 2024 */

	/* Generate detailed timing descriptor for preferred mode */
	edid_generate_detailed_timing(&edid_data[EDID_OFFSET_DESCRIPTORS], 
				      width, height, refresh_rate);

	/* Generate monitor name descriptor */
	edid_generate_monitor_name(&edid_data[EDID_OFFSET_DESCRIPTORS + 18], name);

	/* Generate serial number descriptor */
	edid_generate_serial(&edid_data[EDID_OFFSET_DESCRIPTORS + 36], serial);

	/* Calculate and set checksum */
	edid_data[EDID_OFFSET_CHECKSUM] = edid_checksum(edid_data);

	/* Copy to output structure */
	memcpy(edid->data, edid_data, EDID_SIZE);
	edid->size = EDID_SIZE;

	kfree(edid_data);
	return ret;
}

/* Validate EDID */
int sudovda_edid_validate(const struct sudovda_edid *edid)
{
	__u8 sum = 0;
	int i;

	if (!edid || edid->size < EDID_SIZE)
		return -EINVAL;

	/* Check header */
	if (memcmp(edid->data, "\x00\xff\xff\xff\xff\xff\xff\x00", 8) != 0)
		return -EINVAL;

	/* Check checksum */
	for (i = 0; i < EDID_SIZE; i++)
		sum += edid->data[i];

	if (sum != 0)
		return -EINVAL;

	return 0;
}

/* Print EDID information */
void sudovda_edid_print(const struct sudovda_edid *edid)
{
	if (!edid || edid->size < EDID_SIZE) {
		pr_err("SudoVDA: Invalid EDID\n");
		return;
	}

	pr_info("SudoVDA: EDID Information:\n");
	pr_info("  Manufacturer: %c%c\n", 
		edid->data[EDID_OFFSET_MANUFACTURER + 1],
		edid->data[EDID_OFFSET_MANUFACTURER]);
	pr_info("  Product Code: 0x%04x\n",
		edid->data[EDID_OFFSET_PRODUCT_CODE] | 
		(edid->data[EDID_OFFSET_PRODUCT_CODE + 1] << 8));
	pr_info("  Serial: 0x%08x\n",
		edid->data[EDID_OFFSET_SERIAL] |
		(edid->data[EDID_OFFSET_SERIAL + 1] << 8) |
		(edid->data[EDID_OFFSET_SERIAL + 2] << 16) |
		(edid->data[EDID_OFFSET_SERIAL + 3] << 24));
}
