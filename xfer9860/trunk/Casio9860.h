/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
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

#ifndef CASIO_9860_H
#define CASIO_9860_H

#include <usb.h>

#define C9860_VENDOR_ID		0x07CF
#define C9860_PRODUCT_ID	0x6101

//Types (subtypes actually) used for specifying what packet does.
#define POSITIVE_NORMAL		'0'
#define POSITIVE_OVERWRITE	'1'
#define POSITIVE_SYSINFO	'2'

#define NEGATIVE_NORMAL		'0'
#define NEGATIVE_RETRANSMIT	'1'
#define NEGATIVE_FILEEXISTS	'2'
#define NEGATIVE_NOOVERWRITE	'3'
#define NEGATIVE_OVERWRITEERR	'4'
#define NEGATIVE_MEMFULL	'5'
#define NEGATIVE_IDENTIFY	'6'

#define T_POSITIVE	0x06
#define T_NEGATIVE	0x15
#define T_COMMAND	0x01
#define T_DATA		0x02
#define T_CHANGE	0x03
#define T_VERIFY	0x05

#define ST_FILE_TO_FLASH	"\x34\x35"

#define MAX_DATA_PAYLOAD 256

struct usb_dev_handle *fx_getDeviceHandle();
int fx_initDevice(struct usb_dev_handle *usb_handle);

struct usb_device *device_init(void);

// routine functions
int fx_doConnVer(struct usb_dev_handle*);
int fx_getFlashCapacity(struct usb_dev_handle*, char*);
int fx_getMCSCapacity(struct usb_dev_handle*);
// packet functions
int fx_sendComplete(struct usb_dev_handle*, char*);
int fx_sendVerify(struct usb_dev_handle*, char*, char*);
int fx_sendTerminate(struct usb_dev_handle*, char*);
int fx_sendPositive(struct usb_dev_handle*, char*, char);
int fx_sendNegative(struct usb_dev_handle*, char*, char);
int fx_sendChange_Direction(struct usb_dev_handle*, char*);
int fx_sendFlash_Capacity_Request(struct usb_dev_handle*, char*, char*);
int fx_sendFlashCollectGarbage(struct usb_dev_handle*, char*, char*);
int fx_sendMCSCapacityRequest(struct usb_dev_handle*, char*);
int fx_sendFile_to_Flash(struct usb_dev_handle*, char *, int, char *, char *);
int fx_sendData(struct usb_dev_handle*, char*, char*, int, int, char*, int);

int fx_getPacketType(char*);

// Service functions
int fx_append_Checksum(char*, int);
int fx_escapeBytes(char*, char*, int);
#endif
