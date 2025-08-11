#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include "graphics.h"
#define TELEMEA_IMPLEMENTATION
#include "../telemea.h"

#define STR_STARTSWITH(STR, PREFIX) (strncmp(PREFIX, STR, strlen(PREFIX)) == 0)

typedef struct SockSettings {
	char address[64];
	uint16_t port;
} SockSettings;

static bool readSockSettingFromIni(char *confPath, SockSettings *pSs) {
	const int NUMSTAGES = 3;
	char linebuf[256] = {0};
	int sret = 0;
	int num_parsestage = 0;
	FILE *fconf = NULL;
	
	memset(pSs, 0, sizeof(*pSs));
	
	if ((fconf = fopen(confPath, "r")) == NULL) return 0;
	
	// lazy ini parser
	static char *keynames[] = {
		"[socket]\n",
		"address=",
		"port=",
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
					case 1: num_parsestage++; strncpy(pSs->address, linebuf + len_keyname, sizeof(pSs->address)); break;
					case 2: num_parsestage++; pSs->port = atol(linebuf + len_keyname); break;
				}
			}
		}
	}
	
	char *c = pSs->address + strlen(pSs->address) - 1;
	if (*c == '\n' || *c == '\r') *c = 0; 
	
	fclose(fconf);
	
	
	
	if (num_parsestage != NUMSTAGES)
		return 0;
	return 1;
}












static Telemetry teldat = {0}; 









#define GRAPH_DEFAULTCLIWIDTH 80
#define GRAPH_DEFAULTCLIHEIGHT 25

int main(int argc, char* argv[]) {
	SockSettings socks;
	NET_Address *addr;
	NET_StreamSocket *clientSock = NULL;
	bool isRunning = true;
	bool isConnected = false;
	TelePacket tpRecv = {0};
	
	assert(readSockSettingFromIni("settings.ini", &socks));
	printf("%s %d\n", socks.address, socks.port);
	assert(NET_Init());
	addr = NET_ResolveHostname(socks.address);
	if (NET_WaitUntilResolved(addr, -1) != 1) {
		printf(SDL_GetError());
		return 1;
	}
	
	
	Graph_VGAFont font = {0};
	Graph_SDL3Framebuffer fb = {0};
	Graph_VGAConsole console = {0};
	SDL_Palette *pal;
	
	assert(SDL_InitSubSystem(SDL_INIT_VIDEO));
	
	assert(graph_VGAFontLoadFromPath("./BLUETERM.F12", &font));
	assert(graph_consoleSetSize(&console, &fb, &font, CNFL_9DOTCHAR, GRAPH_DEFAULTCLIWIDTH, GRAPH_DEFAULTCLIHEIGHT));
	assert(pal = graph_framebuffGetPalettePtr(&fb));
	graph_framebuffSetTitle(&fb, "Noctis wingman");
	
	graph_paletteGen_RGB332(pal);
	
	
	
	while (isRunning) {
		int bytesRecvd = 0;
		
		// draw
		if (isConnected) {
			graph_consolePrintf(&console, RGB332_INDEX(0, 255, 0), RGB332_INDEX(0, 0, 0), CNPRTF_NO_OVERFLOW, 0, GRAPH_DEFAULTCLIHEIGHT-1, "TCP connected!!");
		} else {
			char tp[] = "\\|/-";
			static ts = 0;
			graph_consolePrintf(&console, RGB332_INDEX(255, 0, 0), RGB332_INDEX(255, 255, 255), CNPRTF_NO_OVERFLOW, 0, GRAPH_DEFAULTCLIHEIGHT-1, 
					"%c connecting...",
					tp[(ts+0)%4]);
			ts = ++ts % 4;
		}
		
		graph_consoleRenderToBuf(&console, &fb);
		graph_framebuffBlit(&fb);
		
		
		
		
		if (isConnected) {
			int isNotEmpty = tpRecv.bytesWritten > 0;
			int bytesWanted = isNotEmpty ? telemetry_packetSize(tpRecv.data[0]) : 1;
			
			bytesRecvd = NET_ReadFromStreamSocket(clientSock, &(tpRecv.data[tpRecv.bytesWritten]), (bytesWanted - tpRecv.bytesWritten));
			
			if (bytesRecvd != -1) {
				tpRecv.bytesWritten += bytesRecvd;
				
				if (bytesRecvd > 0 && isNotEmpty && (tpRecv.bytesWritten == bytesWanted)) {
					unsigned char cc = 0;
					
					telemetry_packetReadByte(&tpRecv, &cc);
					switch (cc) {
						case CC_tp_pressure: {
							float hudPressure;
							
							telemetry_packetReadFloat(&tpRecv, &hudPressure);
							printf("%.6f\n", hudPressure);
						} break;
						
						case CC_DEBUGBEEP: {
							
						} break;
					}
					
					telemetry_packetClear(&tpRecv);
				}
			} else { // socket error
				NET_DestroyStreamSocket(clientSock);
				clientSock = NULL;
				isConnected = false;
			}
			
			
		} else { // not connected
			if (!clientSock)
				clientSock = NET_CreateClient(addr, socks.port);
			
			int status = NET_WaitUntilConnected(clientSock, 100);
			if (status == 1) {
				telemetry_packetClear(&tpRecv);
				isConnected = true;
				printf("\nClient connected to DOSBox.\n");
			} else if (status == -1) {
				NET_DestroyStreamSocket(clientSock);
				clientSock = NULL;
			}
		}
		
		SDL_Event event;
		
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				isRunning = 0;
			}
			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.key == SDLK_ESCAPE) {
					isRunning = 0;
				}
			}
		}
		
		// drain the packet queue so we don't lag behind
		if (bytesRecvd > 0)
			continue;
		
		SDL_Delay(32);
	}
	
	NET_Quit();
	graph_consoleDestroy(&console, &fb);
	graph_framebuffDestroy(&fb);
	SDL_Quit();
	
	return 0;
}