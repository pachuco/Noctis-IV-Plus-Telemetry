#include <math.h>
#include "noctis-d.h"
#define TELEMEA_IMPLEMENTATION
#include "telemea.h"
#include "telepriv.h"

extern "C" {
	#include "serial.h"
}

#define STR_STARTSWITH(STR, PREFIX) (strncmp(PREFIX, STR, strlen(PREFIX)) == 0)

static FILE *fDebugLog = NULL;

void logOpen() {
	if (!fDebugLog) {
		fDebugLog = fopen(".\\deburger.txt", "w");
	}
}

void logWrite(const char *text) {
	if (fDebugLog) {
		fprintf(fDebugLog, "%s", text);
		fflush(fDebugLog);
	}
}

void logClose() {
	if (fDebugLog) {
		fclose(fDebugLog);
		fDebugLog = NULL;
	}
}



#define XXX(TYPE, NAME) extern TYPE NAME;
TELE_MEMBERS
#undef XXX

static Telemetry teldat = {0};
static int isActive = 0;
static int serialPort = -1;



	

static int initSerialFromIniPath(char *serialconfpath) {
	const NUMSTAGES = 7;
	char linebuf[256] = {0};
	int sret = 0;
	int num_parsestage = 0;
	FILE *fconf = NULL;
	
	if ((fconf = fopen(serialconfpath, "r")) == NULL) return -20;
	
	// lazy ini parser
	static char *keynames[] = {
		"[serial]\n",
		"port_number=",
		"data_rate=",
		"data_bits=",
		"parity=",
		"stop_bits=",
		"handshake="
	};
	int val_numport=0; long val_bps=0; int val_databits=0; char val_parity=0; int val_stopbits=0; int val_handshake=0;
	
	while (fgets(linebuf, sizeof(linebuf), fconf) != NULL) {
		if (num_parsestage == 0) { // find ini section
			if (strcmp(linebuf, keynames[num_parsestage]) == 0)
				num_parsestage++;
		} else if (num_parsestage == NUMSTAGES) { // done!
			break;
		} else { // find key=value
			if (STR_STARTSWITH(linebuf, keynames[num_parsestage])) {
				int len_keyname = strlen(keynames[num_parsestage]);
				
				switch (num_parsestage) {
					case 1: num_parsestage++; val_numport   = atoi(linebuf + len_keyname) - 1; break; // see serial.h
					case 2: num_parsestage++; val_bps       = atol(linebuf + len_keyname); break;
					case 3: num_parsestage++; val_databits  = atoi(linebuf + len_keyname); break;
					case 4: num_parsestage++; val_parity    = linebuf[len_keyname]; break;
					case 5: num_parsestage++; val_stopbits  = atoi(linebuf + len_keyname); break;
					case 6: {
						if (strcmp(linebuf + len_keyname, "none") == 0) {
							num_parsestage++;
							val_handshake = SER_HANDSHAKING_NONE;
						} else if (strcmp(linebuf + len_keyname, "xonxoff") == 0) {
							num_parsestage++;
							val_handshake = SER_HANDSHAKING_XONXOFF;
						} else if (strcmp(linebuf + len_keyname, "rtscts") == 0) {
							num_parsestage++;
							val_handshake = SER_HANDSHAKING_RTSCTS;
						} else if (strcmp(linebuf + len_keyname, "dtrdts") == 0) {
							num_parsestage++;
							val_handshake = SER_HANDSHAKING_DTRDSR;
						}
					} break;
				}
			}
		}
	}
	fclose(fconf);
	
	if (num_parsestage != NUMSTAGES)
		return -21;
	if ((sret = serial_open(val_numport, val_bps, val_databits, val_parity, val_stopbits, val_handshake)) != SER_SUCCESS)
		return sret; // -1 to -16
	return val_numport; // 0 to 4
}






void telemetry_init() {
	if (!isActive) {
		int port = initSerialFromIniPath("..\\DATA\\serial.ini");
		
		//logOpen();
		if (port >= 0) {
			telemetry_clearData(&telemetry);
			serialPort = port;
			isActive = 1;
		}
	}
}

void telemetry_halt() {
	if (isActive) {
		//logClose();
		// don't clog client with partial packets
		while (serial_get_tx_buffered(serialPort));
		while (serial_get_rx_buffered(serialPort));
		serial_close(serialPort);
		isActive = 0;
	}
}


int sortOfEquals(float a, float b) {
  return (signed long)(a * 1000.0f) == (signed long)(b * 1000.0f);
}
#define SEND_FLOAT_EXACT_COMPARE(NAME) \
if (teldat.NAME != NAME) { \
	telemetry_packetWriteByte(&tp, CC_##NAME); \
	telemetry_packetWriteFloat(&tp, NAME); \
	serial_write_buffered(serialPort, tp.data, tp.bytesWritten); \
	teldat.NAME = NAME; \
	telemetry_packetClear(&tp); \
}
#define SEND_FLOAT_ROUGH_COMPARE(NAME) \
if (!sortOfEquals(teldat.NAME, NAME)) { \
	telemetry_packetWriteByte(&tp, CC_##NAME); \
	telemetry_packetWriteFloat(&tp, NAME); \
	serial_write_buffered(serialPort, tp.data, tp.bytesWritten); \
	teldat.NAME = NAME; \
	telemetry_packetClear(&tp); \
}
void telemetry_updateAll() {
	if (isActive) {
		TelePacket tp = {0};
		
		SEND_FLOAT_EXACT_COMPARE(tiredness);
		SEND_FLOAT_EXACT_COMPARE(pp_gravity);
		SEND_FLOAT_EXACT_COMPARE(pp_temp);
		SEND_FLOAT_EXACT_COMPARE(pp_pressure);
		SEND_FLOAT_EXACT_COMPARE(pp_pulse);
		SEND_FLOAT_EXACT_COMPARE(tp_gravity);
		SEND_FLOAT_EXACT_COMPARE(tp_temp);
		SEND_FLOAT_ROUGH_COMPARE(tp_pressure);
		SEND_FLOAT_EXACT_COMPARE(tp_pulse);
	}
}


void telemetry_out_debugBeep() {
	if (isActive) {
		TelePacket tp = {0};
		
		telemetry_packetWriteByte(&tp, CC_DEBUGBEEP);
		serial_write_buffered(serialPort, tp.data, tp.bytesWritten);
	}
}
