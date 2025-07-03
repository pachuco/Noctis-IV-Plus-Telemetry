#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#define TELEMEA_IMPLEMENTATION
#include "../telemea.h"

#define STR_STARTSWITH(STR, PREFIX) (strncmp(PREFIX, STR, strlen(PREFIX)) == 0)

typedef struct SockSettings {
	char address[64];
	int port;
} SockSettings;

static bool readSockSettingFromIni(char *confPath, SockSettings *pSs) {
	const int NUMSTAGES = 3;
	char linebuf[256] = {0};
	int sret = 0;
	int num_parsestage = 0;
	FILE *fconf = NULL;
	
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
	fclose(fconf);
	
	if (num_parsestage != NUMSTAGES)
		return 0;
	return 1;
}

int isErrorMajor_winsock() {
	switch(WSAGetLastError()) {
		case WSAEWOULDBLOCK:
		case WSAEINTR:
		case WSAEINPROGRESS:
		case WSAEALREADY:
		case WSAENOBUFS:
			return 0;
			break;
		default:
			return 1;
	}
/*
Maybe retry after timeout:
WSAETIMEDOUT
WSAEHOSTUNREACH
WSAENETUNREACH
WSAENETDOWN
WSAECONNREFUSED
WSAEHOSTDOWN

Back out entirely:
WSAECONNRESET
WSAECONNABORTED
WSAEDISCON
WSAENOTCONN
WSAEISCONN
WSAENOTSOCK
WSAEBADF
*/
}




























int main(int argc, char* argv[]) {
	SockSettings socks;
	struct sockaddr_in addr = {0};
	SOCKET clientSock;
	WSADATA wsaData;
	bool isRunning = true;
	bool isConnected = true;
	u_long mode = 1;
	TelePacket tpRecv = {0};
	bool isPacketRecvd = true;
	
	assert(readSockSettingFromIni("settings.ini", &socks));
	WSAStartup(MAKEWORD(2,2), &wsaData);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(socks.port);
	addr.sin_addr.s_addr = inet_addr(socks.address);
	
	while (isRunning) {
		if (isConnected) {
			int isNotEmpty = tpRecv.bytesWritten > 0;
			int bytesWanted = isNotEmpty ? telemetry_packetSize(tpRecv.data[0]) : 1;
			
			int bytesRecvd = recv(clientSock, &(tpRecv.data[tpRecv.bytesWritten]), (bytesWanted - tpRecv.bytesWritten), 0);
			
			if (bytesRecvd != SOCKET_ERROR) {
				tpRecv.bytesWritten += bytesRecvd;
				
				if (bytesRecvd > 0 && isNotEmpty && (tpRecv.bytesWritten == bytesWanted)) {
					unsigned char cc = 0;
					
					isPacketRecvd = true;
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
				if (isErrorMajor_winsock())
					isConnected = false;
			}
			
			
		} else { // not connected
			clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (connect(clientSock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
				
				ioctlsocket(clientSock, FIONBIO, &mode);
				telemetry_packetClear(&tpRecv);
				isConnected = true;
				printf("\nClient connected to DOSBox.\n");
			} else {
				printf(".");
				SleepEx(2000, 1);
			}
		}
		
		// drain the packet queue so we don't lag behind
		if (isPacketRecvd) {
			
			SleepEx(10, 1);
		}
		isPacketRecvd = false;
	}
}