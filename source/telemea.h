#ifndef TELEMEA_HEADER
#define TELEMEA_HEADER

#define TELEMETRY_VERSION 0x0000


// All endianess are LE!
typedef enum Telecodeout {
	TCOUT_ASTRONAUT_TIREDNESS,
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
	TCIN_HELLO,
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

#endif