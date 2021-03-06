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
 * @file raycast.c
 *
 * Raycasting routine that gives intersection points with map tiles.
 *
 */

#include "lib/framework/frame.h"

#include "raycast.h"
#include "map.h"
#include "display3d.h" // TILE_SIZE and clipXY()


typedef struct
{
	const int height;
	float pitch;
} HeightCallbackHelp_t;

typedef struct
{
	const int minDist, origHeight;
	int highestHeight;
	float pitch;
} HighestCallbackHelp_t;


void rayCast(Vector3i src, Vector3i direction, int length,
			 RAY_CALLBACK callback, void *data)
{
	unsigned int lengthSq = (length == RAY_MAXLEN ? RAY_MAXLEN : length * length), currLengthSq;
	Vector3i curr = src, diff;
	Vector3i step = Vector3f_To3i(
						Vector3f_Mult(
							Vector3f_Normalise(Vector3i_To3f(direction)),
							sqrtf(TILE_SIZE)
						)
					);

	while (curr = Vector3i_Add(curr, step),
			diff = Vector3i_Sub(curr, src),
			currLengthSq = Vector3i_ScalarP(diff, diff),
			currLengthSq < lengthSq)
	{
		// stop at the edge of the map
		if (curr.x < 0 || curr.x >= world_coord(mapWidth) ||
				curr.y < 0 || curr.y >= world_coord(mapHeight))
		{
			return;
		}

		if (!callback(curr, currLengthSq, data))
		{
			// callback doesn't want any more points so return
			return;
		}
	}
}


/*!
 * Gets the maximum terrain height along a certain direction to the edge of the grid
 * from wherever you specify, as well as the distance away
 */
static bool getTileHighestCallback(Vector3i pos, int distSq, void *data)
{
	HighestCallbackHelp_t *help = data;

	if (clipXY(pos.x, pos.y))
	{
		int height = map_Height(pos.x, pos.y), dist = sqrtf(distSq);
		if (height > help->highestHeight && dist >= help->minDist)
		{
			int heightDif = height - help->origHeight;
			help->pitch = rad2degf(atan2f(heightDif, world_coord(6)));// (float)(dist - world_coord(3))));
			help->highestHeight = height;
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------
/* Will return false when we've hit the edge of the grid */
static bool getTileHeightCallback(Vector3i pos, int distSq, void *data)
{
	HeightCallbackHelp_t *help = data;
#ifdef TEST_RAY
	Vector3i effect;
#endif

	/* Are we still on the grid? */
	if (clipXY(pos.x, pos.y))
	{
		int dist = sqrtf(distSq);
		bool HasTallStructure = TileHasTallStructure(mapTile(map_coord(pos.x), map_coord(pos.y)));

		if (dist > TILE_UNITS || HasTallStructure)
		{
			// Only do it the current tile is > TILE_UNITS away from the starting tile. Or..
			// there is a tall structure  on the current tile and the current tile is not the starting tile.
			/* Get height at this intersection point */
			int height = map_Height(pos.x, pos.y), heightDiff;
			float newPitch;

			if (HasTallStructure)
			{
				height += TALLOBJECT_ADJUST;
			}

			if (height <= help->height)
			{
				heightDiff = 0;
			}
			else
			{
				heightDiff = height - help->height;
			}

			/* Work out the angle to this point from start point */
			newPitch = rad2degf(atan2f(heightDiff, dist));

			/* Is this the steepest we've found? */
			if (newPitch > help->pitch)
			{
				/* Yes, then keep a record of it */
				help->pitch = newPitch;
			}
			//---

#ifdef TEST_RAY
			effect.x = pos.x;
			effect.y = height;
			effect.z = pos.y;
			addEffect(&effect, EFFECT_EXPLOSION, EXPLOSION_TYPE_SMALL, false, NULL, 0);
#endif
		}

		/* Not at edge yet - so exit */
		return true;
	}

	/* We've hit edge of grid - so exit!! */
	return false;
}

void getBestPitchToEdgeOfGrid(uint32_t x, uint32_t y, uint32_t direction, int32_t *pitch)
{
	Vector3i pos = { x, y, 0 };
	Vector3i dir = rayAngleToVector3i(direction);
	HeightCallbackHelp_t help = { map_Height(x, y), 0.0f };

	rayCast(pos, dir, 5430, getTileHeightCallback, &help); // FIXME Magic value

	*pitch = help.pitch;
}

void getPitchToHighestPoint( uint32_t x, uint32_t y, uint32_t direction,
							 uint32_t thresholdDistance, int32_t *pitch)
{
	Vector3i pos = { x, y, 0 };
	Vector3i dir = rayAngleToVector3i(direction);
	HighestCallbackHelp_t help = { thresholdDistance, map_Height(x, y), map_Height(x, y), 0.0f };

	rayCast(pos, dir, 3000, getTileHighestCallback, &help); // FIXME Magic value

	*pitch = help.pitch;
}
