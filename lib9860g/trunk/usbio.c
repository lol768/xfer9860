/**
 * @file usbio.c
 * @author Andreas Bertheussen
 * @brief Creates a layer of abstraction over the libusb-functionality.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <usb.h>

#include "usbio.h"
#include "log.h"

struct usb_device *fx_findDevice(void) {
	struct usb_bus *usb_bus;
	struct usb_device *dev;
	usb_init();
	usb_find_busses();
	usb_find_devices();
	for (usb_bus = usb_get_busses(); usb_bus; usb_bus = usb_bus->next) {
		for (dev = usb_bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == 0x07CF) && (dev->descriptor.idProduct == 0x6101))
				/* Device found */
				return dev;
		}
	}
	return NULL;
}


struct usb_dev_handle *fx_claimDevice(void) {
	int ret, interface = 0;
	char *buffer;	/* for control messages */
	struct usb_device *device;
	struct usb_dev_handle *deviceHandle;

	
	if ((device = fx_findDevice()) == NULL) {
		FX_LOG(1, "%s: Device not found on bus(es)", __func__)	return NULL;
		goto fail;
	}

	deviceHandle = usb_open(device);
	if (deviceHandle == NULL) {
		FX_LOG(1, "%s: usb_open() failed: %s", __func__, usb_strerror()) return NULL;
		goto fail;
	}

	ret = usb_set_configuration(deviceHandle, 1);
	if (ret < 0) { 
		FX_LOG(1, "%s: usb_set_configuration() failed: %s", __func__, usb_strerror())
		goto fail_close;
	}

	interface = usb_claim_interface(deviceHandle, 0);
	if (interface < 0) {
		FX_LOG(1, "%s, usb_claim_interface() failed: %s", __func__, usb_strerror())
		goto fail_close;
	}

	buffer = malloc(39 * sizeof(char));

	ret = usb_control_msg(deviceHandle, 0x80, 0x6, 0x100, 0, buffer, 0x12, 200);
	if (ret < 0) {
		FX_LOG(1, "%s, first usb_control_msg() failed: %s", __func__, usb_strerror())
		goto fail_free_release_close;
	}

	ret = usb_control_msg(deviceHandle, 0x80, 0x6, 0x200, 0, buffer, 0x29, 250);
	if (ret < 0) {
		FX_LOG(1, "%s, second usb_control_msg() failed: %s", __func__, usb_strerror())
		goto fail_free_release_close;
	}

	ret = usb_control_msg(deviceHandle, 0x41, 0x1, 0x0, 0, buffer, 0x0, 250);
	if (ret < 0) {
		FX_LOG(1, "%s, third usb_control_msg() failed: %s", __func__, usb_strerror())
		goto fail_free_release_close;
	}

	/* Normal return: */
	free(buffer);
	return deviceHandle;

	/* Cleanup on error:*/	
	fail_free_release_close:
		free(buffer);
		usb_release_interface(deviceHandle, interface);
	fail_close:
		usb_close(deviceHandle);
	fail:
		return NULL;
}

/* bpl is the number of bytes to show per line */
int fx_printData(char *data, unsigned int length, unsigned int bpl, int direction) {
	int line, byte;
	if (data == NULL) { FX_LOG(1, "%s: provided data pointer is NULL pointer.", __func__) return -1; }
	for (line = 0; line*bpl < length; line++) {
		/* Print in hex */
		
		if (direction == 1 || direction == 0) {
			fprintf(stderr, direction ? " << " : " >> ");
		} else { printf("\t"); }
		
		for (byte = line*bpl; byte < (line*bpl + bpl); byte++) {
			if (byte < length) { fprintf(stderr, "%02X ", data[byte] & 0xFF); }
			else { fprintf(stderr, "   "); } /* if done, pad so that ascii chars line up */
		}
		fprintf(stderr, "    ");
		
		/* Print in ascii */
		for (byte = line*bpl; byte < (line*bpl + bpl); byte++) {
			if (byte >= length) { break; }
			fprintf(stderr, "%c", isprint(data[byte]) ? (data[byte]&0xFF) : '.');
		}
		
		fprintf(stderr, "\n");
	}
	return 0;
}

int fx_releaseDevice(struct usb_dev_handle *deviceHandle, int disconnectOptions) {
	/*
	 *TODO: Use the disconnectOptions. Treat the bits in disconnectOptions as options, to allow OR'ing options together.
	 *	Possible options to implement;
	 *	- show a "Complete" dialog when finished.
	 *	- initiate a storage memory cleanup.
	*/
	/* TODO: Error checking not so important on release/close. Maybe check it?*/
	usb_release_interface(deviceHandle, 0);
	usb_close(deviceHandle);
	return 0;
}

int fx_write(struct usb_dev_handle *deviceHandle, char *buffer, int length) {
	int ret = 0;
	/* Optional debugging */
	FX_LOG(2, "%s: %i bytes of data:", __func__, length)
	if (2 <= fx_debugLevel) { fx_printData(buffer, length, 16, 0); }
	fflush(stderr);
	ret = usb_bulk_write(deviceHandle, 0x1, buffer, length, 5000);
	if (ret < 0) {
		FX_LOG(1, "%s: usb_bulk_write() failed: %s", __func__, usb_strerror())
		return -1;
	}

	return ret;
}

int fx_read(struct usb_dev_handle *deviceHandle, char *buffer, int length) {
	int ret = 0;
	ret = usb_bulk_read(deviceHandle, 0x82, buffer, length, 5000);
	if (ret < 0) {
		FX_LOG(1, "%s: usb_bulk_read() failed: %s", __func__, usb_strerror())
		return -1;
	}
	FX_LOG(2, "%s(): got %i of %i requested bytes of data:\n", __func__, ret, length)
	if (2 <= fx_debugLevel && ret > 0) { fx_printData(buffer, ret, 16, 1); }

	return ret;
}


