usbmonctl: usbmonctl.c
	gcc --std=gnu99 -Wall -Wextra usbmonctl.c -o usbmonctl

install: usbmonctl
	install -o root -g root -m 755 -s usbmonctl /usr/bin/
	install -o root -g root -m 644 95-usbmonctl-monitor.rules /etc/udev/rules.d/

clean: 
	rm -f usbmonctl
