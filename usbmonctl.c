#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/hiddev.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define VERSION "1.1"

#define HID_USAGE_PAGE		0xffff0000

#define HID_UP_MONITOR		0x00800000
#define HID_UP_MONITOR_ENUM	0x00810000
#define HID_UP_MONITOR_VESA	0x00820000

struct name_table {
	unsigned char id;
	char *name;
	bool hidden;	/* to hide broken controls */
};

/* Generic names from USB Monitor Controll Class Specification */
static struct name_table vesa_controls_generic[] = {
	{ .id = 0x01, .name = "Degauss" },
	{ .id = 0x10, .name = "Brightness" },
	{ .id = 0x12, .name = "Contrast" },
	{ .id = 0x16, .name = "Red Video Gain" },
	{ .id = 0x18, .name = "Green Video Gain" },
	{ .id = 0x1a, .name = "Blue Video Gain" },
	{ .id = 0x1c, .name = "Focus" },
	{ .id = 0x20, .name = "Horizontal Position" },
	{ .id = 0x22, .name = "Horizontal Size" },
	{ .id = 0x24, .name = "Horizontal Pincushion" },
	{ .id = 0x26, .name = "Horizontal Pincushion Balance" },
	{ .id = 0x28, .name = "Horizontal Misconvergence" },
	{ .id = 0x2a, .name = "Horizontal Linearity" },
	{ .id = 0x2c, .name = "Horizontal Linearity Balance" },
	{ .id = 0x30, .name = "Vertical Position" },
	{ .id = 0x32, .name = "Vertical Size" },
	{ .id = 0x34, .name = "Vertical Pincushion" },
	{ .id = 0x36, .name = "Vertical Pincushion Balance" },
	{ .id = 0x38, .name = "Vertical Misconvergence" },
	{ .id = 0x3a, .name = "Vertical Linearity" },
	{ .id = 0x3c, .name = "Vertical Linearity Balance" },
	{ .id = 0x40, .name = "Parallelogram Balance (Key Distortion)" },
	{ .id = 0x42, .name = "Trapezoidal Distortion (Key)" },
	{ .id = 0x44, .name = "Tilt (Rotation)" },
	{ .id = 0x46, .name = "Top Corner Distortion Control" },
	{ .id = 0x48, .name = "Top Corner Distortion Balance" },
	{ .id = 0x4a, .name = "Bottom Corner Distortion Control" },
	{ .id = 0x4c, .name = "Bottom Corner Distortion Balance" },
	{ .id = 0x56, .name = "Horizontal Moire" },
	{ .id = 0x58, .name = "Vertical Moire" },
	{ .id = 0x5e, .name = "Input Level Select" },
	{ .id = 0x60, .name = "Input Source Select" },
	{ .id = 0x6c, .name = "Red Video Black Level" },
	{ .id = 0x6e, .name = "Green Video Black Level" },
	{ .id = 0x70, .name = "Blue Video Black Level" },
	{ .id = 0xa2, .name = "Auto Size Center" },
	{ .id = 0xa4, .name = "Polarity Horizontal Sychronization" },
	{ .id = 0xa6, .name = "Polarity Vertical Synchronization" },
	{ .id = 0xaa, .name = "Screen Orientation" },
	{ .id = 0xac, .name = "Horizontal Frequency in Hz" },
	{ .id = 0xae, .name = "Vertical Frequency in 0.1 Hz" },
	{ .id = 0xb0, .name = "Settings" },
	{ .id = 0xca, .name = "On Screen Display (OSD)" },
	{ .id = 0xd4, .name = "Stereo Mode" },
	{ .id = 0 }
};

#define USB_VENDOR_SAMSUNG 0x0419
/* Samsung-specific control names, found on SyncMaster 757DFX */
static struct name_table vesa_controls_samsung[] = {
	{ .id = 0x1c, .name = "???", .hidden = true },
	{ .id = 0x34, .name = "???", .hidden = true },
	{ .id = 0x36, .name = "???", .hidden = true },
	{ .id = 0x54, .name = "Color Temperature" },
	{ .id = 0x92, .name = "???", .hidden = true },
	{ .id = 0x94, .name = "???", .hidden = true },
	{ .id = 0x96, .name = "???", .hidden = true },
	{ .id = 0x98, .name = "???", .hidden = true },
	{ .id = 0x9c, .name = "???", .hidden = true },
	{ .id = 0x9e, .name = "???", .hidden = true },
	{ .id = 0xa1, .name = "Highlight Zone Horizontal Position" },
	{ .id = 0xa2, .name = "Highlight Zone Vertical Position" },
	{ .id = 0xa3, .name = "Highlight Zone Horizontal Size" },
	{ .id = 0xa4, .name = "Highlight Zone Vertical Size" },
	{ .id = 0xa5, .name = "Highlight Zone Contrast" },
	{ .id = 0xa6, .name = "Highlight Zone Red" },
	{ .id = 0xa7, .name = "Highlight Zone Green" },
	{ .id = 0xa8, .name = "Highlight Zone Blue" },
	{ .id = 0xa9, .name = "Highlight Zone Sharpness" },
	{ .id = 0xaa, .name = "Red Video Custom Value" },
	{ .id = 0xab, .name = "Green Video Custom Value" },
	{ .id = 0xaf, .name = "Blue Video Custom Value" },
	{ .id = 0xc6, .name = "Settings Management (1 = save current settings, 2 = recall default values)" },
	{ .id = 0xb1, .name = "Highlight Zone Horizontal Size (read-only)" },
	{ .id = 0xb2, .name = "Highlight Zone Vertical Size (read-only)" },
	{ .id = 0xb3, .name = "Highlight Zone ??? (read-only)" },
	{ .id = 0xb4, .name = "Highlight Zone ??? (read-only)" },
	{ .id = 0xb5, .name = "Highlight Zone ??? (read-only)" },
	{ .id = 0xd8, .name = "Horizontal Size (not in OSD)" },
	{ .id = 0xdc, .name = "Highlight Zone/OSD Control (0=off, 2=on, 4=fullscreen/8=lock OSD, 16=unlock OSD)" },
	{ .id = 0xee, .name = "Vertical Focus" },
	{ .id = 0xf0, .name = "Unknown" },
	{ .id = 0xf5, .name = "???", .hidden = true },
	{ .id = 0 }
};

static char *hiddev_paths[] = { "/dev/", "/dev/usb/", NULL };

bool check = false;
bool verbose = false;
#define verbose_printf	if (verbose) printf

int monitor_fd = 0;
int monitor_vendor = 0;

char *get_name_from_table(int id, struct name_table table[]) {
	for (int i = 0; table[i].id != 0; i++)
		if (table[i].id == id)
			return table[i].name;
	return NULL;
}

/* Convert REPORT id into readable name */
char *control_name(int id) {
	char *retval = NULL;

	/* Check vendor-specific lists first */
	switch (monitor_vendor) {
	case USB_VENDOR_SAMSUNG:
		retval = get_name_from_table(id, vesa_controls_samsung);
		break;
	}
	if (!retval)
		retval = get_name_from_table(id, vesa_controls_generic);

	return retval ? retval : "unknown";
}

/* Check if a control is broken and should be hidden */
bool control_hidden(int id) {
	struct name_table *table = NULL;

	switch (monitor_vendor) {
	case USB_VENDOR_SAMSUNG:
		table = vesa_controls_samsung;
		break;
	}
	if (!table)
		return false;
	for (int i = 0; table[i].id != 0; i++)
		if (table[i].id == id)
			return table[i].hidden;
	return false;
}

/* Convert REPORT number into readable name */
char *type_name(int id) {
	switch (id) {
		case 1:  return "INPUT  ";
		case 2:  return "OUTPUT ";
		case 3:  return "FEATURE";
		default: return "UNKNOWN";
	}
}

bool get_control_value(int fd, int report_type, int report_id, int field_idx,
		      int usage_idx, int *value) {
	if (control_hidden(report_id)) {
		fprintf(stderr, "Control 0x%x is hidden because it's probably broken.\n", report_id);
		return false;
	}
	struct hiddev_report_info rinfo = {
		.report_type = report_type,
		.report_id   = report_id,
	};
	struct hiddev_usage_ref uref = {
		.report_type = report_type,
		.report_id   = report_id,
		.field_index = field_idx,
		.usage_index = usage_idx,
	};
	if (ioctl(fd, HIDIOCGREPORT, &rinfo) < 0) {
		fprintf(stderr, "HIDIOCGREPORT failed - unable to get control value.\n");
		return false;
	}
	if (ioctl(fd, HIDIOCGUSAGE, &uref) < 0) {
		fprintf(stderr, "HIDIOCGUSAGE failed - unable to get control value.\n");
		return false;
	}
	*value = uref.value;
	return true;
}

bool set_control_value(int fd, int report_type, int report_id, int field_idx,
		      int usage_idx, int value) {
	if (control_hidden(report_id)) {
		fprintf(stderr, "Control 0x%x is hidden because it's probably broken.\n", report_id);
		return false;
	}
	struct hiddev_report_info rinfo = {
		.report_type = report_type,
		.report_id   = report_id,
	};
	struct hiddev_usage_ref uref = {
		.report_type = report_type,
		.report_id   = report_id,
		.field_index = field_idx,
		.usage_index = usage_idx,
		.value       = value,
	};
	if (ioctl(fd, HIDIOCSUSAGE, &uref) < 0) {
		fprintf(stderr, "HIDIOCSUSAGE failed - unable to set control value.\n");
		return false;
	}
	if (ioctl(fd, HIDIOCSREPORT, &rinfo) < 0) {
		fprintf(stderr, "HIDIOCSREPORT failed - unable to set control value.\n");
		return false;
	}
	return true;
}

void get_controls(int fd, int report_type) {
	if (ioctl(fd, HIDIOCINITREPORT, 0) < 0) {
		fprintf(stderr, "HIDIOCINITREPORT failed - unable to get controls.\n");
		return;
	}

	struct hiddev_report_info rinfo = {
		.report_type = report_type,
		.report_id = HID_REPORT_ID_FIRST,
	};
	int ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
	/* walk all reports */
	while (ret >= 0) {
		if (control_hidden(rinfo.report_id)) {
			rinfo.report_id |= HID_REPORT_ID_NEXT;
			ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
			continue;
		}
		printf("%s: 0x%02x - %s\n", type_name(rinfo.report_type),rinfo.report_id,control_name(rinfo.report_id));
		/* walk all fields of a report */
		for (unsigned int i = 0; i < rinfo.num_fields; i++) {
			struct hiddev_field_info finfo = {
				.report_type = rinfo.report_type,
				.report_id = rinfo.report_id,
				.field_index = i,
			};
			if (ioctl(fd, HIDIOCGFIELDINFO, &finfo) < 0) {
				fprintf(stderr, "HIDIOCGFIELDINFO failed - unable to get field info for field %d.\n", finfo.field_index);
				continue;
			} else
				printf("\tfield %d, flags=%d, range=%d..%d\n",
					finfo.field_index, finfo.flags,
					finfo.logical_minimum,
					finfo.logical_maximum);
			/* walk all usages of a field */
			for (unsigned int j = 0; j < finfo.maxusage; j++) {
				struct hiddev_usage_ref uref = {
					.report_type = rinfo.report_type,
					.report_id = rinfo.report_id,
					.field_index = i,
					.usage_index = j,
				};
				/* get value */
				if (ioctl(fd, HIDIOCGUSAGE, &uref) < 0)
					fprintf(stderr, "HIDIOCGUSAGE failed - unable to get control value for usage %d.\n", uref.usage_index);
				else
					printf("\t\tusage %d = %d (0x%x)\n",
						uref.usage_index, uref.value,
						uref.value);
			}
		}
		rinfo.report_id |= HID_REPORT_ID_NEXT;
		ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
	}
}

/* Check if a hiddev device is an USB HID monitor */
bool is_monitor(char *devname, bool keep_open) {
	struct hiddev_devinfo dev_info;
	int fd = open(devname, O_RDONLY);

	if (fd < 1) {
		perror("Unable to open device");
		return false;
	}
	if (ioctl(fd, HIDIOCGDEVINFO, &dev_info) < 0)
		return false;
	for (unsigned int i = 0; i < dev_info.num_applications; i++) {
		int usage = ioctl(fd, HIDIOCAPPLICATION, i);
		if (usage < 1)
			continue;
		if (((usage & HID_USAGE_PAGE) == HID_UP_MONITOR)) {
			char name[256];
			name[0] = 0;
			if (ioctl(fd, HIDIOCGNAME(sizeof(name)), &name) < 0)
				strcpy(name, "Unable to get device name");
			if (!keep_open || verbose)
				printf("%s (0x%04hx:0x%04hx) v%x.%02x\n",
				       name, dev_info.vendor,
				       dev_info.product, dev_info.version >> 8,
				       dev_info.version & 0xff);
			if (!keep_open) {
				monitor_vendor = dev_info.vendor;
				get_controls(fd, HID_REPORT_TYPE_INPUT);
				get_controls(fd, HID_REPORT_TYPE_OUTPUT);
				get_controls(fd, HID_REPORT_TYPE_FEATURE);
				printf("\n");
			}
			break;
		}
	}
	if (keep_open) {
		monitor_fd = fd;
		monitor_vendor = dev_info.vendor;
	} else
		close(fd);
	return true;
}

/* Filter to find hiddevN files for scandir() */
int is_hiddev(const struct dirent *ent) {
	return !strncmp(ent->d_name, "hiddev", strlen("hiddev"));
}

/* Find all hiddevN devices that are USB HID monitors
   and list them or find only first one */
int find_monitors(bool list_all) {
	char path[PATH_MAX];
	int mon_count = 0;

	for (int i = 0; hiddev_paths[i] != NULL; i++) {
		struct dirent **filelist;
		int count = scandir(hiddev_paths[i], &filelist, is_hiddev,
				    alphasort);
		if (count < 0)
			continue;
		for (int j = 0; j < count; j++) {
			snprintf(path, PATH_MAX, "%s%s", hiddev_paths[i],
				 filelist[j]->d_name);
			if (list_all || verbose)
				printf("%s: ", path);
			if (is_monitor(path, !list_all))
				mon_count++;
			else if (list_all || verbose)
				printf("not an USB monitor\n");
			free(filelist[j]);
		}
		free(filelist);
	}

	return mon_count;
}

void help(char *progname) {
	printf("Usage: %s [OPTION] [DEVICE]\n", progname);
	printf("USB HID Monitor Control Utility\n\n");
	printf("DEVICE is hiddevN device (usually /dev/hiddevN or /dev/usb/hiddevN\n");
	printf("If DEVICE is omitted, first USB monitor found is assumed.\n");
	printf("Use '-l' option to find all DEVICEs automatically.\n");
	printf("\nAvailable OPTIONs:\n");
	printf("  -g, --get=TYPE,NUMBER\t\tget value of control NUMBER\n");
	printf("        \t\t\t(TYPE=F for FEATURE or I for INPUT)\n");
	printf("  -s, --set=TYPE,NUMBER=VALUE\tset value of control NUMBER to VALUE\n");
	printf("        \t\t\t(TYPE=F for FEATURE or O for OUTPUT)\n");
	printf("  -c, --check\t\t\tcheck if DEVICE is an USB HID monitor (for udev use)\n");
	printf("  -h, --help\t\t\tdisplay this help and exit\n");
	printf("  -l, --list\t\t\tlist all USB monitors and their controls\n");
	printf("  -v, --verbose\t\t\tbe verbose\n");
	printf("  -V, --version\t\t\tdisplay version information and exit\n");
	printf("\nNumbers can be specified in decimal or hexadecimal (prefixed by '0x').\n");
	printf("\nExamples:\n");
	printf("  usbmonctl -s O,0x01,0,0=1\t\tdegauss\n");
	printf("  usbmonctl -g F,16\t\tget current brightness value\n");
	printf("  usbmonctl -s F,0x12=10\tset contrast to 10\n");
}

void version() {
	printf("usbmonctl v%s - USB HID Monitor Control Utility\n", VERSION);
	printf("Copyright (c) 2010 Ondrej Zary - http://www.rainbow-software.org\n");
	printf("License: GLPv2\n");
}

/* Parse decimal or hexadecimal (prefixed by 0x) number string */
bool parse_number(char *str, int *num) {
	return sscanf(str, strstr(str, "0x") ? "%x" : "%d", num);
}

/* parse control argument TYPE,ID,FIELD,USAGE[=VALUE] */
void parse_control_arg(char *arg, int *type, int *id, int *field, int *usage, int *value) {
	/* FIELD and USAGE are optional */
	*field = 0;
	*usage = 0;

	/* parse VALUE if requested */
	if (value) {
		char *pos = strchr(arg, '=');
		if (!pos) {
			fprintf(stderr, "Missing VALUE in set command.\n");
			exit(2);
		}
		pos++;
		if (!parse_number(pos, value)) {
			fprintf(stderr, "Invalid value '%s'\n", pos);
			exit(2);
		}
	}

	int state = 0;
	while (arg != NULL && *arg != 0 && *arg != '=') {
		/* compute length of current part */
		char *pos = strchr(arg, ',');
		if (!pos)
			pos = strchr(arg, '=');
		int len = (pos) ? pos-arg : (int)strlen(arg);
		/* copy current part to tmp */
		char tmp[16];
		memset(tmp, 0, 16);
		strncpy(tmp, arg, (len < 16) ? len : 16);

		int i;
		if (state > 0) {
			if (!parse_number(tmp, &i)) {
				fprintf(stderr, "Invalid number '%s'\n", tmp);
				exit(2);
			}
		}
		/* state machine */
		switch (state) {
		case 0:
			if (len != 1) {
				fprintf(stderr, "Invalid type '%s'\n", tmp);
				exit(2);
			}
			switch (tmp[0]) {
			case 'F':
				*type = HID_REPORT_TYPE_FEATURE;
				break;
			case 'I':
				if (value) {
					fprintf(stderr, "Type I is invalid for set operation.\n");
					exit(2);
				}
				*type = HID_REPORT_TYPE_INPUT;
				break;
			case 'O':
				if (!value) {
					fprintf(stderr, "Type O is invalid for get operation.\n");
					exit(2);
				}
				*type = HID_REPORT_TYPE_OUTPUT;
				break;
			default:
				fprintf(stderr, "Invalid type '%s'\n", tmp);
				exit(2);
			}
			break;
		case 1:	*id = i; break;
		case 2: *field  = i; break;
		case 3: *usage  = i; break;
		}
		state++;
		/* move to the next part */
		arg = strchr(arg, ',');
		if (arg)
			arg++;
	}
}

void getopt_pass(int argc, char *argv[], bool execute) {
	while (1) {
		int type, id, field, usage, value;
		extern char *optarg;
		struct option long_options[] = {
			{ .name = "get",     .has_arg = 1, .val = 'g' },
			{ .name = "set",     .has_arg = 1, .val = 's' },
			{ .name = "check",   .has_arg = 0, .val = 'c' },
			{ .name = "help",    .has_arg = 0, .val = 'h' },
			{ .name = "list",    .has_arg = 0, .val = 'l' },
			{ .name = "verbose", .has_arg = 0, .val = 'v' },
			{ .name = "version", .has_arg = 0, .val = 'V' },
			{ .name = NULL },
		};
		int opt = getopt_long(argc, argv, "g:s:chlvV", long_options, NULL);
		if (opt < 0)
			break;
		switch (opt) {
		case 'g':
			parse_control_arg(optarg, &type, &id, &field, &usage, NULL);
			if (execute) {
				int value;
				if (get_control_value(monitor_fd, type, id, field, usage, &value)) {
					verbose_printf("Control %s (0x%x) value is ", control_name(id), id);
					printf("%d (0x%x)\n", value, value);
				}
			}
			break;
		case 's':
			parse_control_arg(optarg, &type, &id, &field, &usage, &value);
			if (execute) {
				verbose_printf("Setting control %s to %d\n", control_name(id), value);
				set_control_value(monitor_fd, type, id, field, usage, value);
			}
			break;
		case 'c':
			check = true;
			break;
		case 'h':
			help(argv[0]);
			exit(0);
		case 'l':
			printf("Found %d USB HID monitors.\n", find_monitors(true));
			exit(0);
		case 'v':
			verbose = true;
			break;
		case 'V':
			version();
			exit(0);
		default:
			fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
			exit(2);
		}
	}

}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		help(argv[0]);
		exit(0);
	}
	extern int optind;

	/* Parse arguments but don't execute any monitor control commands yet */
	getopt_pass(argc, argv, false);

	/* If DEVICE is specified, use it */
	if (optind < argc) {
		verbose_printf("Using specified device %s: ", argv[optind]);
		if (!is_monitor(argv[optind], true)) {
			if (!check)
				fprintf(stderr, "Specified device '%s' is not an USB HID monitor!\n", argv[optind]);
			exit(1);
		}
	} else {	/* otherwise find one */
		if (find_monitors(false) < 1) {
			fprintf(stderr, "No USB HID monitors found!\n");
			exit(1);
		}
	}

	/* Now we have DEVICE, execute monitor control commands */
	optind = 1;
	getopt_pass(argc, argv, true);

	if (monitor_fd > 0)
		close(monitor_fd);
	return 0;
}
