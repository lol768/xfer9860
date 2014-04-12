#ifndef PACKETIO_H
#define PACKETIO_H

/**
 * @file packetio.h
 * @author Andreas Bertheussen
 * @brief Definition of the structures needed for packet-based communication.
 */

#define ATRUE	'1'
#define AFALSE	'0'

#define CHECK_INITIAL	0
#define CHECK_INTERMEDIATE	1	/* FIXME: find a good word*/

#define CMD_RESET		0
#define	CMD_REQINFO		1
#define	CMD_SETLINK		2
#define	CMD_MCS_ADDDIR	0x20
#define	CMD_MCS_DELETEDIR	0x21
#define	CMD_MCS_RENAMEDIR	0x22
#define	CMD_MCS_CHANGEDIR	0x23
#define	CMD_MCS_REQUESTFILE	0x24
#define	CMD_MCS_SENDFILE	0x25
#define	CMD_MCS_DELETEFILE	0x26
#define	CMD_MCS_RENAMEFILE	0x27
#define	CMD_MCS_COPYFILE	0x28
#define	CMD_MCS_REQUESTFILES	0x29
#define	CMD_MCS	0x2A				/* FIXME: do we know this ?? */
#define	CMD_MCS_REQUESTCAP	0x2B
#define	CMD_MCS_SENDCAP		0x2C
#define	CMD_MCS_REQUESTINFOS	0x2D
#define	CMD_MCS_SENDINFO	0x2E
#define	CMD_MCS_REQUESTDUMP	0x2F
#define	CMD_MCS_SENDDUMP	0x30
#define	CMD_MCS_REQUESTSETUP	0x31
#define	CMD_MCS_SENDSETUP	0x32
#define	CMD_MCS_REQUESTSETUPS	0x33

#define	CMD_FLS_DELETEDIR	0x41
#define	CMD_FLS_RENAMEDIR	0x42
#define	CMD_FLS_CHANGEDIR	0x43
#define	CMD_FLS_REQUESTFILE	0x44
#define	CMD_FLS_SENDFILE	0x45
#define	CMD_FLS_DELETEFILE	0x46
#define	CMD_FLS_RENAMEFILE	0x47
#define	CMD_FLS_COPYFILE	0x48
#define	CMD_FLS_REQUESTFILES	0x49
#define	CMD_FLS	0x4A				/* FIXME: do we know this ?? */
#define	CMD_FLS_REQUESTCAP	0x4B
#define	CMD_FLS_SENDCAP		0x4C
#define	CMD_FLS_REQUESTINFOS	0x4D
#define	CMD_FLS_SENDINFO	0x4E
#define	CMD_FLS_REQUESTDUMP	0x4F
#define	CMD_FLS_SENDDUMP		0x50
#define	CMD_FLS_OPTIMIZE	0x51


#define ACK_DEFAULT		0
#define ACK_OWOK		1 /* overwrite ok*/
#define ACK_IDENTIFY	2	/* extended ack containing special stuff. FIXME: implement below */

#define ERR_DEFAULT		0
#define ERR_RESEND		1
#define ERR_FILEEXISTS	2
#define ERR_SKIPOW		3 /* 'no' to the overwrite question */
#define ERR_OWUNAVAIL	4	/* could not overwrite. filewrite was skipped. */
#define ERR_MEMFULL		5

#define TERM_DEFAULT	0
#define TERM_USERREQ	1
#define TERM_TIMEDOUT	2
#define TERM_ABORTOW	3	/* FIXME: better name*/

#include <stdbool.h>

enum PacketType_e {
	DefaultPacket=0, /* Only used to signal to fx_initializePacket to create with the default type */
	CommandPacket=1,
	DataPacket=2,
	RoleswapPacket=3,
	CheckPacket=5,
	AckPacket=6,
	ErrorPacket=0x15,
	TerminatePacket=0x18
};

struct PacketTypeSet_t {
	enum PacketType_e type;
	char name[24];
};

union SubHeader_t {
	struct DataHeader_t* dh;	/* the usage of dh or ch is dependant */
	struct CommandHeader_t* ch;	/* on the type of packet */
};

struct Packet_t {
	enum PacketType_e type;
	char subtype;
	bool extended;	/// Indicates if a SubHeader is used in the packet.
	union SubHeader_t d;
	
};

struct DataHeader_t {
	/* notice that ds is missing. Is computed when generating packet */
	short unsigned int total;
	short unsigned int current;
	unsigned int datasize; /* positive when data ptr is used */
	char *data;
};

struct CommandHeader_t {
	/* notice that ds is missing. Is computed when generating packet */
	unsigned char overwrite;
	unsigned char datatype;
	unsigned int filesize;
	unsigned char sd[6];
	char *d[6];

};

int fx_isTypeValid(enum PacketType_e);
int fx_calculateChecksum(char *, int);
int fx_initializePacket(struct Packet_t *, enum PacketType_e, char subtype);
int fx_validatePacket(struct Packet_t *);
int fx_encodePacket(struct Packet_t *, char *, int);
int fx_printPacketStruct(struct Packet_t *);
int fx_extendPacket(struct Packet_t *);
int fx_contractPacket(struct Packet_t *);
int fx_attachDataPayload(struct Packet_t *, char *, int);
int fx_setCommandParameter(struct Packet_t *, unsigned int, char *, unsigned int);
int fx_getEscapedDataSize(unsigned char *, int);
int fx_escapeData(unsigned char *, unsigned char *, int);
int fx_send(struct usb_dev_handle*, struct Packet_t *);
#endif /* PACKETIO_H */
