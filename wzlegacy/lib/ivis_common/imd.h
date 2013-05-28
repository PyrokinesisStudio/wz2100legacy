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
#ifndef _imd_
#define _imd_


#include "ivisdef.h"

#define IMD_NAME				"IMD"
#define PIE_NAME				"PIE"  // Pumpkin image export data file
#define IMD_VER				1
#define PIE_VER				2

//*************************************************************************

#define iV_IMD_MAX_POINTS pie_MAX_VERTICES
#define iV_IMD_MAX_POLYS pie_MAX_POLYGONS

//*************************************************************************

// polygon flags	b0..b7: col, b24..b31: anim index


#define iV_IMD_TEX 0x00000200
#define iV_IMD_TEXANIM 0x00004000 // iV_IMD_TEX must be set also

//*************************************************************************

extern iIMDShape *iV_ProcessIMD(const char **ppFileData, const char *FileDataEnd );

extern BOOL iV_IMDSave(char *filename, iIMDShape *s, BOOL PieIMD);
extern void iV_IMDRelease(iIMDShape *s);

// How high up do we want to stop looking
#define DROID_VIS_UPPER	100

// How low do we stop looking?
#define DROID_VIS_LOWER	10

#endif
