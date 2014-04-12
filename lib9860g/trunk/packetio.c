/* FIXME: change to the logging interface */   /* Required for the logging messages */
#include <stdio.h>
#include <string.h>
#include <usb.h> 

#include "packetio.h"
#include "log.h"
#include "usbio.h"
#include "util.h"

enum PacketType_e fx_defaultPacketType = CommandPacket;
int fx_defaultPacketSubtype = 0;

/* The following structure is used to name the type when printing a packet struct for debug purposes */
struct PacketTypeSet_t packetTypeSet[8] = {
	{DefaultPacket,		"DefaultPacket (INVALID)"},
	{CommandPacket,		"CommandPacket"},
	{DataPacket,		"DataPacket"},
	{RoleswapPacket,	"RoleswapPacket"},
	{CheckPacket,		"CheckPacket"},
	{AckPacket,			"AckPacket"},
	{ErrorPacket,		"ErrorPacket"},
	{TerminatePacket,	"TerminatePacket"}
};


int fx_isTypeValid(enum PacketType_e type) {
	switch (type) {
		case CommandPacket:
		case AckPacket:
		case DataPacket:
		case RoleswapPacket:
		case CheckPacket:
		case ErrorPacket:
		case TerminatePacket: return 1;	/* Type is valid */
		
		case DefaultPacket: /* This is INVALID type because it is not a real type */
		default:
			FX_LOG(1, "%s: Unrecognized packet type.", __func__)
			return 0;
	}
}

/* checks both the type and subtype fields */
int fx_isTypeSubtypeValid(enum PacketType_e type, char subtype) {
	if (!fx_isTypeValid(type)) { FX_LOG(1, "%s: Unrecognized packet type (%X) provided.", __func__, type); return 0; }
	switch(type) {
		case DefaultPacket:
			FX_LOG(1, "%s: The DefaultPacket type is not a real packet type and should not occur in packets.", __func__)
			return 0;
		case CommandPacket:
		case DataPacket:
			if ((subtype > 3 && subtype < 0x20) || subtype > 0x57 || (subtype > 0x33 && subtype < 0x40)) {
				FX_LOG(1, "%s: Subtype of cmd or data packet (%X) is not valid", __func__, subtype)
				return 0;
			}
			break;
		case RoleswapPacket:
			if (subtype != 0) {
				FX_LOG(1, "%s: Subtype of roleswap packet (%X) is not valid", __func__, subtype)
				return 0;
			}
			break;
		case ErrorPacket:
			if (subtype > 5) {
				FX_LOG(1, "%s: Subtype of error packet (%X) is not valid", __func__, subtype)
				return 0;
			}
			break;
		case AckPacket:
			if (subtype > 2) {
				FX_LOG(1, "%s: Subtype of ack packet (%X) is not valid", __func__, subtype)
				return 0;
			}
			break;
		case CheckPacket:
			if (subtype > 1) {
				FX_LOG(1, "%s: Subtype of check packet (%X) is not valid", __func__, subtype)
				return 0;
			}
			break;
		case TerminatePacket:
			if (subtype > 3) {
				FX_LOG(1, "%s: Subtype of check packet (%X) is not valid", __func__, subtype)
				return 0;
			}
			break;
	}
	return 1;	 /* is not invalid */
}

/* Frees allocated memory in packet structure */
int fx_initializePacket(struct Packet_t *packet, enum PacketType_e type, char subtype) {
/* FIXME: enable extended ack packets */

	/* de-extend it, freeing potentially allocated data */
	fx_contractPacket(packet);
	
	/* set subtype and type of fresh packet */
	if (type == DefaultPacket) {
		packet->type = fx_defaultPacketType;
		packet->subtype = 0; /* TODO: implement default subtypes? */
	} else if (fx_isTypeSubtypeValid(type, subtype)) {
		packet->type = type;
		packet->subtype = subtype;
	} else {
		FX_LOG(1, "%s: Provided invalid packet type and subtype combination", __func__)
		return -1;
	}

	packet->d.ch = NULL; /* Could use d.dh because they are unioned */
	return 0;
}

int fx_setDefaultTypeSubtype(enum PacketType_e type, char subtype) {
	if (!fx_isTypeSubtypeValid(type, subtype)) {
		FX_LOG(1, "%s: Cannot set default types to invalid values.", __func__)
		return -1;
	}
	fx_defaultPacketType = type;
	fx_defaultPacketSubtype = subtype;
	return 0;
}

int fx_contractPacket(struct Packet_t *packet) {
	int i;
	if (packet->extended == true) {
		/* free data allocated in extension */
		switch(packet->type) {
			case CommandPacket:
				for (i = 0; i < 6; i++) { /* Free argument/parameter data */
					if (packet->d.ch->d[i] != NULL)
						free(packet->d.ch->d[i]);
				}
				break;
			case DataPacket:
				if (packet->d.dh->data != NULL) /* Free attached bulk data */
					free(packet->d.dh->data);
				break;
			default: break;
		}
		free(packet->d.ch); /* free extension header */
		packet->extended = false;
	}
	return 0;
}

int fx_extendPacket(struct Packet_t *packet) {
	if (packet->extended == true) {
		FX_LOG(1, "%s: Note: Packet was extended. Continuing.", __func__)
	}
	fx_contractPacket(packet); /* get rid of any previously extensions */
	switch(packet->type) {
		case CommandPacket:
			packet->extended = true;
			packet->d.ch = calloc(1, sizeof(struct CommandHeader_t));
			break;
		case DataPacket:
			packet->extended = true;
			packet->d.dh = calloc(1, sizeof(struct DataHeader_t));
			break;
		default:
			FX_LOG(1, "%s: Packet not of extendable type.", __func__)
			return -1;
	}
	return 0;
}

int fx_attachDataPayload(struct Packet_t *packet, char *data, int size) {
	if (size > 256) { FX_LOG(1, "%s: Payload size (%i) is out of spec", __func__, size) return -1; }
	if (data == NULL) { FX_LOG(1, "%s: Provided payload data pointer is NULL", __func__) return -1; }
	if (packet->type != DataPacket || packet->extended == false || packet->d.dh == NULL) {
		FX_LOG(1, "%s: Cannot attach to this packet. type=0x%x, extended=0x%x, dh=%p", __func__, packet->type, packet->extended, packet->d.dh)
		return -1;
	}
	free(packet->d.dh->data); /* Free potentially old allocated data */
	packet->d.dh->data = malloc(size); /* Allocate */
	memcpy(packet->d.dh->data, data, size); /* And copy it */
	return 0;
}

/* Takes a Packet_t and makes it ready for transfer */
int fx_encodePacket(struct Packet_t *packet, char *dest, int sizeLimit) {
	int currentSize = 4, i, ds, cs;	/* ds is used in extended packets */
	char *pBuffer;
	if (fx_validatePacket(packet) < 0) {
		FX_LOG(1, "%s: fx_validatePacket() failed.", __func__)
		return -1;
	}
	pBuffer = malloc(530);	/* TODO: must make this more flexible */
	memset(pBuffer, 0, 530);
	pBuffer[0] = packet->type; /* Write type (can be used directly). We know it is valid. */
	intToAschex(packet->subtype, pBuffer+1, 2); /* Write subtype */
	
	if (packet->extended == false) { /* non-extended packets are the easiest */
		pBuffer[3] = '0'; /* EX */
		cs = fx_calculateChecksum(pBuffer, currentSize);
		currentSize = 6;
		intToAschex(cs, pBuffer+4, 2); /* End (short) packet with CS */
		goto done;
	}
	
	//realloc(pBuffer, 530);
	/* Packet is extended */
	pBuffer[3] = '1';
	if (packet->type == CommandPacket) {
		ds = 0; /* Start filling D field */
		intToAschex(packet->d.ch->overwrite, pBuffer+8, 2); ds+=2;
		intToAschex(packet->d.ch->datatype, pBuffer+10, 2); ds+=2;
		intToAschex(packet->d.ch->filesize, pBuffer+12, 8); ds+=8;
		/* Fields done so far:  T:1, ST:2, EX:1, DS:4, OW:2, DT:2, FS:8
		 * Total size is 20. */
		for (i = 0; i < 6; i++) {
			intToAschex(packet->d.ch->sd[i], pBuffer+8+ds, 2); ds+=2;
		}
		/* Total size is 32 */
		
		for (i = 0; i < 6; i++) {
			/* Copy referenced data */
			memcpy(pBuffer+8+ds, packet->d.ch->d[i], packet->d.ch->sd[i]);
			ds+=packet->d.ch->sd[i];
		}
		intToAschex(ds, pBuffer+4, 4); /* calculated DS field */
		currentSize = 8+ds;
		cs = fx_calculateChecksum(pBuffer, currentSize);
		intToAschex(cs, pBuffer+currentSize, 2); currentSize+=2;
		goto done;	/* doing this explicitly is not needed */
	} else if (packet->type == DataPacket) {
		ds = 0;
		intToAschex(packet->d.dh->total, pBuffer+8, 4); ds+=4;
		intToAschex(packet->d.dh->current, pBuffer+12, 4); ds+=4;
		
		if (packet->d.dh->datasize > 256) {
			/* Enforce maximum size of packet D field */
			FX_LOG(1, "%s: Data attachment too big (%i)", __func__, packet->d.dh->datasize)
			free(pBuffer);
			return -1;
		}
		
		ds += fx_escapeData(packet->d.dh->data, pBuffer+16, packet->d.dh->datasize);
		intToAschex(ds, pBuffer+4,4); /* calculated DS field */
		currentSize = 8+ds;
		cs = fx_calculateChecksum(pBuffer, currentSize);
		intToAschex(cs, pBuffer+currentSize, 2); currentSize+=2;
	} else if (packet->type == AckPacket) {
		FX_LOG(1, "%s: Handling of extended acks is not implemented.", __func__) /* TODO FIXME */
		free(pBuffer);
		return -1;
	
	}

done:
	fx_printData(pBuffer, currentSize, 16, 9); /* last arg chosen not to be 0 or 1 */
	
	if (currentSize <= sizeLimit) {
		memcpy(dest, pBuffer, currentSize);
		free(pBuffer);
		return currentSize;
	} else {
		free(pBuffer);
		FX_LOG(1, "%s: Encoded packet (%i byte) could not fit in provided buffer (%i byte)", __func__, currentSize, sizeLimit)
		return -1;
	}
}

int fx_send(struct usb_dev_handle* usb_handle, struct Packet_t *packet) {
	char *data;
	int size;
	data = malloc(530);
	if (data == NULL) {
		FX_LOG(1, "%s: Allocation failed.", __func__)
		return -1;
	}
	
	size = fx_encodePacket(packet, data, 530);
	if (size < 0) {
		FX_LOG(1, "%s: encoding packet failed.", __func__)
		return -1;
	}
	
	if (size == 0) {
		/* should not happen, but whatever */
		FX_LOG(1, "%s: Warn: encoding returned zero data, without errors", __func__)
		return 0;
	}
	
	/* we now have a packet ready for shipping. we reuse the size variable. */
	size = fx_write(usb_handle, data, size);
	
	return size;
}

int fx_receive(struct usb_dev_handle* usb_handle, struct Packet_t *packet) {
	return 0;
}

int fx_validatePacket(struct Packet_t *packet) {
	if (!fx_isTypeSubtypeValid(packet->type, packet->subtype)) {
		FX_LOG(1, "%s: Invalid type or subtype", __func__)
		return -1;
	}
	
	register char subtype = packet->subtype; /* local copy */
	
	/* VERIFY COMMAND PACKET */
	if (packet->type == CommandPacket) {
	/* TODO: move to usage of fx_isTypeSubtypeValid */
		if (!fx_isTypeSubtypeValid(packet->type, packet->subtype)) {
			FX_LOG(1, "%s: Subtype or type is invalid.", __func__)
			return -2;
		}
		
		if (packet->extended == false) { return 0; } /* nothing more to check */
		
		/* Packet is supposed to be extended from here on.. */
		if (packet->d.ch == NULL) {
			FX_LOG(1, "%s: Expected data pointer for cmd packet subhdr, NULL found", __func__)
			return -3;
		}
		if (packet->d.ch->overwrite > 2) {  /* Should be 0..2 */
			FX_LOG(1, "%s: d.ch->overwrite is out of range: %X", __func__, packet->d.ch->overwrite);
			return -4;
		}
		/* FIXME: validate datatype when we know its workings */
		/* FIXME: validate filesize depending on command ? */
		
		int i;
		for (i=0; i < 6; i++) {
			/* cycle through the parameters */
			if (packet->d.ch->sd[i] > 0 && packet->d.ch->d[i] == NULL) {
				FX_LOG(1, "%s: d.ch->ds[%i] is nonzero, but null found at d[%i]",__func__, i+1, i+1);
				return 0;
			}
		}
	} else if (packet->type == DataPacket) { /* VERIFY DATA PACKET */
		/* FIXME: narrow check to only accept subtypes for which a data packet stream is expected */
		if (subtype < 0x20 || subtype > 0x57 || (subtype > 0x33 && subtype < 0x40)) {
			FX_LOG(1, "%s: Unrecognized packet subtype (%X) for data packet", __func__, subtype)
			return -2;
		}
		if (packet->extended == false) {
			FX_LOG(1, "%s: Warning: Non-extended DATA packet! Validation OK.", __func__)
			return 0;  /* nothing more to check */
		}
		/* Packet is supposed to be extended from here on.. */
		if (packet->d.ch == NULL) {
			FX_LOG(1, "%s: Expected data pointer for data packet subhdr, NULL found", __func__)
			return -3;
		}
		if (packet->d.dh->current > packet->d.dh->total) {
			FX_LOG(1, "%s: d.dh->current (=%i) is larger than d.dh->total (=%i)", __func__, packet->d.dh->current, packet->d.dh->total)
			return -5;
		}
		if (packet->d.dh->datasize > 0 && packet->d.dh->data == NULL) {
			FX_LOG(1, "%s: d.dh->datasize indicates data, but datapointer is NULL", __func__)
			return -6;
		}
		if (packet->d.dh->datasize > 256) {
			FX_LOG(1, "%s: Data packet is obese. d.dh->datasize = %i", __func__, packet->d.dh->datasize)
			return -7;
		}
	}
		/* VERIFY ACK PACKET */
	else if (packet->type == AckPacket) {
		/* FIXME: check subtype values and compare if extended ack*/
	}
		/* VERIFY PACKETS TYPES WITHOUT AN EXTENDED FIELD */
	else if (packet->type == RoleswapPacket ||
				packet->type == CheckPacket ||
				packet->type == TerminatePacket ||
				packet->type == ErrorPacket)
	{
		/* These packets should not be extended */
		if (packet->extended == true) {
			FX_LOG(1, "%s: Packet of type %i should NOT be extended.", __func__, packet->type)
			return -1;
		}
		switch (packet->subtype) {
			case 0x00: break; /* valid in all types */
			case 0x05:
			case 0x04: if (packet->type != ErrorPacket) {
							FX_LOG(1, "%s: Invalid subtype 0x%x for type 0x%x.", __func__, packet->subtype, packet->type)
							return -7;
						} else { break; }
			case 0x03:
			case 0x02: if (packet->type != ErrorPacket && packet->type != TerminatePacket) {
							FX_LOG(1, "%s: Invalid subtype 0x%x for type 0x%x.", __func__, packet->subtype, packet->type)
							return -7;
						} else { break; }
			case 0x01: if (packet->type == RoleswapPacket) {
				FX_LOG(1, "%s: Invalid subtype 0x%x for type 0x%x.", __func__, packet->subtype, packet->type)
				return -7;
			} else { break; } /* All the other types have ST = 0x01 as an option */
		}
	}
	return 0;
}

int fx_calculateChecksum(char *packet, int length) {
	int i;
	char sum = 0;
	for (i = 1; i < length; i++) { /* skip T field */
		sum+= packet[i];
	}
	sum = (~sum)+1;
	return sum & 0xFF;
}

int fx_getEscapedDataSize(unsigned char *data, int size) {
	int i, escapedSize = size;
	for (i = 0; i < size; i++) {
		if (data[i] < 0x20) {
			escapedSize++;
		} else if (data[i] == 0x5C) {
			escapedSize++;
		}
	}
	return escapedSize;
}

int fx_escapeData(unsigned char *source, unsigned char *dest, int size) {
	int i, j = 0;
	for (i = 0; i < size; i++,j++) {
		if (source[i] < 0x20) {
			dest[j++] = 0x5C; /* prefix by the '\' character.. */
			dest[j] = 0x20 + source[i]; /* and offset by 0x20 */
			continue;
		}
		if (source[i] == 0x5C) {
			dest[j++] = 0x5C;
			dest[j] = 0x5C;	/* TODO: should this be 0x7C instead? */
			continue;
		}
		dest[j] = source[i];
	}
	return j; /* The escaped size */
}

int fx_clearCommandParameter(struct Packet_t *packet, unsigned int paramno) {
	unsigned int paramidx = paramno-1;
	if (packet->type != CommandPacket) {
		FX_LOG(1, "%s: Tried to add command parameter to non-commandpacket.", __func__)
		return -1;
	}
	if (paramno < 1 || paramno > 6) {
		FX_LOG(1, "%s: Parameter number (%i) is out of range 1..6", __func__, paramno)
		return -1;
	}
	free(packet->d.ch->d[paramidx]);
	packet->d.ch->sd[paramidx] = 0;
	return 0;
}

/*  */
int fx_setCommandParameter(struct Packet_t *packet, unsigned int paramno, char *data, unsigned int size) {
	int paramidx = paramno-1; /* used as index in parameter tables of packet structure */
	if (packet->type != CommandPacket) {
		FX_LOG(1, "%s: Tried to add command parameter to non-commandpacket.", __func__)
		return -1;
	}
	if (paramno < 1 || paramno > 6) {
		FX_LOG(1, "%s: Parameter number (%i) is out of range 1..6", __func__, paramno)
		return -1;
	}
	if (size > 12) {
		FX_LOG(1, "%s: Note: parameter size (%i) is larger than usual. Continuing.", __func__, size)
	}
	if (data == NULL || size == 0) {
		FX_LOG(1, "%s: Zero size, or NULL data pointer provided. size=%i, data=%p", __func__, size, data)
		return -1;
	}
	
	if (packet->d.ch->d[paramidx] != NULL  ||  packet->d.ch->sd[paramidx] != 0) {
		FX_LOG(1, "%s: Parameter D%i already exists. SD%i=%i. Will attempt to free it.",
				__func__, paramno, paramno, packet->d.ch->sd[paramidx])
		fx_clearCommandParameter(packet, paramno);
	}
	
	packet->d.ch->d[paramidx] = malloc(size); /* Allocate and 'register' the memory size */
	packet->d.ch->sd[paramidx] = size;
	memcpy(packet->d.ch->d[paramidx], data, size); /* Copy the provided data */
	
	return 0;
}

int fx_printPacketStruct(struct Packet_t *packet) {
	int i, invalidTypeFlag = 0;
	
	fprintf(stderr, "Begin packet structure data:\n");
	if (fx_isTypeValid(packet->type) || packet->type == DefaultPacket) {
		for (i = 0; i < (sizeof(packetTypeSet) / sizeof(packetTypeSet[0])); i++) {
			if (packet->type == packetTypeSet[i].type) {
				fprintf(stderr, "\tType: \t\t0x%x\t(%s)\n", packet->type, packetTypeSet[i].name);
			}
		}
	} else if (!fx_isTypeValid(packet->type) || packet->type == DefaultPacket) {
		fprintf(stderr, "\tType: \t0x%x\t(INVALID)\n", packet->type);	invalidTypeFlag = 1;
	}
	fprintf(stderr, "\tSubtype: \t0x%x\n", packet->subtype & 0xFF);
	fprintf(stderr, "\tExtended: \t0x%x\t(%s)\n", packet->extended, packet->extended ? "yes" :"no" ); /* Too lax check? */
	
	/* If no extended field is expected, we finish */
	switch (packet->type) {
		case AckPacket: /* TODO:implement check of AckPacket extended structure. */
		case RoleswapPacket:
		case ErrorPacket:
		case CheckPacket:
		case TerminatePacket: goto end;
		default: break;
	}
	
	/* Packet is now either Command or data packet. */
	if (invalidTypeFlag) {
		FX_LOG(1, "%s: Invalid type makes it impossible to continue.", __func__)
		goto err;
	}
	
	if (packet->type == CommandPacket) {
		if (packet->extended == false) { goto end; }
		if (packet->d.ch == NULL) { FX_LOG(1, "%s: Command acket should be extended, d.ch=NULL", __func__) return -1; }
		fprintf(stderr, "\t  Overwrite:\t0x%x\n", packet->d.ch->overwrite);
		fprintf(stderr, "\t  Datatype:\t0x%x\n", packet->d.ch->datatype);
		fprintf(stderr, "\t  Filesize:\t0x%x\t(%i)\n", packet->d.ch->filesize, packet->d.ch->filesize);
		
		goto end;
	} else if (packet->type == DataPacket) {
		if (packet->extended == false) { FX_LOG(1, "%s: Warning: non-extended data packet", __func__) goto end; }
		if (packet->d.dh == NULL) { FX_LOG(1, "%s: Data packet should be extended, d.ch=NULL", __func__) return -1; }
		fprintf(stderr, "\t  Total:\t0x%x\t(%i)\n", packet->d.dh->total, packet->d.dh->total);
		fprintf(stderr, "\t  Current:\t0x%x\t(%i)\n", packet->d.dh->current, packet->d.dh->current);
		if (packet->d.dh->datasize == 0) {
			fprintf(stderr, "\t  No data attached! (0 bytes indicated)\n");
			goto end;
		}
		fprintf(stderr, "\t  Attached data, 0x%x (%i) bytes, unescaped):\n", packet->d.dh->datasize, packet->d.dh->datasize);
		if (fx_printData(packet->d.dh->data, packet->d.dh->datasize, 16, -1) < 0) {
			FX_LOG(1, "%s: Attached data ptr in data packet was invalid", __func__)
			goto err;
		}
	}
end:
	fprintf(stderr, "End packet structure data.\n");
	return 0;
err:
	fprintf(stderr, "Ending printout of apparently bad packet!\n");
	return -1;
}
