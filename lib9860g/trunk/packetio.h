#ifndef PACKETIO_H
#define PACKETIO_H

/**
 * @file packetio.h
 * @author Andreas Bertheussen
 * @brief Definition of the structures needed for packet-based communication.
 */

#define ATRUE	'1'
#define AFALSE	'0'

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

enum boolean { false=0, true=1 };

/**
 * @union SubHeader
 * @brief A union either pointing to a DataHeader, a CommandHeader, or nothing (NULL).
 */
union SubHeader_t {
	struct DataHeader_t* dh;	/* the usage of dh or ch is dependant */
	struct CommandHeader_t* ch;	/* on the type of packet */
};

/**
 * @struct Packet
 * @brief The basic skeleton for any packet.
 *
 * The Packet structure contains all or pointers to the necessary data to make a packet.
 * The specifics for data and some command-packets are found in DataHeader and CommandHeader.
 *
 * Avsnitt.
 */
struct Packet_t {
	enum PacketType_e type;
	char subtype;
	enum boolean extended;	/// Indicates if a SubHeader is used in the packet.
	union SubHeader_t d;
	
};

/**
 * @struct DataHeader
 * @brief Defines the fields needed for a data packet, including a pointer to the raw data to transmit.
 */ 
struct DataHeader_t {
	/* notice that ds is missing. Is computed when generating packet */
	short unsigned int total;
	short unsigned int current;
	unsigned int datasize; /* positive when data ptr is used */
	char *data;
};

/**
 * @struct CommandHeader
 * @brief Defines the fields used in some command packets.
 */
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

#endif /* PACKETIO_H */
