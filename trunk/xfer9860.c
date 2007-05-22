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

#define __DEBUG__

#include "usbio.h"
#include "Casio9860.h"

#include <usb.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

/* Macro for nulling freed pointers */
#define FREE(p)   do { free(p); (p) = NULL; } while(0)

#define MAX_RESEND_ATTEMPTS 3
#define BUFFER_SIZE 512
#define MAX_DATA_SIZE 256

int main(int argc, char *argv[]) {
	int ret = 1, errorcount;
	char *buffer, *temp;
	long int freespace = 0;
	
	if (argc < 3) {
		printf(	"Upload a file to the fx-9860 by USB\n"
			"Usage:\t%s <source> <destination>\n", argv[0]);
		return 0;
	}
	FILE *sourcefile = fopen(argv[1], "rb");
	if (sourcefile == NULL) {
		printf("[E] Unable to open file: %s\n", argv[1]);
		return 1;
	}
	short int destlen = strlen(argv[2]);
	
	/* Filename-check, not needed for now..
	if (destlen > 12 || strncasecmp(argv[2]+(destlen-4), ".g1a", 4) != 0) {
		printf("[E] Invalid destination. Should be 8 characters with a \".G1A\" extension.\n");
		return 1;
	}
	*/
	struct stat file_status;
	stat(argv[1], &file_status);	// gets filesize

	printf(	"[I]  Found file: %s\n"
		"     Filesize:   %i byte(s)\n", argv[1], file_status.st_size);
	
	printf("[>] Opening connection...\n");
	struct usb_device *usb_dev;
	struct usb_dev_handle *usb_handle;

	usb_dev = (struct usb_device*)device_init();
	if (usb_dev == NULL) {
		fprintf(stderr,	"[E] A listening device cannot be found,\n"
				"    Make sure it is receiving; press [ON], [MENU], [sin], [F2]\n");
		return 1;
	}

	usb_handle = (struct usb_dev_handle*)usb_open(usb_dev);
	if (usb_handle == NULL) {
		fprintf(stderr, "[E] Unable to open the USB device. Exiting.\n");
		goto exit_close;
	}

	ret = usb_claim_interface(usb_handle, 0);
	if (ret < 0) {
		fprintf(stderr, "[E] Unable to claim USB Interface. Exiting.\n");
		goto exit_close;
	}

	ret = init_9860(usb_handle);
	if (ret < 0){
		fprintf(stderr, "[E] Couldn't initialize connection. Exiting.\n");
		goto exit_close;
	}
	printf("[I]  Connected.\n");

	buffer = (char*)calloc(BUFFER_SIZE, sizeof(char));
	if (buffer == NULL) {
		printf("[E] Out of system memory. Exiting.\n");
		goto exit;
	}

	int i;
	for(i = 1; i <= MAX_RESEND_ATTEMPTS; i++) {
		printf("[>] Verifying device, attempt %i...\n", i);
		fx_Send_Verify(usb_handle, buffer);
		ReadUSB(usb_handle, buffer, 6);
		if (buffer[0] == 0x06) {	/* lazy check */
			printf("[I]  Got verification response.\n");
			break;
		} else {
			if (i == MAX_RESEND_ATTEMPTS) {
				printf("[E] Did not receive verification response. Exiting.\n");
				goto exit;
			}
			sleep(1);
		}
	}
	
	printf("[>] Getting fls0 capacity information...\n");
	fx_Send_Flash_Capacity_Request(usb_handle, buffer, "fls0");
	
	ReadUSB(usb_handle, buffer, 6);
	if(buffer[0] != 0x06) {
		printf("[E] Error requesting capacity information. Exiting.\n");
		goto exit;
	}

	fx_Send_Change_Direction(usb_handle, buffer);

	/* expect free space transmission and ack */
	ReadUSB(usb_handle, buffer, 0x26);
	if (buffer[0] == 0x01) {	/* lazy check */
		temp = (char*)calloc(8, sizeof(char));	/* need checking */
		memcpy(temp, buffer+12, 8);
		freespace = strtol(temp, NULL, 16);
		printf("[I]  Free space in fls0: %d byte(s).\n", freespace);
		FREE(temp);
	} else {
		printf("[E] Did not receive acknowledgement. Exiting.\n");
		goto exit;
	}
	fx_Send_Positive(usb_handle, buffer, POSITIVE_NORMAL);
	
	if (file_status.st_size > freespace) {
		printf("[E] There is not enough room for your file (%i b). Exiting.\n", file_status.st_size);
		goto exit;
	}
	ReadUSB(usb_handle, buffer, 6);
	
	// Preparing data for transfer
	char *fdata;
	char *sdata;
	fdata = calloc(file_status.st_size, sizeof(char)); /* to contain the file data */
	sdata = calloc(file_status.st_size*2, sizeof(char)); /* to contain expanded data */
	
	if (fdata == NULL || sdata == NULL) {
		printf("[E] Error allocating memory for file\n");
	}
	fread(fdata, 1, (int)file_status.st_size, sourcefile);
	
	// Required, because the calc can't handle bytel
	int expandedbytes = fx_Escape_Specialbytes(fdata, sdata, file_status.st_size);
	int numpackets = (int)ceilf(expandedbytes/MAX_DATA_SIZE)+1;
	
	printf(	"[>] Initiating transfer of %s to fls0, %i b per packet:\n"
		"    %i bytes (%i bytes), %i packet(s).\n", argv[2], MAX_DATA_SIZE, file_status.st_size, expandedbytes, numpackets);
	fx_Send_File_to_Flash(usb_handle, buffer, file_status.st_size, argv[2], "fls0");
	ReadUSB(usb_handle, buffer, 6);
	if (!memcmp(buffer, "\x15\x30\x32\x30\x36\x45", 6)) {
		printf("[E] The file %s already exists. Exiting.\n", argv[2]);
		goto exit;
	}

	for (i = 0; i < numpackets; i++) {
	resend_data:
		if (i+1 == numpackets) { // Check if this is the last packet..
			printf("SENDING: index:%i , dataoffset:%i , length:%i\n", i+1, i*MAX_DATA_SIZE, expandedbytes-(i*MAX_DATA_SIZE));
			ret = fx_Send_Data(usb_handle, buffer, ST_FILE_TO_FLASH, numpackets, i+1, sdata+(i*MAX_DATA_SIZE), expandedbytes-(i*MAX_DATA_SIZE));
		} else {
			printf("SENDING: index:%i , dataoffset:%i , length:%i\n", i+1, i*MAX_DATA_SIZE, MAX_DATA_SIZE);
			ret = fx_Send_Data(usb_handle, buffer, ST_FILE_TO_FLASH, numpackets, i+1, sdata+(i*MAX_DATA_SIZE), MAX_DATA_SIZE);
		}
		printf("SENT %i BYTES, %i BYTES OF DATA\n", ret, ret-18);
		while(ReadUSB(usb_handle, buffer, 6) == 0) {
			if (errorcount = MAX_RESEND_ATTEMPTS) {
				errorcount = 0;
				printf("[E] Got no response, resending...\n");
				goto resend_data;
			}
			printf("\n[E] GOT ZERO BYTES, retrying..\n");
			usleep(1000*1000);
			errorcount++;
		}
		
		if (buffer[0] != 0x06) {
			printf("\nERROR, unexpected answer\n");
			goto exit;
		}
		usleep(1000*1000); // 50ms
	}
	printf("\n [I] File transfer complete.\n");
	fx_Send_Complete(usb_handle, buffer);
	
	exit:
		FREE(fdata);
		FREE(sdata);
		printf("[>] Closing connection.\n");
		fx_Send_Terminate(usb_handle, buffer);
		usb_release_interface(usb_handle, 0);
	exit_close:
		usb_close(usb_handle);
		FREE(usb_dev);
		FREE(buffer);
	
	return 0;
}
