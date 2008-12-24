#include <usb.h>
#include <stdlib.h>
#include <stdio.h>
#include "packetio.h"
#include "usbio.h"
#include "log.h"

struct usb_dev_handle *deviceHandle;
struct Packet_t myPacket;
int main() {
	char buffer[400];
	char data[300] = "DATADATADATA";
	int i;
	for (i = 0; i < 256; i++) {
		data[i] = i;
	}
	
	fx_debugLevel = 2;
	fx_initializePacket(&myPacket, DataPacket, 0x57);
	fx_extendPacket(&myPacket);
	// fx_setCommandParameter(&myPacket, 1, "lulz", 4);
	myPacket.d.dh->current = 0x33;
	myPacket.d.dh->total = 0x39;
	myPacket.d.dh->data = data;
	myPacket.d.dh->datasize = 256;
	
	fx_printPacketStruct(&myPacket);
	fx_encodePacket(&myPacket, buffer, 400);
	
	fprintf(stderr, "FINISHED\n");
/*	
	deviceHandle = fx_claimDevice();
	if (deviceHandle == NULL) {
		return -1;
	}
	
	fx_write(deviceHandle, buffer, 6);
	fx_read(deviceHandle, buffer, 6);
	fx_
	fx_write(deviceHandle, "\x18\x30\x31\x30\x36\x46", 6);
	fx_releaseDevice(deviceHandle, 0);
*/
	return 0;
}
