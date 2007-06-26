/***************************************************************************
 *   xfer9860 - Transfer files between a Casio fx-9860G and computer	   *
 *									   *
 *   Copyright (C) 2007							   *
 *	Andreas Bertheussen <andreasmarcel@gmail.com>			   *
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

void debug(int input, char* array, int len){
#ifdef __SNOOP__
	unsigned char temp;
	int i, j, line = 0;

	if (input) { fprintf(stderr, "<< "); }
	else { fprintf(stderr, ">> "); }
	
	for (i = 0 ; i < len ; i++){
		temp = (unsigned char) array[i];
		
		if (i % LEN_LINE == 0 && i != 0){
			fprintf(stderr, "\t");
			for (j = line; j < line + LEN_LINE; j++){
				char u = (unsigned char) array[j];
				if (u > 31) { fprintf(stderr, "%c", u); }
				else { fprintf(stderr, "."); }
			}

			line = i;
			fprintf(stderr,"\n");
			if (input) { fprintf(stderr, "<< "); }
			else { fprintf(stderr, ">> "); }
		}
		
		fprintf(stderr,"%02X ", temp);
	}

	if (i % LEN_LINE != 0)	
		for (j = 0 ; j < (int)(LEN_LINE-(i % LEN_LINE)); j++)
			fprintf(stderr,"   ");
	
	fprintf(stderr, "\t");
	for (j = line; j < len; j++){
		temp = (short unsigned int) array[j];
		if (temp > 31)
			fprintf(stderr, "%c", temp);
		else
			fprintf(stderr, ".");			
	}

	fprintf(stderr,"\n\n");
#endif //__SNOOP__
}

int ReadUSB(struct usb_dev_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	ret = usb_bulk_read(usb_handle, 0x82, buffer, length, USB_READ_TIMEOUT);
	if (ret < 0) { printf("Could not read, ERROR: %i", ret); }
	debug(1, buffer, ret);
	return ret;
}

int WriteUSB(struct usb_dev_handle *usb_handle, char *buffer, int length) {
	int ret = 0;
	ret = usb_bulk_write(usb_handle, 0x1, buffer, length, USB_WRITE_TIMEOUT);
	if (ret < 0) { printf("Could not write, ERROR: %i", ret); }
	debug(0, buffer, ret);
	return ret;
}
