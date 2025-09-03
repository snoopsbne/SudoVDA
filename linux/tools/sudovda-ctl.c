/*
 * SudoVDA Control Tool
 * User-space utility for managing virtual displays
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>

#include "../include/sudovda.h"

#define SUDOVDA_DEVICE "/dev/dri/card0"

static void print_usage(const char *progname)
{
	printf("Usage: %s [OPTIONS] COMMAND\n", progname);
	printf("\nCommands:\n");
	printf("  add WIDTH HEIGHT REFRESH_RATE NAME SERIAL    Add a virtual display\n");
	printf("  remove DISPLAY_ID                            Remove a virtual display\n");
	printf("  list                                         List all virtual displays\n");
	printf("  version                                      Show driver version\n");
	printf("  ping                                         Ping the driver\n");
	printf("\nOptions:\n");
	printf("  -d, --device DEVICE    DRM device to use (default: %s)\n", SUDOVDA_DEVICE);
	printf("  -h, --help             Show this help message\n");
	printf("\nExamples:\n");
	printf("  %s add 1920 1080 60000 \"Virtual Display\" \"VD001\"\n", progname);
	printf("  %s remove 0\n", progname);
	printf("  %s list\n", progname);
}

static int open_drm_device(const char *device)
{
	int fd = open(device, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open device %s: %s\n", device, strerror(errno));
		return -1;
	}
	return fd;
}

static int cmd_add_display(int fd, int argc, char *argv[])
{
	struct sudovda_add_display args;
	int ret;

	if (argc < 5) {
		fprintf(stderr, "Error: add command requires WIDTH HEIGHT REFRESH_RATE NAME SERIAL\n");
		return -1;
	}

	memset(&args, 0, sizeof(args));
	args.width = atoi(argv[0]);
	args.height = atoi(argv[1]);
	args.refresh_rate = atoi(argv[2]);
	strncpy(args.name, argv[3], sizeof(args.name) - 1);
	strncpy(args.serial, argv[4], sizeof(args.serial) - 1);

	printf("Adding virtual display: %dx%d@%dHz, name='%s', serial='%s'\n",
		args.width, args.height, args.refresh_rate, args.name, args.serial);

	ret = ioctl(fd, DRM_IOCTL_SUDOVDA_ADD_DISPLAY, &args);
	if (ret < 0) {
		fprintf(stderr, "Failed to add display: %s\n", strerror(errno));
		return -1;
	}

	printf("Display added successfully: ID=%d, Connector=%d\n", 
		args.display_id, args.connector_id);
	return 0;
}

static int cmd_remove_display(int fd, int argc, char *argv[])
{
	struct sudovda_remove_display args;
	int ret;

	if (argc < 1) {
		fprintf(stderr, "Error: remove command requires DISPLAY_ID\n");
		return -1;
	}

	memset(&args, 0, sizeof(args));
	args.display_id = atoi(argv[0]);

	printf("Removing display ID %d\n", args.display_id);

	ret = ioctl(fd, DRM_IOCTL_SUDOVDA_REMOVE_DISPLAY, &args);
	if (ret < 0) {
		fprintf(stderr, "Failed to remove display: %s\n", strerror(errno));
		return -1;
	}

	printf("Display removed successfully\n");
	return 0;
}

static int cmd_list_displays(int fd, int argc, char *argv[])
{
	struct sudovda_list_displays args;
	int ret, i;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	memset(&args, 0, sizeof(args));

	ret = ioctl(fd, DRM_IOCTL_SUDOVDA_LIST_DISPLAYS, &args);
	if (ret < 0) {
		fprintf(stderr, "Failed to list displays: %s\n", strerror(errno));
		return -1;
	}

	printf("Virtual Displays (%d total):\n", args.count);
	printf("ID  Connector  Resolution        Refresh  Name            Serial     Status\n");
	printf("--- ---------  ----------------  -------  --------------- ---------- ------\n");

	for (i = 0; i < args.count; i++) {
		printf("%-3d %-9d %-16s %-7d  %-15s %-10s %s\n",
			args.displays[i].id,
			args.displays[i].connector_id,
			"", /* Resolution will be filled from preferred_mode */
			args.displays[i].preferred_mode.refresh_rate / 1000,
			args.displays[i].name,
			args.displays[i].serial,
			args.displays[i].status ? "Connected" : "Disconnected");
	}

	return 0;
}

static int cmd_version(int fd, int argc, char *argv[])
{
	struct sudovda_version args;
	int ret;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	memset(&args, 0, sizeof(args));

	ret = ioctl(fd, DRM_IOCTL_SUDOVDA_GET_VERSION, &args);
	if (ret < 0) {
		fprintf(stderr, "Failed to get version: %s\n", strerror(errno));
		return -1;
	}

	printf("SudoVDA Driver Version: %d.%d.%d\n", args.major, args.minor, args.patch);
	printf("Build Date: %s\n", args.build_date);
	return 0;
}

static int cmd_ping(int fd, int argc, char *argv[])
{
	int ret;

	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	ret = ioctl(fd, DRM_IOCTL_SUDOVDA_PING, NULL);
	if (ret < 0) {
		fprintf(stderr, "Failed to ping driver: %s\n", strerror(errno));
		return -1;
	}

	printf("Driver ping successful\n");
	return 0;
}

int main(int argc, char *argv[])
{
	const char *device = SUDOVDA_DEVICE;
	const char *progname = argv[0];
	int fd, ret;
	char *command;

	static struct option long_options[] = {
		{"device", required_argument, 0, 'd'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	/* Parse options */
	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "d:h", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'd':
			device = optarg;
			break;
		case 'h':
			print_usage(progname);
			return 0;
		case '?':
			print_usage(progname);
			return 1;
		default:
			abort();
		}
	}

	/* Get command */
	if (optind >= argc) {
		fprintf(stderr, "Error: No command specified\n");
		print_usage(progname);
		return 1;
	}

	command = argv[optind];
	argc -= optind + 1;
	argv += optind + 1;

	/* Open DRM device */
	fd = open_drm_device(device);
	if (fd < 0)
		return 1;

	/* Execute command */
	if (strcmp(command, "add") == 0) {
		ret = cmd_add_display(fd, argc, argv);
	} else if (strcmp(command, "remove") == 0) {
		ret = cmd_remove_display(fd, argc, argv);
	} else if (strcmp(command, "list") == 0) {
		ret = cmd_list_displays(fd, argc, argv);
	} else if (strcmp(command, "version") == 0) {
		ret = cmd_version(fd, argc, argv);
	} else if (strcmp(command, "ping") == 0) {
		ret = cmd_ping(fd, argc, argv);
	} else {
		fprintf(stderr, "Error: Unknown command '%s'\n", command);
		print_usage(progname);
		ret = 1;
	}

	close(fd);
	return ret;
}
