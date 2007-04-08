/***************************************************************************
 *   Copyright (C) 2007							   *
 *		Andreas Bertheussen <andreasmarcel@gmail.com>		   *
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
#include "usbio.h"

#include <stdio.h>
#include <usb.h>

int ReadUSB(struct usb_dev_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	ret = usb_bulk_read(usb_handle, 0x82, buffer, length, USB_READ_TIMEOUT);
	if (ret < 0) { printf("Could not read, ERROR: %i", ret); }
	return ret;
}

int WriteUSB(struct usb_dev_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	ret = usb_bulk_write(usb_handle, 0x1, buffer, length, USB_WRITE_TIMEOUT);
	if (ret < 0) { printf("Could not write, ERROR: %i", ret); }
	return ret;
}
