/*******************************************************************************
	xfer9860 - fx-9860G (SD) communication utility
	Copyright (C) 2007
		Manuel Naranjo <naranjo.manuel@gmail.com>
		Andreas Bertheussen <andreasmarcel@gmail.com>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
	MA  02110-1301, USA.
*******************************************************************************/

#include <stdio.h>
#include <usb.h>
#include <string.h>

#include "Casio9860.h"

struct usb_dev_handle* fx_getDeviceHandle() {
	int ret = 0;
	struct usb_device *usb_dev;
	struct usb_dev_handle *usb_handle;

	usb_dev = (struct usb_device*)device_init();
	if (usb_dev == NULL) {
		return (struct usb_dev_handle*)-1;	// this is a common 'error', won't print error message
	}

	usb_handle = (struct usb_dev_handle*)usb_open(usb_dev);
	if (usb_handle == NULL) {
		printf("\nERR: usb_open() returned NULL.\n");
		return NULL;
	}

	ret = usb_set_configuration(usb_handle, 1);
	if (ret < 0) { // needed on WIN32
		printf("\nERR: usb_set_configuration(): %i\n", ret);
		usb_close(usb_handle);
		return NULL;
	}

	ret = usb_claim_interface(usb_handle, 0);
	if (ret < 0) {
		printf("\nERR: usb_claim_interface(): %i\n", ret);
		usb_close(usb_handle);
		return NULL;
	}

	return usb_handle;
}

struct usb_device *device_init(void) {
	struct usb_bus *usb_bus;
	struct usb_device *dev;
	usb_init();
	usb_find_busses();
	usb_find_devices();
	for (usb_bus = usb_busses; usb_bus; usb_bus = usb_bus->next) {
		for (dev = usb_bus->devices; dev; dev = dev->next) {
			if ((dev->descriptor.idVendor == C9860_VENDOR_ID) && (dev->descriptor.idProduct == C9860_PRODUCT_ID))
				return dev;
		}
	}
	return NULL;
}

int fx_initDevice(struct usb_dev_handle *usb_handle) {
	int ret;
	char* buffer;
	buffer = calloc(0x29, sizeof(char));

	ret = usb_control_msg(usb_handle, 0x80, 0x6, 0x100, 0, buffer, 0x12, 200);
	debug(1, buffer, ret);

	if (ret < 0) {
		fprintf(stderr, "\nERR: fx_initDevice(), 1'st control: %i \n", ret);
		goto exit;
	}

	ret = usb_control_msg(usb_handle, 0x80, 0x6, 0x200, 0, buffer, 0x29, 250);
	debug(1, buffer, ret);
	if (ret < 0) {
		fprintf(stderr, "\nERR: fx_initDevice(), 2'nd control: %i \n", ret);
		goto exit;
	}

	ret = usb_control_msg(usb_handle, 0x41, 0x1, 0x0, 0, buffer, 0x0, 250);
	debug(1, buffer, ret);
	if (ret < 0) {
		fprintf(stderr, "\nERR: fx_initDevice(), 3'rd control: %i \n", ret);
		goto exit;
	}
	ret = 0;
exit:
	free(buffer);
	return ret;
}

int fx_sendComplete(struct usb_dev_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x01\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendVerify(struct usb_dev_handle *usb_handle, char *buffer, char *type) {
	/* Type: 0x05
	 * ST: 00 or 01*/
	memcpy(buffer, "\x05\x30\x30\x30", 4);
	if (type[1] == '1') {
		memcpy(buffer+2, "\x31", 1);
	}
	fx_appendChecksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_doConnVer(struct usb_dev_handle *usb_handle) {
	char *buffer = calloc(6, sizeof(char));
	if (buffer == NULL) {"ERR: fx_doConnVer(): allocation failed.\n";}
	fx_sendVerify(usb_handle, buffer, "00");	// sends connver for start of communication
	ReadUSB(usb_handle, buffer, 6);
	if (fx_getPacketType(buffer) == T_POSITIVE) {// assuming the positive response is 'plain'
		free(buffer);
		return 0;
	} else {
		free(buffer);
		return 1;	// failure
	}
}

int fx_sendTerminate(struct usb_dev_handle *usb_handle, char *buffer) {
	/* Type: 0x18
	 * ST: 01 */
	memcpy(buffer, "\x18\x30\x31\x30\x36\x46", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendPositive(struct usb_dev_handle *usb_handle, char *buffer, char type) {
	int i;
	char sum;

	/* Type: 0x06
	 * ST: given as argument */
	memcpy(buffer, "\x06\x30\x30\x30", 4);
	if (type == POSITIVE_OVERWRITE || type == POSITIVE_SYSINFO) {
		memcpy(buffer+2, &type, 1);
	}

	fx_appendChecksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_getPacketType(char *buffer) {
	if (buffer != NULL && buffer[0] != 0x00)
		return buffer[0];
	else
		return -1;
}

int fx_sendNegative(struct usb_dev_handle *usb_handle, char *buffer, char type) {
	int i;
	char sum;

	/* Type 0x05
	 * ST: given as argument */
	memcpy(buffer, "\x05\x30\x30\x30", 4);
	if (type >= 0x30 || type <= 0x36) {	// from '0' and '6'
		memcpy(buffer+2, &type, 1);
	}
	fx_appendChecksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendChange_Direction(struct usb_dev_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x03\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_sendFlash_Capacity_Request(struct usb_dev_handle *usb_handle, char *buffer, char *device) {
	short int len = strlen(device);
	memcpy(buffer, "\x01\x34\x42\x31", 4);
	sprintf(buffer+4, "%04X", 24+len);
	memcpy(buffer+8, "000000000000", 12);
	sprintf(buffer+20, "00000000%02X00", len);
	memcpy(buffer+32, device, len);

	fx_appendChecksum(buffer, 32+len);

	return WriteUSB(usb_handle, buffer, 34+len);
}

int fx_getFlashCapacity(struct usb_dev_handle *usb_handle, char *device) {
	int freeSize = 0;
	char * buffer = calloc(40,sizeof(char));
		if (buffer == NULL) { printf("ERR: fx_getFlashCapacity: alloc error\n"); return -1; }

	fx_sendFlash_Capacity_Request(usb_handle, buffer, device);
	ReadUSB(usb_handle, buffer, 6);
		if (fx_getPacketType(buffer) != T_POSITIVE) { printf("ERR: fx_getFlashCapacity: no proper response\n"); return -1; }

	fx_sendChange_Direction(usb_handle, buffer);
	ReadUSB(usb_handle, buffer, 0x26);
		if (fx_getPacketType(buffer) != T_COMMAND) { printf("ERR: fx_getFlashCapacity: no returned command\n"); return -1; }

	char *tmp = calloc(9, sizeof(char));
		if (tmp == NULL) { printf("ERR: fx_getFlashCapacity: alloc error\n"); return -1; }

	memcpy(tmp, buffer+12, 8);	// got value
	freeSize = strtol(tmp, NULL, 16);

	fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);
	ReadUSB(usb_handle, buffer, 6);

	free(tmp);
	free(buffer);

	return freeSize; // converts 'hex-ascii' to int
}

int fx_sendFile_to_Flash(struct usb_dev_handle *usb_handle, char *buffer, int filesize, char *filename, char *device) {
	short int fnsize = strlen(filename);
	short int devsize = strlen(device);
	memcpy(buffer, "\x01\x34\x35\x31", 4); /* T, ST, DF */
	sprintf(buffer+4, "%04X", 24+fnsize+devsize);
	memcpy(buffer+8, "0280", 4); /* OW, DT */
	sprintf(buffer+12, "%08X", filesize); /* FS   8 byte*/
	sprintf(buffer+20, "00%02X0000%02X00", fnsize, devsize); /* DS1 - DS6 (12b)*/
	sprintf(buffer+32, "%s%s", filename, device);

	fx_appendChecksum(buffer, 32+fnsize+devsize);

	return WriteUSB(usb_handle, buffer, 34+fnsize+devsize);
}

int fx_sendData(struct usb_dev_handle *usb_handle, char *buffer, char* subtype, int total, int number, char *data, int length) {
	memcpy(buffer, "\x02\x00\x00\x31", 4); // T, DF, leaves a hole for ST
	 memcpy(buffer+1, subtype, 2); // ST
	sprintf(buffer+4, "%04X", 8+length); // DS, 4 b
	sprintf(buffer+8, "%04X%04X", total, number);
	memcpy(buffer+16, data, length);

	fx_appendChecksum(buffer, 16+length);
	return WriteUSB(usb_handle, buffer, 18+length);
}

int fx_appendChecksum(char *buffer, int length) {
	int i;
	char sum = 0;
	for (i = 1; i < length; i++) {
		sum += *(buffer+i);
	}
	sprintf(buffer+length, "%02X", ((~sum)+1) & 0xFF);
	/* The value appears sometimes as FFFFxx, where AND'ing it with 0xFF, you
	 * get the wanted one-byte 0000xx.. */
	return 0;
}

/*
 * TODO: This function will have to take a parameter choosing whether to expect binary data or not.
 */
int fx_escapeBytes(char *source, char *dest, int length) {
	int i = 0, j = 0;
	while(i < length) {
		// replacement filter, disabled for now
		/*switch (source[i]) {
			case 0x0A:	source[i] = 0x0D;	// replace LF with CR
			default:	break;
		}*/

		if (source[i] < 0x20) {
			dest[j] = 0x5C;
			dest[j+1] = 0x20+source[i];
			j++;
			goto done;
		}

		if (source[i] == 0x5C) {
			dest[j] = 0x5C; dest[j+1] = 0x5C; j++; // I might have to use 0x7C here
			goto done;
		}
		// default action
		*(dest+j) = *(source+i);

	done:
		j++;
		i++;
	}
	return j; // length of destination (new) buffer
}
