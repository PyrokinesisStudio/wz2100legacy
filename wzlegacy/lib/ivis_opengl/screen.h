/*This code copyrighted (2013) for the Warzone 2100 Legacy Project under the GPLv2.

Warzone 2100 Legacy is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Warzone 2100 Legacy is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Warzone 2100 Legacy; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*//*
 * Screen.h
 *
 * Interface to the OpenGL double buffered display.
 *
 */
#ifndef _screen_h
#define _screen_h

/* Check the header files have been included from frame.h if they
 * are used outside of the framework library.
 */
#if !defined(_frame_h) && !defined(FRAME_LIB_INCLUDE)
#error Framework header files MUST be included from Frame.h ONLY.
#endif

#include "lib/framework/types.h"

/* ------------------------------------------------------------------------------------------- */

/* Legacy stuff
 * - only used in the sequence video code we have not yet decided whether to port or to junk */

/* Set the colour for text */
extern void screenSetTextColour(uint8_t red, uint8_t green, uint8_t blue);

/* backDrop */
extern void screen_SetBackDropFromFile(const char *filename);
extern void screen_StopBackDrop(void);
extern void screen_RestartBackDrop(void);
extern BOOL screen_GetBackDrop(void);
extern void screen_Upload(const char *newBackDropBmp, BOOL preview);

/* screendump */
extern void screenDumpToDisk(const char *path);

/* Toggle the display between full screen or windowed */
extern void	screenToggleMode(void);

extern int wz_texture_compression;

extern void screenDoDumpToDiskIfRequired(void);

void screen_enableMapPreview(char *name, int width, int height, Vector2i *playerpositions);
void screen_disableMapPreview(void);
BOOL screen_getMapPreview(void);
const char *screen_getMapName(void);
#endif
