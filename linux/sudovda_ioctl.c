/*
 * SudoVDA IOCTL Interface
 * User-space communication interface for managing virtual displays
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/drm/drm.h>
#include <linux/drm/drm_ioctl.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#include "include/sudovda.h"

/* IOCTL definitions */
#define DRM_IOCTL_SUDOVDA_ADD_DISPLAY DRM_IOWR(DRM_COMMAND_BASE + 0x01, struct sudovda_add_display)
#define DRM_IOCTL_SUDOVDA_REMOVE_DISPLAY DRM_IOW(DRM_COMMAND_BASE + 0x02, struct sudovda_remove_display)
#define DRM_IOCTL_SUDOVDA_LIST_DISPLAYS DRM_IOR(DRM_COMMAND_BASE + 0x03, struct sudovda_list_displays)
#define DRM_IOCTL_SUDOVDA_GET_VERSION DRM_IOR(DRM_COMMAND_BASE + 0x04, struct sudovda_version)
#define DRM_IOCTL_SUDOVDA_PING DRM_IO(DRM_COMMAND_BASE + 0x05)

/* IOCTL function prototypes */
static int sudovda_ioctl_add_display(struct drm_device *dev, void *data, struct drm_file *file);
static int sudovda_ioctl_remove_display(struct drm_device *dev, void *data, struct drm_file *file);
static int sudovda_ioctl_list_displays(struct drm_device *dev, void *data, struct drm_file *file);
static int sudovda_ioctl_get_version(struct drm_device *dev, void *data, struct drm_file *file);
static int sudovda_ioctl_ping(struct drm_device *dev, void *data, struct drm_file *file);

/* IOCTL table */
static const struct drm_ioctl_desc sudovda_ioctls[] = {
	DRM_IOCTL_DEF_DRV(SUDOVDA_ADD_DISPLAY, sudovda_ioctl_add_display, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(SUDOVDA_REMOVE_DISPLAY, sudovda_ioctl_remove_display, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(SUDOVDA_LIST_DISPLAYS, sudovda_ioctl_list_displays, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(SUDOVDA_GET_VERSION, sudovda_ioctl_get_version, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(SUDOVDA_PING, sudovda_ioctl_ping, DRM_AUTH),
};

/* Global device structure */
static struct sudovda_device *g_sudovda_dev = NULL;
static DEFINE_MUTEX(g_sudovda_mutex);

/* Forward declarations */
extern int sudovda_display_create(struct drm_device *dev, struct sudovda_display **display_out,
				  __u32 width, __u32 height, __u32 refresh_rate,
				  const char *name, const char *serial);
extern void sudovda_display_destroy(struct sudovda_display *display);

/* Add display IOCTL */
static int sudovda_ioctl_add_display(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct sudovda_add_display *args = data;
	struct sudovda_display *display;
	int ret;

	pr_info("SudoVDA: Adding display: %dx%d@%dHz, name=%s, serial=%s\n",
		args->width, args->height, args->refresh_rate, args->name, args->serial);

	/* Validate parameters */
	if (args->width < 640 || args->width > 7680 ||
	    args->height < 480 || args->height > 4320 ||
	    args->refresh_rate < 30 || args->refresh_rate > 500000) {
		pr_err("SudoVDA: Invalid display parameters\n");
		return -EINVAL;
	}

	mutex_lock(&g_sudovda_mutex);

	if (!g_sudovda_dev) {
		pr_err("SudoVDA: Device not initialized\n");
		ret = -ENODEV;
		goto out;
	}

	/* Check if we've reached the maximum number of displays */
	if (g_sudovda_dev->next_display_id >= SUDOVDA_MAX_DISPLAYS) {
		pr_err("SudoVDA: Maximum number of displays reached\n");
		ret = -ENOSPC;
		goto out;
	}

	/* Create the display */
	ret = sudovda_display_create(dev, &display, args->width, args->height,
				     args->refresh_rate, args->name, args->serial);
	if (ret) {
		pr_err("SudoVDA: Failed to create display: %d\n", ret);
		goto out;
	}

	/* Assign IDs */
	display->id = g_sudovda_dev->next_display_id++;
	display->connector_id = g_sudovda_dev->next_connector_id++;

	/* Add to list */
	list_add_tail(&display->list, &g_sudovda_dev->displays);

	/* Return display information */
	args->display_id = display->id;
	args->connector_id = display->connector_id;

	pr_info("SudoVDA: Display created with ID %d, connector ID %d\n",
		display->id, display->connector_id);

out:
	mutex_unlock(&g_sudovda_mutex);
	return ret;
}

/* Remove display IOCTL */
static int sudovda_ioctl_remove_display(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct sudovda_remove_display *args = data;
	struct sudovda_display *display, *tmp;
	int ret = -ENOENT;

	pr_info("SudoVDA: Removing display ID %d\n", args->display_id);

	mutex_lock(&g_sudovda_mutex);

	if (!g_sudovda_dev) {
		pr_err("SudoVDA: Device not initialized\n");
		ret = -ENODEV;
		goto out;
	}

	/* Find and remove the display */
	list_for_each_entry_safe(display, tmp, &g_sudovda_dev->displays, list) {
		if (display->id == args->display_id) {
			list_del(&display->list);
			sudovda_display_destroy(display);
			ret = 0;
			break;
		}
	}

	if (ret == -ENOENT)
		pr_err("SudoVDA: Display ID %d not found\n", args->display_id);

out:
	mutex_unlock(&g_sudovda_mutex);
	return ret;
}

/* List displays IOCTL */
static int sudovda_ioctl_list_displays(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct sudovda_list_displays *args = data;
	struct sudovda_display *display;
	int count = 0;

	pr_info("SudoVDA: Listing displays\n");

	mutex_lock(&g_sudovda_mutex);

	if (!g_sudovda_dev) {
		pr_err("SudoVDA: Device not initialized\n");
		mutex_unlock(&g_sudovda_mutex);
		return -ENODEV;
	}

	/* Count displays */
	list_for_each_entry(display, &g_sudovda_dev->displays, list) {
		if (count >= SUDOVDA_MAX_DISPLAYS)
			break;

		args->displays[count].id = display->id;
		args->displays[count].connector_id = display->connector_id;
		strncpy(args->displays[count].name, display->name, 31);
		strncpy(args->displays[count].serial, display->serial, 31);
		args->displays[count].preferred_mode = display->preferred_mode;
		args->displays[count].edid = display->edid;
		args->displays[count].status = display->connected ? 1 : 0;

		count++;
	}

	args->count = count;

	mutex_unlock(&g_sudovda_mutex);

	pr_info("SudoVDA: Listed %d displays\n", count);
	return 0;
}

/* Get version IOCTL */
static int sudovda_ioctl_get_version(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct sudovda_version *args = data;

	pr_info("SudoVDA: Getting version\n");

	args->major = SUDOVDA_DRIVER_MAJOR;
	args->minor = SUDOVDA_DRIVER_MINOR;
	args->patch = SUDOVDA_DRIVER_PATCHLEVEL;
	strncpy(args->build_date, SUDOVDA_DRIVER_DATE, 31);

	return 0;
}

/* Ping IOCTL */
static int sudovda_ioctl_ping(struct drm_device *dev, void *data, struct drm_file *file)
{
	pr_info("SudoVDA: Ping received\n");
	return 0;
}

/* Initialize IOCTL interface */
int sudovda_ioctl_init(struct drm_device *dev)
{
	pr_info("SudoVDA: Initializing IOCTL interface\n");

	/* Allocate device structure */
	g_sudovda_dev = kzalloc(sizeof(*g_sudovda_dev), GFP_KERNEL);
	if (!g_sudovda_dev)
		return -ENOMEM;

	g_sudovda_dev->drm_dev = dev;
	INIT_LIST_HEAD(&g_sudovda_dev->displays);
	mutex_init(&g_sudovda_dev->displays_lock);
	g_sudovda_dev->next_display_id = 0;
	g_sudovda_dev->next_connector_id = 0;
	g_sudovda_dev->initialized = true;

	return 0;
}

/* Cleanup IOCTL interface */
void sudovda_ioctl_cleanup(void)
{
	struct sudovda_display *display, *tmp;

	pr_info("SudoVDA: Cleaning up IOCTL interface\n");

	if (!g_sudovda_dev)
		return;

	mutex_lock(&g_sudovda_mutex);

	/* Destroy all displays */
	list_for_each_entry_safe(display, tmp, &g_sudovda_dev->displays, list) {
		list_del(&display->list);
		sudovda_display_destroy(display);
	}

	kfree(g_sudovda_dev);
	g_sudovda_dev = NULL;

	mutex_unlock(&g_sudovda_mutex);
}

/* Get IOCTL table */
const struct drm_ioctl_desc *sudovda_get_ioctls(void)
{
	return sudovda_ioctls;
}

/* Get number of IOCTLs */
int sudovda_get_ioctl_count(void)
{
	return ARRAY_SIZE(sudovda_ioctls);
}
