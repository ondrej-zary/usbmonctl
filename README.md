usbmonctl - USB HID Monitor Control Utility
===========================================

Controls higher-end monitors with USB port that comply with USB Monitor Control Class Specification.

Requirements:
-------------
Linux, GCC, Make

Compilation:
------------
    $ make

Installation:
-------------
    # make install

This installs usbmonctl to /usr/bin and 95-usbmonctl-monitor.rules to
/etc/udev/rules.d.
The udev rule is needed to allow non-root users to control a monitor. It sets
group "video" on all hiddev* devices that are USB HID monitors.

Usage:
------
Usage: usbmonctl [OPTION] [DEVICE]
USB HID Monitor Control Utility

DEVICE is hiddevN device (usually /dev/hiddevN or /dev/usb/hiddevN
If DEVICE is omitted, first USB monitor found is assumed.
Use '-l' option to find all DEVICEs automatically.

Available OPTIONs:
  -g, --get=TYPE,NUMBER         get value of control NUMBER
                                (TYPE=F for FEATURE or I for INPUT)
  -s, --set=TYPE,NUMBER=VALUE   set value of control NUMBER to VALUE
                                (TYPE=F for FEATURE or O for OUTPUT)
  -c, --check                   check if DEVICE is an USB HID monitor (for udev use)
  -h, --help                    display this help and exit
  -l, --list                    list all USB monitors and their controls
  -v, --verbose                 be verbose
  -V, --version                 display version information and exit

Numbers can be specified in decimal or hexadecimal (prefixed by '0x').

Examples:
  usbmonctl -s O,0x01,0,0=1             degauss
  usbmonctl -g F,16             get current brightness value
  usbmonctl -s F,0x12=10        set contrast to 10

Tested and working monitors:
----------------------------

Monitor name                                                                | USB ID
--------------------------------------------------------------------------- |-----------------
Samsung SyncMaster 757DFX                                                   | 0x0419:0x8002
Samsung SyncMaster 765MB                                                    | 0x0419:0x8002
Eizo FlexScan HD2441W                                                       | 0x056d:0x0002
Eizo FlexScan S1921                                                         | 0x056d:0x0002
Apple Cinema HD Display M8536 (with Apple DVI to ADC adapter A1006 EMC1918) | 0x05ac:0x9218
