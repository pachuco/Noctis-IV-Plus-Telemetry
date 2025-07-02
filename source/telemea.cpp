#include "noctis-d.h"
#define TELEMEA_IMPLEMENTATION
#include "telemea.h"
#include "telepriv.h"

extern "C" {
	#include "serial.h"
}

#define STR_STARTSWITH(STR, PREFIX) (strncmp(PREFIX, STR, strlen(PREFIX)) == 0)









typedef struct Telemetry {
	int isActive;
	int serialPort;
	
	
	
	// from perspective of environment, outside the suit
	/*
	float environment_gravity;
	float environment_temperature;
	int   environment_star_class;
	float environment_star_mass;
	int   environment_planet_class;
	int   environment_planet_mass;
	*/
	float environment_pressure;    // pp_pressure
	
	// from perspective of suit/astronaut
	float perceptual_tiredness;    // tiredness
	float perceptual_gravity;      // pp_gravity
	float perceptual_temperature;  // pp_temp
	float perceptual_pressure;     // tp_pressure
	float perceptual_pulse;        // pp_pulse
	
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

static Telemetry teldat = {0};


static void clearTelemetryData() {
	teldat.perceptual_tiredness   = -1.0f;
	teldat.perceptual_gravity     = -1.0f;
	teldat.perceptual_temperature = -1.0f;
	teldat.perceptual_pressure    = -1.0f;
	teldat.perceptual_pulse       = -1.0f;
}
	

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
	if (!teldat.isActive) {
		int serialPort = initSerialFromIniPath("..\\DATA\\serial.ini");
		
		//logOpen();
		if (serialPort >= 0) {
			clearTelemetryData();
			teldat.serialPort = serialPort;
			teldat.isActive = 1;
		}
	}
}

void telemetry_halt() {
	if (teldat.isActive) {
		//logClose();
		serial_close(teldat.serialPort);
		teldat.isActive = 0;
	}
}




void telemetry_out_debugBeep() {
	if (teldat.isActive) {
		TelePacket tp = {0};
		
		telemetry_packetWriteByte(&tp, CC_DEBUGBEEP);
		serial_write_buffered(teldat.serialPort, tp.data, tp.bytesWritten);
	}
}

void telemetry_out_perceptualPressure(float val) {
	if (teldat.isActive) {
		TelePacket tp = {0};
		
		if (teldat.perceptual_pressure != val) {
			telemetry_packetWriteByte(&tp, CC_PERCEPTUAL_PRESSURE);
			telemetry_packetWriteFloat(&tp, val);
			serial_write_buffered(teldat.serialPort, tp.data, tp.bytesWritten);
			teldat.perceptual_pressure = val;
		}
	}
}
