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

Tested and working monitors:
----------------------------

Monitor name                 | USB ID
-----------------------------|-----------------
Samsung SyncMaster 757DFX    | 0x0419:0x8002
Samsung SyncMaster 765MB     | 0x0419:0x8002
Eizo FlexScan HD2441W        | 0x056d:0x0002
Eizo FlexScan S1921          | 0x056d:0x0002
