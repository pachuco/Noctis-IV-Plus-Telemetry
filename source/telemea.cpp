#include "noctis-d.h"
#include "telemea.h"

extern "C" {
	#include "serial.h"
}

#define STR_STARTSWITH(STR, PREFIX) (strncmp(PREFIX, STR, strlen(PREFIX)) == 0)

static int serialnum_open = -1;
static int is_clientconnected = 0;









typedef struct Telemetry {
	// from perspective of environment, outside the suit
	/*
	float environment_gravity;
	float environment_temperature;
	float environment_pressure;
	int   environment_star_class;
	float environment_star_mass;
	int   environment_planet_class;
	int   environment_planet_mass;
	*/
	
	// from perspective of suit/astronaut
	float astronaut_tiredness;    // tiredness
	float astronaut_gravity;      // pp_gravity
	float astronaut_temperature;  // pp_temp
	float astronaut_pressure;     // pp_pressure
	float astronaut_pulse;        // pp_pulse
	
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
static Telemetry teldat;


static void clear_telemetry(Telemetry* tele)
{
	tele->astronaut_tiredness   = -1.0f;
	tele->astronaut_gravity     = -1.0f;
	tele->astronaut_temperature = -1.0f;
	tele->astronaut_pressure    = -1.0f;
	tele->astronaut_pulse       = -1.0f;
}
	

int telemetry_startup_and_init_serial(char *serialconfpath)
{
	char linebuf[256] = {0};
	int sret = 0;
	int num_parsestage = 0;
	FILE *fconf = NULL;
	
	if (serialnum_open >= 0) return SER_ERR_ALREADY_OPEN;
	
	clear_telemetry(&teldat);
	
	if ((fconf = fopen(serialconfpath, "r")) == NULL) return -20;
	
	// lazy ini parser
	static char *keynames[] = {
		"",
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
			if (strcmp(linebuf, "[serial]\n") == 0)
				num_parsestage++;
		} else if (num_parsestage == 7) { // done!
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
	
	if (num_parsestage != 7)
		return -21;
	if ((sret = serial_open(val_numport, val_bps, val_databits, val_parity, val_stopbits, val_handshake)) != SER_SUCCESS)
		return sret; // 0 to -16
	serialnum_open = val_numport;
//telemetry_out_debugbeep();
//char helloMsg[] = "Hello from noctis.\n"; serial_write(val_numport, helloMsg, sizeof(helloMsg));
	return 0;
}

void telemetry_shutdown()
{
	int snum = serialnum_open;
	
	if (snum >= 0) {
		serialnum_open = -1;
		is_clientconnected = 0;
		serial_close(snum);
	}
}



void telemetry_out_debugbeep()
{
	char data = TCOUT_DEBUGBEEP;
	if (serialnum_open < 0) return;
	
	serial_write(serialnum_open, &data, 1);
}






/*
void telemetry_update_astronaut_ppstuff(float gravity, float temperature, float pressure, float pulse)
{
	
}
*/
