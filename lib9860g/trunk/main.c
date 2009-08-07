#include <usb.h>
#include <stdlib.h>
#include <stdio.h>
#include "packetio.h"
#include "usbio.h"
#include "log.h"

struct usb_dev_handle *deviceHandle;
struct Packet_t myPacket;

int main() {
	char buffer[0x400];
	
	struct Packet_t *packet;
	int ret, interface = 0;
	struct usb_device *device;
	struct usb_dev_handle *deviceHandle;
	fx_debugLevel = 3;
	
	
	device = fx_findDevice();
	
	if (device == NULL) {
		FX_LOG(1, "%s: Device not found on bus(es)", __func__)
		return (int)NULL;
	}

	deviceHandle = usb_open(device);
	if (deviceHandle == NULL) {
		FX_LOG(1, "%s: usb_open() failed: %s", __func__, usb_strerror())
		return (int)NULL;
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
	printf("Connected! Acking.\n");
	
	usb_resetep(deviceHandle, 0);
	
	fx_initializePacket(packet, CheckPacket, 0);
	fx_send(deviceHandle, packet);
	usb_set_debug(255);
	fx_read(deviceHandle, buffer, 7);	/* just assume it is an ack */
	
	fx_initializePacket(packet, CommandPacket, CMD_REQINFO);
	fx_send(deviceHandle, packet);

	fx_read(deviceHandle, buffer, 6);
	
	/*fx_initializePacket(packet, TerminatePacket, 0);
	fx_send(deviceHandle, packet);*/
	
fail_close:
	fflush(stdout);
	fflush(stderr);
	fx_releaseDevice(deviceHandle, 0);
	return 0;
}
