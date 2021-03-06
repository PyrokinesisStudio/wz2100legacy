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

#ifndef __INCLUDED_SRC_GEOMETRY_H__
#define __INCLUDED_SRC_GEOMETRY_H__

#include "map.h"
#include "hci.h"

typedef struct _t_quad
{
	Vector2i coords[4];
} QUAD;

extern uint32_t adjustDirection( int32_t present, int32_t difference );
extern int32_t calcDirection( uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1 );
extern void initBulletTable( void );
extern int inQuad( const Vector2i *pt, const QUAD *quad );
extern DROID *getNearestDroid( uint32_t x, uint32_t y, BOOL bSelected );
extern BOOL droidOnScreen( DROID *psDroid, int32_t tolerance );

extern unsigned int WZ_DECL_CONST dirtyHypot(int deltaX, int deltaY);

static inline STRUCTURE *getTileStructure(uint32_t x, uint32_t y)
{
	BASE_OBJECT *psObj = mapTile(x, y)->psObject;
	if (psObj && psObj->type == OBJ_STRUCTURE)
	{
		return (STRUCTURE *)psObj;
	}
	return NULL;
}

static inline FEATURE *getTileFeature(uint32_t x, uint32_t y)
{
	BASE_OBJECT *psObj = mapTile(x, y)->psObject;
	if (psObj && psObj->type == OBJ_FEATURE)
	{
		return (FEATURE *)psObj;
	}
	return NULL;
}

static inline BASE_OBJECT *getTileOccupier(uint32_t x, uint32_t y)
{
	MAPTILE *psTile = mapTile(x, y);

	if (TEST_TILE_VISIBLE(selectedPlayer, psTile))
	{
		return mapTile(x, y)->psObject;
	}
	else
	{
		return NULL;
	}
}

#endif // __INCLUDED_SRC_GEOMETRY_H__
