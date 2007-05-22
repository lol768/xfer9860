/***************************************************************************
 *   xfer9860 - Transfer files between a Casio fx-9860G and computer	   *
 *									   *
 *   Copyright (C) 2007							   *
 *	Manuel Naranjo <naranjo.manuel@gmail.com>			   *
 *	Andreas Bertheussen <andreasmarcel@gmail.com>			   *
 *   Copyright (C) 2004							   *
 *	Greg Kroah-Hartman <greg@kroah.com>				   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <usb.h>
#include <string.h>
#include "Casio9860.h"

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

int init_9860(struct usb_dev_handle *usb_handle) {
	int retval;
	char* buffer;
	buffer = calloc(0x29, sizeof(char));

	retval = usb_control_msg(usb_handle, 0x80, 0x6, 0x100, 0, buffer, 0x12, 200);
	debug(1, buffer, retval);
	
	if (retval < 0) {
		fprintf(stderr, "Unable to send first message\n");
		goto exit;
	}

	retval = usb_control_msg(usb_handle, 0x80, 0x6, 0x200, 0, buffer, 0x29, 250);
	debug(1, buffer, retval);
	
	if (retval < 0) {
		fprintf(stderr, "Unable to send second message\n");
		goto exit;
	}

	retval = usb_control_msg(usb_handle, 0x41, 0x1, 0x0, 0, buffer, 0x0, 250);
	debug(1, buffer, retval);
	
	if (retval < 0) {
		fprintf(stderr, "Unable to send third message\n");
		goto exit;
	}
exit:
	free(buffer);
	return retval;
}

int fx_Send_Complete(struct usb_dev_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x01\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_Send_Verify(struct usb_dev_handle *usb_handle, char *buffer) {
	/* Type: 0x05
	 * ST: 00 */
	memcpy(buffer, "\x05\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_Send_Terminate(struct usb_dev_handle *usb_handle, char *buffer) {
	/* Type: 0x18
	 * ST: 01 */
	memcpy(buffer, "\x18\x30\x31\x30\x36\x46", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_Send_Positive(struct usb_dev_handle *usb_handle, char *buffer, char type) {
	int i;
	char sum;
	
	/* Type: 0x06
	 * ST: given as argument */
	memcpy(buffer, "\x06\x30\x30\x30", 4);
	if (type == POSITIVE_OVERWRITE || type == POSITIVE_SYSINFO) {
		memcpy(buffer+2, &type, 1);
	}

	fx_Append_Checksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_Send_Negative(struct usb_dev_handle *usb_handle, char *buffer, char type) {
	int i;
	char sum;
	
	/* Type 0x05
	 * ST: given as argument */
	memcpy(buffer, "\x05\x30\x30\x30", 4);
	if (type >= 0x30 || type <= 0x36) {	// from '0' and '6'
		memcpy(buffer+2, &type, 1);
	}
	fx_Append_Checksum(buffer, 4);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_Send_Change_Direction(struct usb_dev_handle *usb_handle, char *buffer) {
	memcpy(buffer, "\x03\x30\x30\x30\x37\x30", 6);
	return WriteUSB(usb_handle, buffer, 6);
}

int fx_Send_Flash_Capacity_Request(struct usb_dev_handle *usb_handle, char *buffer, char *device) {
	short int len = strlen(device);
	memcpy(buffer, "\x01\x34\x42\x31", 4);
	sprintf(buffer+4, "%04X", 24+len);
	memcpy(buffer+8, "000000000000", 12);
	sprintf(buffer+20, "00000000%02X00", len);
	memcpy(buffer+32, device, len);
	
	fx_Append_Checksum(buffer, 32+len);
	
	return WriteUSB(usb_handle, buffer, 34+len);
}

int fx_Send_File_to_Flash(struct usb_dev_handle *usb_handle, char *buffer, int filesize, char *filename, char *device) {
	short int fnsize = strlen(filename);
	short int devsize = strlen(device);
	memcpy(buffer, "\x01\x34\x35\x31", 4); /* T, ST, DF */
	sprintf(buffer+4, "%04X", 24+fnsize+devsize);
	memcpy(buffer+8, "0080", 4); /* OW, DT */
	sprintf(buffer+12, "%08X", filesize); /* FS   8 byte*/
	sprintf(buffer+20, "00%02X0000%02X00", fnsize, devsize); /* DS1 - DS6 (12b)*/
	sprintf(buffer+32, "%s%s", filename, device);
	
	fx_Append_Checksum(buffer, 32+fnsize+devsize);
	
	return WriteUSB(usb_handle, buffer, 34+fnsize+devsize);
}

int fx_Send_Data(struct usb_dev_handle *usb_handle, char *buffer, char* subtype, int total, int number, char *data, int length) {
	memcpy(buffer, "\x02\x00\x00\x31", 4); // T, DF, leaves a hole for ST
	 memcpy(buffer+1, subtype, 2); // ST
	sprintf(buffer+4, "%04X", 8+length); // DS, 4 b
	sprintf(buffer+8, "%04X%04X", total, number);
	memcpy(buffer+16, data, length);
	
	fx_Append_Checksum(buffer, 16+length);
	return WriteUSB(usb_handle, buffer, 18+length);
}

int fx_Append_Checksum(char *buffer, int length) {
	int i;
	char sum = 0x00;
	for (i = 1; i < length; i++) {
		sum += *(buffer+i);
	}
	sprintf(buffer+length, "%02X", ((~sum)+1) & 0xFF);
	/* The value appears sometimes as FFFFxx, where AND'ing it with 0xFF, you
	 * get the wanted one-byte 0000xx.. */
	return 0;
}

int fx_Escape_Specialbytes(char *source, char *dest, int length) {
	int i = 0, j = 0;
	while(i < length) {
		// replacement filter
		switch (source[i]) {
			case 0x0A:	source[i] = 0x0D;
			default:	break;
		}
		// note - 0x20 = 32
		if (((short int)source[i] & 0xFF) < 0x20) {
			dest[j] = 0x5C;
			dest[j+1] = 0x20+source[i];
			j++;
		} else {
			*(dest+j) = *(source+i);
		}
		j++;
		i++;
	}
	return j; // length of destination (new) buffer
}
