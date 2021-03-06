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
/** \file
 *  Display structures.
 */

#ifndef __INCLUDED_DISPLAYDEF_H__
#define __INCLUDED_DISPLAYDEF_H__

#include "lib/ivis_common/imd.h"
#include "lib/ivis_common/pieclip.h"

#define	BOUNDARY_X			(22)
#define BOUNDARY_Y			(22)
//#define BOUNDARY_X		(DISP_WIDTH/20)	   // proportional to resolution - Alex M
//#define	BOUNDARY_Y		(DISP_WIDTH/16)

typedef struct _screen_disp_data
{
	iIMDShape	*imd;
	uint32_t		frameNumber;		// last frame it was drawn
	uint32_t		screenX, screenY;
	uint32_t		screenR;
} SCREEN_DISP_DATA;

#endif // __INCLUDED_DISPLAYDEF_H__
