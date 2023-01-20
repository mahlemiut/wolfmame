/*
 *
 * INP v4 file handling
 * --------------------
 *
 * Theory of operation:
 *
 * Information about the INP file is stored in various packets, allowing for easier expansion of the format, without affecting compatibility with base MAME
 *
 * Packets each have a four character ID header, a size, and the data itself
 *
 * 	Packet IDs (to be expanded/ updated):
 * 		INPH (4 bytes): Initial header - Major version number (2 bytes), Minor version number (2 bytes)
 * 		VERS (string): Version string - version number and the version string derived from the current Git tag
 * 		SYST (string): Shortname for system being used.  MAME currently supports up to 16 characters for this.
 *		STRT (64 bits): Time recording was started
 * 		ENDT (64 bits): Time recording was stopped
 *		INPS: Empty packet indicating start of INP data stream
 * 		INPD: Packet containing frame input data
 * 		ISPD: Packet containing frame speed (as recorded)
 */

#include "emu.h"
//#include "inp4.h"

enum packet_type
{
	PACKET_INPH,
	PACKET_VERS,
	PACKET_SYST,
	PACKET_STRT,
	PACKET_ENDT,
	PACKET_INPS,
	PACKET_INPD,
	PACKET_FSPD
};

class input_file_v4
{

};

class input_packet
{
public:
	input_packet(const char* hdr) { strncpy((char*)header, hdr, 4); data = nullptr; size = 0; }
	input_packet(std::string d) { data = d.c_str(); size = d.size(); }
	input_packet(uint64_t d) { data = d; size = sizeof(d); }
	input_packet(uint32_t d) { data = d; size = sizeof(d); }
	input_packet(std::vector<uint64_t> d) { data = d.data(); size = d.size(); }
private:
	unsigned char header[4];
	uint64_t size;
	const std::variant<uint64_t,uint32_t,uint16_t,uint8_t,const char*> data;
};
