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

#ifndef __INCLUDED_SRC_LIGHTNING_H__
#define __INCLUDED_SRC_LIGHTNING_H__

#include "lib/ivis_common/pietypes.h"

#define FOG_FLAGS		7
#define FOG_BACKGROUND	1
#define FOG_DISTANCE	2
#define FOG_GROUND		4
#define FOG_ENABLED		8

extern uint32_t fogStatus;

typedef enum _lightcols
{
	LIGHT_RED,
	LIGHT_GREEN,
	LIGHT_BLUE,
	LIGHT_YELLOW,
	LIGHT_WHITE
} LIGHT_COLOUR;

typedef struct _light
{
	Vector3i	position;
	uint8_t	type;
	uint32_t	range;
	LIGHT_COLOUR	colour;
} LIGHT;

extern void setTheSun(Vector3f newSun);
extern Vector3f getTheSun(void);

extern void	processLight(LIGHT *psLight);
//extern void	initLighting( void );
extern void initLighting(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
extern void	lightValueForTile(uint32_t tileX, uint32_t tileY);
extern void	doBuildingLights( void );
extern void UpdateFogDistance(float distance);
extern void	calcDroidIllumination(DROID *psDroid);
//darkens down the tiles that are outside the scroll limits
extern void setScrollLimitLighting(void);

extern void	findSunVector(void);

#endif // __INCLUDED_SRC_LIGHTNING_H__
