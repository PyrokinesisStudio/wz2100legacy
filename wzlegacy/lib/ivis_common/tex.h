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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*/
#ifndef _tex_
#define _tex_

//#include <GLee.h>
#if defined __APPLE__ && defined __MACH__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "png_util.h"

//*************************************************************************

#define iV_TEX_MAX 128
#define iV_TEX_INVALID -1
#define iV_TEXNAME_MAX 64

//*************************************************************************

#define iV_TEXNAME(i)	((char *) (&_TEX_PAGE[(i)].name))

//*************************************************************************

typedef struct
{
	char name[iV_TEXNAME_MAX];
	GLuint id;
} iTexPage;

//*************************************************************************

extern unsigned int _TEX_INDEX;
extern iTexPage _TEX_PAGE[iV_TEX_MAX];

//*************************************************************************

extern int iV_GetTexture(const char *filename);
extern void iV_unloadImage(iV_Image *image);
extern unsigned int iV_getPixelFormat(const iV_Image *image);

extern int pie_ReplaceTexPage(iV_Image *s, const char *texPage, int maxTextureSize);
extern int pie_AddTexPage(iV_Image *s, const char *filename, int slot, int maxTextureSize);
extern void pie_TexInit(void);

extern void pie_InitSkybox(SDWORD pageNum);

extern void pie_MakeTexPageName(char * filename);

//*************************************************************************

extern void pie_TexShutDown(void);

#endif
