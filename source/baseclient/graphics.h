#ifndef GRAPHICS_HEADER
#define GRAPHICS_HEADER

#include <stdint.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#define BIT_SET(BYTE, BIT)   (BYTE)|(BIT)
#define BIT_CLEAR(BYTE, BIT) (BYTE)&(~(BIT))
#define BIT_GET(BYTE, BIT)   !!((BYTE)&(BIT))

typedef struct Graph_SDL3Framebuffer {
	SDL_Window *window;
	SDL_Surface *windowSurface;
	SDL_Surface *surface;
	SDL_Palette *palette;
	int width;
	int height;
} Graph_SDL3Framebuffer;

void graph_framebuffDestroy(Graph_SDL3Framebuffer *pBuf);
bool graph_framebuffSetSize(Graph_SDL3Framebuffer *pBuf, int width, int height);
SDL_Palette* graph_framebuffGetPalettePtr(Graph_SDL3Framebuffer *pBuf);
void graph_framebuffBlit(Graph_SDL3Framebuffer *pBuf);
void graph_framebuffSetTitle(Graph_SDL3Framebuffer *pBuf, char *title);

#define GRAPH_MAXFONTHEIGHT 32
#define GRAPH_FONTWIDTH 8
#define COL_DONTCARE -1
typedef struct Graph_VGAFont {
	uint8_t data[GRAPH_MAXFONTHEIGHT * 256];
	int height;
} Graph_VGAFont;

typedef struct Graph_VGAConsole {
	Graph_VGAFont *pFont;
	uint32_t fontFlags;
	
	SDL_Point calculatedFontSize;
	int widthChars;
	int heightChars;
	
	void *allocPtr;
	uint8_t *text, *bgCol, *fgCol, *attr;
} Graph_VGAConsole;

enum {
	CNFL_9DOTCHAR        =1<<0,  // repeats 8th pixel on the 9th
	CNFL_9THDOTISBLANK   =1<<1,  // instead of repeating 8th pixel on 9th, make it blank
	CNFL_DOUBLEWIDTH     =1<<2,	 // doubles the width
};

enum {
	CNAT_UNDERLINE       =1<<0,
	CNAT_STRIKETHROUGH   =1<<1,
	
	CNAT_RENDERED        =1<<7,
};

enum {
	CNPRTF_NO_OVERFLOW     =1<<0,
	CNPRTF_NO_CURSORUPDATE =1<<1,
};

SDL_Point calculateFontSize(Graph_VGAFont *pFont, uint32_t flags);
bool graph_VGAFontLoadFromPath(char *path, Graph_VGAFont *pFont);
void graph_consoleDestroy(Graph_VGAConsole *pCon, Graph_SDL3Framebuffer *pBuf);
bool graph_consoleSetSize(Graph_VGAConsole *pCon, Graph_SDL3Framebuffer *pBuf, Graph_VGAFont *pFont, uint32_t fontFlags, int widthChars, int heightChars);
void graph_consoleRenderToBuf(Graph_VGAConsole *pCon, Graph_SDL3Framebuffer *pBuf);
int graph_consolePrintf(Graph_VGAConsole *pCon, int bgc, int fgc, uint32_t flags, int x, int y, const char *format, ...);

#endif