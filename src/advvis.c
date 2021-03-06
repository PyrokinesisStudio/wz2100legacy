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
/**
 * @file advvis.c
 *
 * Makes smooth transitions for terrain visibility.
 */

#include "lib/framework/frame.h"
#include "lib/gamelib/gtime.h"

#include "advvis.h"
#include "display3d.h"
#include "hci.h"
#include "map.h"

// ------------------------------------------------------------------------------------
#define FADE_IN_TIME	(GAME_TICKS_PER_SEC/10)
#define	START_DIVIDE	(8)

static BOOL bRevealActive = false;


void	avInformOfChange(int32_t x, int32_t y)
{
	MAPTILE	*psTile = mapTile(x, y);

	if (!tileIsExplored(psTile))
	{
		SET_TILE_EXPLORED(psTile);
	}
}


// ------------------------------------------------------------------------------------
static void processAVTile(uint32_t x, uint32_t y, float increment)
{
	MAPTILE	*psTile = mapTile(x, y);
	float	maxLevel = psTile->illumination;

	if (bRevealActive && psTile->level == 0 && !tileIsExplored(psTile))	// stay unexplored
	{
		return;
	}
	if (!hasSensorOnTile(psTile, selectedPlayer))
	{
		maxLevel /= 2;
	}
	if (psTile->level > maxLevel)
	{
		psTile->level = MAX(psTile->level - increment, 0);
	}
	else if (psTile->level < maxLevel)
	{
		psTile->level = MIN(psTile->level + increment, maxLevel);
	}
}


// ------------------------------------------------------------------------------------
void	avUpdateTiles( void )
{
	uint32_t	i, j;
	float	increment = timeAdjustedIncrement(FADE_IN_TIME, true);	// call once per frame

	/* Go through the tiles */
	for (i = 0; i < mapWidth; i++)
	{
		for (j = 0; j < mapHeight; j++)
		{
			processAVTile(i, j, increment);
		}
	}
}


// ------------------------------------------------------------------------------------
uint32_t	avGetObjLightLevel(BASE_OBJECT *psObj, uint32_t origLevel)
{
	float div = (float)psObj->visible[selectedPlayer] / 255.f;

	unsigned int lowest = origLevel / START_DIVIDE;
	unsigned int newLevel = div * origLevel;

	if(newLevel < lowest)
	{
		newLevel = lowest;
	}

	return newLevel;
}

// ------------------------------------------------------------------------------------
BOOL	getRevealStatus( void )
{
	return(bRevealActive);
}

// ------------------------------------------------------------------------------------
void	setRevealStatus( BOOL val )
{
	debug(LOG_FOG, "avSetRevealStatus: Setting reveal to %s", val ? "ON" : "OFF");
	bRevealActive = val;
}

// ------------------------------------------------------------------------------------
void	preProcessVisibility( void )
{
	uint32_t		i, j;
	MAPTILE		*psTile;

	for(i = 0; i < mapWidth; i++)
	{
		for(j = 0; j < mapHeight; j++)
		{
			psTile = mapTile(i, j);
			psTile->level = 0;
			if(TEST_TILE_VISIBLE(selectedPlayer, psTile))
			{
				processAVTile(i, j, uint8_t_MAX);
				SET_TILE_EXPLORED(psTile);
			}
			else
			{
				CLEAR_TILE_EXPLORED(psTile);
				psTile->level = 0;
			}
		}
	}


}
// ------------------------------------------------------------------------------------
