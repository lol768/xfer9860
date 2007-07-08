/*******************************************************************************
	xfer9860 - a Casio fx-9860G (SD) communication utility
	Copyright (C) 2007
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
#include <stdlib.h>
#include <string.h>

#include "Casio9860.h"

int readPacket(struct usb_dev_handle *usb_handle, char *buffer) {
	// This basically works by receiving until all data has been read, because
	// the calculator sometimes writes its packets in chunks. This makes downloading
	// quite slow, as we have to have a minimum read timeout to make sure packets are
	// read correctly, but at the same time allows short waiting if no data is ready.
	int pos = 0, read = 0;
	do {
		read = ReadUSB(usb_handle, buffer+pos, ((MAX_DATA_PAYLOAD*2)+18)-pos);
		pos += read;
	} while (read > 0);

	return pos;
}

int downloadFile(char* sourceFileName, char* destFileName, int throttleSetting) {
	int fileSize = 0;
	FILE *destFile = fopen(destFileName, "w");
	if (destFile == NULL) { printf("[E] Cannot open file for writing.\n"); goto exit; }

	char* buffer =	calloc((MAX_DATA_PAYLOAD*2)+18, sizeof(char));
	char* fData =	calloc((MAX_DATA_PAYLOAD*2)+18, sizeof(char));

	if (fData == NULL || buffer == NULL) { printf("[E] Error allocating memory\n"); goto exit; }

	printf("[>] Setting up USB connection.. ");
	struct usb_dev_handle *usb_handle;
	usb_handle = fx_getDeviceHandle();	// initiates usb system
	if ((int)usb_handle == -1 || usb_handle == NULL) {
		printf(	"\n[E] A listening device could not be found.\n"
		      	"    Make sure it is receiving; press [ON], [MENU], [sin], [F2]\n");
		goto exit;
	}
	if (fx_initDevice(usb_handle) < 0) {	// does calculator-specific setup
		printf("\n[E] Error initializing device.\n"); goto exit;
	}
	printf("Connected!\n");

	printf("[>] Verifying device.. ");
	if (fx_doConnVer(usb_handle) != 0) { printf("Failed.\n"); goto exit; }
	else { printf("Done!\n"); }

	fx_sendFlashFileTransmissionRequest(usb_handle, buffer, sourceFileName, "fls0");
	ReadUSB(usb_handle, buffer, 6);
	if (fx_getPacketType(buffer) == T_NEGATIVE) { printf("[E] The file could not be found on the device.\n"); goto exit;}

	fx_sendChange_Direction(usb_handle, buffer);
	ReadUSB(usb_handle, buffer, 0x40);
	if (fx_getPacketType( buffer) != T_COMMAND) { printf("[E] Did not receive expected transmission.\n"); goto exit; }

	// Read filesize from offset 12, 8 bytes
	fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);

	fileSize = fx_asciiHexToInt(buffer+OFF_FS, 8);
	printf("\n[I] Got file transmission request, filesize is %i bytes.\n", fileSize);

	// main receive loop
	int i = 0;
	int dataLength = 0, unescapedSize = 0, totalCount = 0, packetCount = 0;
	int packetLength = 0;
	int dataWritten = 0;
	printf("[");
	while(1) { // receive loop

		packetLength = readPacket(usb_handle, buffer);
		// need checks on packet
		dataLength = fx_asciiHexToInt(buffer+OFF_DS, 4);
		dataLength -= 8; // subtract the space for PN and TP to get length of DD field

		unescapedSize = fx_unescapeBytes(buffer+OFF_DD, fData, dataLength); // unescaped data to fData

		dataWritten += fwrite(fData, 1, unescapedSize, destFile);
		fx_sendPositive(usb_handle, buffer, POSITIVE_NORMAL);
		totalCount = fx_asciiHexToInt( buffer+OFF_TP, 4);
		packetCount = fx_asciiHexToInt( buffer+OFF_PN, 4);
		if (i % 4 == 0) { printf("#"); fflush(stdout); } // indicates every 1kB
		i++;
		if (packetCount == totalCount) { // if this was last packet
			break;
		}
		usleep(1000*throttleSetting);
	}

	ReadUSB(usb_handle, buffer, 6);

	printf("]\n[I] File download completed.\n\n"); fflush(stdout);
	fx_sendComplete(usb_handle, buffer);
exit:
	free(fData);
	free(buffer);
	fclose(destFile);


	return 0;
}
