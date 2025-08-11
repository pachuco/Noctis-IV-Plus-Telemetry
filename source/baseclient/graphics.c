#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <SDL3/SDL.h>
#include "graphics.h"

void graph_framebuffDestroy(Graph_SDL3Framebuffer *pBuf) {
	if (pBuf->surface) {
		SDL_DestroySurface(pBuf->surface);
		pBuf->surface = NULL;
	}
	if (pBuf->palette) {
		SDL_DestroyPalette(pBuf->palette);
		pBuf->palette = NULL;
	}
	pBuf->windowSurface = NULL;
	
	if (pBuf->window) {
		SDL_DestroyWindow(pBuf->window);
		pBuf->window = NULL;
	}
	pBuf->width = 0;
	pBuf->height = 0;
}

bool graph_framebuffSetSize(Graph_SDL3Framebuffer *pBuf, int width, int height) {
	if (!pBuf) goto l_fail;
	
	if (pBuf->window) {
		if (!SDL_SetWindowSize(pBuf->window, width, height)) goto l_fail;
	} else {
		pBuf->window = SDL_CreateWindow("SDL3 Window", width, height, 0);
		if (!pBuf->window) goto l_fail;
	}
	pBuf->windowSurface = SDL_GetWindowSurface(pBuf->window);
	if (!pBuf->windowSurface) goto l_fail;
	
	if (!pBuf->palette) {
		pBuf->palette = SDL_CreatePalette(256);
		if (!pBuf->palette) goto l_fail;
	}
	
	if (pBuf->surface) {
		SDL_DestroySurface(pBuf->surface);
	}
	pBuf->surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
	if (!pBuf->surface) goto l_fail;
	
	if (!SDL_SetSurfacePalette(pBuf->surface, pBuf->palette)) goto l_fail;
	
	pBuf->width = width;
	pBuf->height = height;
	
	return true;
	l_fail:
		graph_framebuffDestroy(pBuf);
		return false;
}

SDL_Palette* graph_framebuffGetPalettePtr(Graph_SDL3Framebuffer *pBuf) {
	SDL_Palette *ret = NULL;
	
	if (pBuf) ret = pBuf->palette;
	
	return ret;
}

void graph_framebuffBlit(Graph_SDL3Framebuffer *pBuf) {
	if (!SDL_BlitSurface(pBuf->surface, NULL, pBuf->windowSurface, NULL)) goto l_fail;
	if (!SDL_UpdateWindowSurface(pBuf->window)) goto l_fail;
	return;
	l_fail:
		return;
}

void graph_framebuffSetTitle(Graph_SDL3Framebuffer *pBuf, char *title) {
	SDL_SetWindowTitle(pBuf->window, title);
}









void graph_palleteGen_grayscale(SDL_Palette *pal) {
	for (int i = 0; i < 256; i++) {
		pal->colors[i].r = i;
		pal->colors[i].g = i;
		pal->colors[i].b = i;
		pal->colors[i].a = 0xFF;
	};
}

void graph_paletteGen_RGB332(SDL_Palette *pal) {
	for (int i = 0; i < 256; i++) {
		// Extract RGB components from the 8-bit value
		int r = (i >> 5) & 0x07;  // Top 3 bits (bits 7-5)
		int g = (i >> 2) & 0x07;  // Middle 3 bits (bits 4-2)
		int b = i & 0x03;         // Bottom 2 bits (bits 1-0)

		// Scale to full 8-bit range
		pal->colors[i].r = (r * 255) / 7;  // 3 bits: 0-7 scaled to 0-255
		pal->colors[i].g = (g * 255) / 7;  // 3 bits: 0-7 scaled to 0-255
		pal->colors[i].b = (b * 255) / 3;  // 2 bits: 0-3 scaled to 0-255
		pal->colors[i].a = 0xFF;
	}
}

























SDL_Point calculateFontSize(Graph_VGAFont *pFont, uint32_t flags) {
	SDL_Point dim;
	
	dim.x = GRAPH_FONTWIDTH;
	if (BIT_GET(flags, CNFL_9DOTCHAR)) dim.x += 1;
	if (BIT_GET(flags, CNFL_DOUBLEWIDTH)) dim.x *= 2;
	dim.y = pFont->height;
	
	return dim;
}

bool graph_VGAFontLoadFromPath(char *path, Graph_VGAFont *pFont) {
	FILE* file = fopen(path, "rb");
	long length;

	if (!file) goto l_fail;
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (length == -1) goto l_fail;
	if (length % 256) goto l_fail;
	if (length > GRAPH_MAXFONTHEIGHT * 256) goto l_fail;

	pFont->height = length / 256;
	fread(pFont->data, 1, length, file);
	fclose(file);
	
	return true;
	l_fail:
		if (file) fclose(file);
		pFont->height = 0;
		return false;
}

void graph_consoleDestroy(Graph_VGAConsole *pCon, Graph_SDL3Framebuffer *pBuf) {
	if (pBuf) graph_framebuffDestroy(pBuf);
	if (pCon->allocPtr) free(pCon->allocPtr);
	
	memset(pCon, 0, sizeof(*pCon));
}

bool graph_consoleSetSize(Graph_VGAConsole *pCon, Graph_SDL3Framebuffer *pBuf,
				Graph_VGAFont *pFont, uint32_t fontFlags, int widthChars, int heightChars) {
	SDL_Point fontSize = calculateFontSize(pFont, fontFlags);
	
	if (!graph_framebuffSetSize(pBuf, fontSize.x * widthChars, fontSize.y * heightChars)) goto l_fail;
	if (pCon->allocPtr) free(pCon->allocPtr);
	pCon->allocPtr = calloc(4, widthChars * heightChars);
	if (!pCon->allocPtr) goto l_fail;
	
	pCon->text  = (uint8_t*)pCon->allocPtr + 0 * widthChars * heightChars;
	pCon->bgCol = (uint8_t*)pCon->allocPtr + 1 * widthChars * heightChars;
	pCon->fgCol = (uint8_t*)pCon->allocPtr + 2 * widthChars * heightChars;
	pCon->attr  = (uint8_t*)pCon->allocPtr + 3 * widthChars * heightChars;
	
	pCon->widthChars = widthChars;
	pCon->heightChars = heightChars;
	
	pCon->fontFlags = fontFlags;
	pCon->pFont = pFont;
	
	return true;
	l_fail:
		graph_consoleDestroy(pCon, pBuf);
		return false;
}

void graph_consoleRenderToBuf(Graph_VGAConsole *pCon, Graph_SDL3Framebuffer *pBuf) {
	if (!pCon || !pBuf) goto l_fail;
	
	assert(pBuf && pBuf->surface && pBuf->surface->format == SDL_PIXELFORMAT_INDEX8 && pCon->pFont);
	
	SDL_Point fontSize = calculateFontSize(pCon->pFont, pCon->fontFlags);
	int pixelCharStride = fontSize.x;
	int pixelScanlineStride = pBuf->width;
	int pixelRowStride = pBuf->width * fontSize.y;
	int inc = 1 + BIT_GET(pCon->fontFlags, CNFL_DOUBLEWIDTH);
	
	for (int i = 0; i < (pBuf->width * pBuf->height); i+= fontSize.x) {
		int surfaceX = i % pBuf->width;
		int surfaceY = i / pBuf->width;
		int consoleOffset = (surfaceY / fontSize.y) * (pBuf->width / fontSize.x) + (surfaceX / fontSize.x);
		int fontRow = surfaceY % fontSize.y;
		
		uint8_t c    = pCon->text[consoleOffset];
		uint8_t bgc  = pCon->bgCol[consoleOffset];
		uint8_t fgc  = pCon->fgCol[consoleOffset];
		uint8_t attr = pCon->attr[consoleOffset];
		
		if (BIT_GET(attr, CNAT_RENDERED)) continue;
		
		uint8_t fontRowData = pCon->pFont->data[c*pCon->pFont->height + fontRow];
		uint8_t *surfDatPtr = ((uint8_t*)pBuf->surface->pixels) + i;
		
		surfDatPtr[0+inc*0] = bgc * !BIT_GET(fontRowData, 1<<7) + fgc * BIT_GET(fontRowData, 1<<7);
		surfDatPtr[0+inc*1] = bgc * !BIT_GET(fontRowData, 1<<6) + fgc * BIT_GET(fontRowData, 1<<6);
		surfDatPtr[0+inc*2] = bgc * !BIT_GET(fontRowData, 1<<5) + fgc * BIT_GET(fontRowData, 1<<5);
		surfDatPtr[0+inc*3] = bgc * !BIT_GET(fontRowData, 1<<4) + fgc * BIT_GET(fontRowData, 1<<4);
		surfDatPtr[0+inc*4] = bgc * !BIT_GET(fontRowData, 1<<3) + fgc * BIT_GET(fontRowData, 1<<3);
		surfDatPtr[0+inc*5] = bgc * !BIT_GET(fontRowData, 1<<2) + fgc * BIT_GET(fontRowData, 1<<2);
		surfDatPtr[0+inc*6] = bgc * !BIT_GET(fontRowData, 1<<1) + fgc * BIT_GET(fontRowData, 1<<1);
		surfDatPtr[0+inc*7] = bgc * !BIT_GET(fontRowData, 1<<0) + fgc * BIT_GET(fontRowData, 1<<0);
		if (BIT_GET(pCon->fontFlags, CNFL_9DOTCHAR))
			surfDatPtr[0+inc*8] = bgc * !BIT_GET(fontRowData, 1<<0) + fgc * BIT_GET(fontRowData, 1<<0) * !BIT_GET(pCon->fontFlags, CNFL_9THDOTISBLANK);
		
		if (inc>1) {
			surfDatPtr[1+inc*0] = surfDatPtr[0+inc*0];
			surfDatPtr[1+inc*1] = surfDatPtr[0+inc*1];
			surfDatPtr[1+inc*2] = surfDatPtr[0+inc*2];
			surfDatPtr[1+inc*3] = surfDatPtr[0+inc*3];
			surfDatPtr[1+inc*4] = surfDatPtr[0+inc*4];
			surfDatPtr[1+inc*5] = surfDatPtr[0+inc*5];
			surfDatPtr[1+inc*6] = surfDatPtr[0+inc*6];
			surfDatPtr[1+inc*7] = surfDatPtr[0+inc*7];
			if (BIT_GET(pCon->fontFlags, CNFL_9DOTCHAR))
				surfDatPtr[1+inc*8] = surfDatPtr[0+inc*8];
		}
		
		pCon->attr[consoleOffset] = BIT_SET(attr, CNAT_RENDERED * (fontRow == fontSize.y-1));
	}
	
	return;
	l_fail:
		return;
}

int graph_consolePrintf(Graph_VGAConsole *pCon, int bgc, int fgc, uint32_t flags, int x, int y, const char *format, ...) {
	char *buf = NULL;
	int conOff = (x + pCon->widthChars*y);
	int maxXLen = pCon->widthChars - x;
	int maxXYLen = (pCon->widthChars * pCon->heightChars) - conOff;
	int needed = 0;
	int shouldNotBeNeededButIs = 0;
	int doneLen = 0;
	va_list args;
	
	if (x >= pCon->widthChars || y >= pCon->heightChars) return 0;
	if (x < 0 || y < 0) return 0; // TODO maybe: uses as trigger for relative position
	
	va_start(args, format);
	shouldNotBeNeededButIs = needed = vsnprintf(NULL, 0, format, args);
	va_end(args);
	
	if (needed > 0) {
		int max = BIT_GET(flags, CNPRTF_NO_OVERFLOW) ? maxXLen : maxXYLen;
		
		if (needed > max) needed = max;
		
		buf = calloc(shouldNotBeNeededButIs+1, 1); // need 0T
		
		if (buf) {
			va_start(args, format);
			// cannot assume vsnprintf doesn't return -1 on truncation, so always provide fitting buffer
			// fucked over by msvcrt.dll
			doneLen = vsnprintf(buf, shouldNotBeNeededButIs+1, format, args);
			va_end(args);
			
			for (int i=0; i < needed; i++) {
				pCon->text[conOff + i] = buf[i];
				pCon->bgCol[conOff + i] = bgc;
				pCon->fgCol[conOff + i] = fgc;
				pCon->attr[conOff + i] = BIT_CLEAR(pCon->attr[conOff + i], CNAT_RENDERED);
			}
		}
	}
	
	if (buf) free(buf);
	
	return needed;
}