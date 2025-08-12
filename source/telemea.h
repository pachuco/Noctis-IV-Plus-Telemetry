#ifndef TELEMEA_HEADER
#define TELEMEA_HEADER

//#define TELEMETRY_VERSION 0x0000
#define TELE_MEMBERS \
	XXX(float, tiredness) \
	XXX(float, pp_gravity) \
	XXX(float, pp_temp) \
	XXX(float, pp_pressure) \
	XXX(float, pp_pulse) \
	XXX(float, tp_gravity) \
	XXX(float, tp_temp) \
	XXX(float, tp_pressure) \
	XXX(float, tp_pulse)

// All endianess are LE!
#define XXX(TYPE, NAME) CC_##NAME,
typedef enum TelemetryOpcode {
	CC_NOP,
	TELE_MEMBERS
	
	CC_DEBUGBEEP,
	/////////////////////////////////CC_EXTENDED = 0xFF,
};
#undef XXX

#define XXX(TYPE, NAME) TYPE NAME;
typedef struct Telemetry {
	TELE_MEMBERS
	
	// miscelaneous perceptual
	/*
		- is sun visible?
		- what angle are you facintg sun at?
		- how much space does it take on your FOV?
		- is sun's turbulent surface visible?
		- colours/palette
		-
	*/
} Telemetry;
#undef XXX





#define TELE_MAX_PACKETSIZE 32+1
typedef struct {
	unsigned char data[TELE_MAX_PACKETSIZE];
	int bytesWritten;
	int bytesRead;
} TelePacket;

#ifdef TELEMEA_IMPLEMENTATION
	//int telemetry_doendianswap = 0; // unused(LE <-> LE)
	
	void telemetry_clearData(Telemetry *pTel) {
		pTel->tiredness    = -1.0f;
		pTel->pp_gravity   = -1.0f;
		pTel->pp_temp      = -1.0f;
		pTel->pp_pressure  = -1.0f;
		pTel->pp_pulse     = -1.0f;
		pTel->tp_gravity   = -1.0f;
		pTel->tp_temp      = -1.0f;
		pTel->tp_pressure  = -1.0f;
		pTel->tp_pulse     = -1.0f;
	}
  
	void telemetry_packetClear(TelePacket *tp) {
		memset(tp, 0, sizeof(*tp));
	}
	
	void telemetry_packetResetRead(TelePacket *tp) {
		tp->bytesRead = 0;
	}
	
	int telemetry_packetSize(int packtype) {
		switch (packtype) {
			case CC_NOP: return 1;
			
			case CC_tiredness:
			case CC_pp_gravity:
			case CC_pp_temp:
			case CC_pp_pressure:
			case CC_pp_pulse:
			case CC_tp_gravity:
			case CC_tp_temp:
			case CC_tp_pressure:
			case CC_tp_pulse:
				return 1 + 4;
			
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

	int telemetry_packetWriteByte(TelePacket *tp, unsigned char x)  { _TELE_WRITE_GUTS }
	int telemetry_packetWriteWord(TelePacket *tp, unsigned short x) { _TELE_WRITE_GUTS }
	int telemetry_packetWriteDword(TelePacket *tp, unsigned long x) { _TELE_WRITE_GUTS }
	int telemetry_packetWriteFloat(TelePacket *tp, float x)         { _TELE_WRITE_GUTS }
	int telemetry_packetWriteDouble(TelePacket *tp, double x)       { _TELE_WRITE_GUTS }
	int telemetry_packetReadByte(TelePacket *tp, unsigned char *px)  { _TELE_READ_GUTS }
	int telemetry_packetReadWord(TelePacket *tp, unsigned short *px) { _TELE_READ_GUTS }
	int telemetry_packetReadDword(TelePacket *tp, unsigned long *px) { _TELE_READ_GUTS }
	int telemetry_packetReadFloat(TelePacket *tp, float *px)         { _TELE_READ_GUTS }
	int telemetry_packetReadDouble(TelePacket *tp, double *px)       { _TELE_READ_GUTS }

	#undef _TELE_READ_GUTS
	#undef _TELE_WRITE_GUTS
#else
	//extern int telemetry_doendianswap;

	void telemetry_clearData(Telemetry *pTel);
	void telemetry_packetClear(TelePacket *tp);
	void telemetry_packetResetRead(TelePacket *tp);
	int telemetry_packetSize(int packtype);
	int telemetry_packetWriteByte(TelePacket *tp, unsigned char x);
	int telemetry_packetWriteWord(TelePacket *tp, unsigned short x);
	int telemetry_packetWriteDword(TelePacket *tp, unsigned long x);
	int telemetry_packetWriteFloat(TelePacket *tp, float x);
	int telemetry_packetWriteDouble(TelePacket *tp, double x);
	int telemetry_packetReadByte(TelePacket *tp, unsigned char *px);
	int telemetry_packetReadWord(TelePacket *tp, unsigned short *px);
	int telemetry_packetReadDword(TelePacket *tp, unsigned long *px);
	int telemetry_packetReadFloat(TelePacket *tp, float *px);
	int telemetry_packetReadDouble(TelePacket *tp, double *px);
#endif

#endif