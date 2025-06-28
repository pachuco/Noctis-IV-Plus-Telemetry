#ifndef TELEMEA_HEADER
#define TELEMEA_HEADER

#define TELEMETRY_VERSION 0x0000


// All endianess are LE!
typedef enum TelemetryOpcode {
	CC_NOP,
	CC_PERCEPTUAL_TIREDNESS,
	CC_PERCEPTUAL_GRAVITY,
	CC_PERCEPTUAL_TEMPERATURE,
	CC_PERCEPTUAL_PRESSURE,
	CC_PERCEPTUAL_PULSE,
	
	CC_HELLO,
	CC_HELLOREPLY,
	CC_DEBUGBEEP,
	/////////////////////////////////CC_EXTENDED = 0xFF,
};




#define TELE_MAX_PACKETSIZE 32+1
typedef struct {
	unsigned char data[TELE_MAX_PACKETSIZE];
	int bytesWritten;
	int bytesRead;
} TelePacket;

#ifdef TELEMEA_IMPLEMENTATION
	//int telemetry_doendianswap = 0; // unused(LE <-> LE)
	
	void telemetry_packet_reset(TelePacket *tp) {
		memset(tp, 0, sizeof(*tp));
	}
	
	int telemetry_packetSize(int packtype) {
		switch (packtype) {
			case CC_NOP: return 1;
			
			case CC_PERCEPTUAL_TIREDNESS:
			case CC_PERCEPTUAL_GRAVITY:
			case CC_PERCEPTUAL_TEMPERATURE:
			case CC_PERCEPTUAL_PRESSURE:
			case CC_PERCEPTUAL_PULSE:
				return 1 + 4;
			
			case CC_HELLO: return 1 + 2;
			case CC_HELLOREPLY: return 1 + 1;
			case CC_DEBUGBEEP: return 1;
			
			default: return 1;
		}
	}

	#define _TELE_WRITE_GUTS \
		unsigned char *pIn = (unsigned char *)&x; \
		int i; \
		if (tp->bytesWritten + sizeof(x) > TELE_MAX_PACKETSIZE) \
			return 0; \
		for (i=0; i < sizeof(x); i++) \
			tp->data[tp->bytesWritten++] = *pIn++; \
		return 1;

	#define _TELE_READ_GUTS \
		unsigned char *pOut = (unsigned char *)px; \
		int i; \
		if (tp->bytesRead + sizeof(*px) > tp->bytesWritten) \
			return 0; \
		for (i=0; i < sizeof(*px); i++) \
			*pOut++ = tp->data[tp->bytesRead++]; \
		return 1;

	int telemetry_packet_write_byte(TelePacket *tp, unsigned char x)  { _TELE_WRITE_GUTS }
	int telemetry_packet_write_word(TelePacket *tp, unsigned short x) { _TELE_WRITE_GUTS }
	int telemetry_packet_write_dword(TelePacket *tp, unsigned long x) { _TELE_WRITE_GUTS }
	int telemetry_packet_write_float(TelePacket *tp, float x)         { _TELE_WRITE_GUTS }
	int telemetry_packet_write_double(TelePacket *tp, double x)       { _TELE_WRITE_GUTS }
	int telemetry_packet_read_byte(TelePacket *tp, unsigned char *px)  { _TELE_READ_GUTS }
	int telemetry_packet_read_word(TelePacket *tp, unsigned short *px) { _TELE_READ_GUTS }
	int telemetry_packet_read_dword(TelePacket *tp, unsigned long *px) { _TELE_READ_GUTS }
	int telemetry_packet_read_float(TelePacket *tp, float *px)         { _TELE_READ_GUTS }
	int telemetry_packet_read_double(TelePacket *tp, double *px)       { _TELE_READ_GUTS }

	#undef _TELE_READ_GUTS
	#undef _TELE_WRITE_GUTS
#else
	//extern int telemetry_doendianswap;

	void telemetry_packet_reset(TelePacket *tp);
	int telemetry_packetSize(int packtype);
	int telemetry_packet_write_byte(TelePacket *tp, unsigned char x);
	int telemetry_packet_write_word(TelePacket *tp, unsigned short x);
	int telemetry_packet_write_dword(TelePacket *tp, unsigned long x);
	int telemetry_packet_write_float(TelePacket *tp, float x);
	int telemetry_packet_write_double(TelePacket *tp, double x);
	int telemetry_packet_read_byte(TelePacket *tp, unsigned char *px);
	int telemetry_packet_read_word(TelePacket *tp, unsigned short *px);
	int telemetry_packet_read_dword(TelePacket *tp, unsigned long *px);
	int telemetry_packet_read_float(TelePacket *tp, float *px);
	int telemetry_packet_read_double(TelePacket *tp, double *px);
#endif

#endif