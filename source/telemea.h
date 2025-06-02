#ifndef TELEMEA_HEADER
#define TELEMEA_HEADER

#define TELEMETRY_VERSION 0x0000


// All endianess are LE!
#define TC_NOP 0x00

typedef enum Telecodeout {
	TCOUT_ASTRONAUT_TIREDNESS = 1,
	// C XXXX
	// where X is float
	TCOUT_ASTRONAUT_GRAVITY,
	// C XXXX
	// where X is float
	TCOUT_ASTRONAUT_TEMPERATURE,
	// C XXXX
	// where X is float
	TCOUT_ASTRONAUT_PRESSURE,
	// C XXXX
	// where X is float
	TCOUT_ASTRONAUT_PULSE,
	// C XXXX
	// where X is float






	TCOUT_ACKNACK = 0xF0,
	// Responds to TCIN_HELLO.
	// C X
	// where X is a C-styled boolean. True if TELEMETRY_VERSION matches.
	
	TCOUT_PING,
	// Ping client and wait for TCIN_PONG in timely fashion.
	// Disconnect client on timeout.
	// C
	
	TCOUT_DEBUGBEEP,
	// Send the client a ping that produces a short audible pop.
	// Useful for checking how often a function is called.
	// C
	
	// make room for another 256 codes
	//TCOUT_EXTENDED = 0xFF,
} Telecodeout;

typedef enum Telecodein {
	TCIN_HELLO = 1,
	// Client says hello to server and waits for TCOUT_ACKNACK.
	// C XX
	// where X is uint16 with TELEMETRY_VERSION, which must match between client-server.
	
	TCIN_PONG,
	// Reply to TCOUT_PING.
	// C
	
	// make room for another 256 codes
	//TCIN_EXTENDED = 0xFF,
} Telecodein;

int telemetry_startup_and_init_serial(char *serialconfpath);
void telemetry_shutdown();

void telemetry_out_debugbeep();




#define TELE_MAX_PACKETSIZE 16
typedef struct {
	unsigned char data[TELE_MAX_PACKETSIZE];
	unsigned char bytes_wanted;
	unsigned char bytes_written;
	unsigned char bytes_read;
} Telepacket;

#ifdef TELEMEA_IMPLEMENTATION
	int telemetry_doendianswap = 0; // unused(LE <-> LE)
	
	void telemetry_packet_reset(Telepacket *tp) {
		memset(tp, 0, sizeof(*tp));
	}

	int telemetry_packet_isready(Telepacket *tp) {
		return ((tp->bytes_wanted > 0) && (tp->bytes_wanted == tp->bytes_written));
	}

	#define _TELE_WRITE_GUTS \
		unsigned char *pIn = (unsigned char *)&x; \
		int i; \
		if (tp->bytes_written + sizeof(x) > TELE_MAX_PACKETSIZE) \
			return 0; \
		if (tp->bytes_written + sizeof(x) > tp->bytes_wanted) \
			return 0; \
		for (i=0; i < sizeof(x); i++) \
			tp->data[tp->bytes_written++] = *pIn++; \
		return 1;

	#define _TELE_READ_GUTS \
		unsigned char *pOut = (unsigned char *)px; \
		int i; \
		if (!telemetry_packet_isready(tp)) \
			return 0; \
		if (tp->bytes_read + sizeof(*px) > TELE_MAX_PACKETSIZE) \
			return 0; \
		if (tp->bytes_read + sizeof(*px) > tp->bytes_wanted) \
			return 0; \
		for (i=0; i < sizeof(*px); i++) \
			*pOut++ = tp->data[tp->bytes_read]; \
		return 1;

	int telemetry_packet_write_byte(Telepacket *tp, unsigned char x)  { _TELE_WRITE_GUTS }
	int telemetry_packet_write_word(Telepacket *tp, unsigned short x) { _TELE_WRITE_GUTS }
	int telemetry_packet_write_dword(Telepacket *tp, unsigned long x) { _TELE_WRITE_GUTS }
	int telemetry_packet_write_float(Telepacket *tp, float x)         { _TELE_WRITE_GUTS }
	int telemetry_packet_write_double(Telepacket *tp, double x)       { _TELE_WRITE_GUTS }
	int telemetry_packet_read_byte(Telepacket *tp, unsigned char *px)  { _TELE_READ_GUTS }
	int telemetry_packet_read_word(Telepacket *tp, unsigned short *px) { _TELE_READ_GUTS }
	int telemetry_packet_read_dword(Telepacket *tp, unsigned long *px) { _TELE_READ_GUTS }
	int telemetry_packet_read_float(Telepacket *tp, float *px)         { _TELE_READ_GUTS }
	int telemetry_packet_read_double(Telepacket *tp, double *px)       { _TELE_READ_GUTS }

	#undef _TELE_READ_GUTS
	#undef _TELE_WRITE_GUTS
#else
	extern int telemetry_doendianswap;

	void telemetry_packet_reset(Telepacket *tp);
	int telemetry_packet_isready(Telepacket *tp);
	int telemetry_packet_write_byte(Telepacket *tp, unsigned char x);
	int telemetry_packet_write_word(Telepacket *tp, unsigned short x);
	int telemetry_packet_write_dword(Telepacket *tp, unsigned long x);
	int telemetry_packet_write_float(Telepacket *tp, float x);
	int telemetry_packet_write_double(Telepacket *tp, double x);
	int telemetry_packet_read_byte(Telepacket *tp, unsigned char *px);
	int telemetry_packet_read_word(Telepacket *tp, unsigned short *px);
	int telemetry_packet_read_dword(Telepacket *tp, unsigned long *px);
	int telemetry_packet_read_float(Telepacket *tp, float *px);
	int telemetry_packet_read_double(Telepacket *tp, double *px);
#endif

#endif