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
 * @file lighting.c
 * Calculates the shading values for the terrain world.
 * The terrain intensity values are calculated at map load/creation time.
 * - Alex McLean, Pumpkin Studios, EIDOS Interactive.
 */

#include "lib/framework/frame.h"
#include "lib/framework/math_ext.h"

#include "lib/ivis_common/piestate.h"
#include "lib/ivis_opengl/piematrix.h"
#include "lib/framework/fixedpoint.h"

#include "lib/gamelib/gtime.h"

#include "map.h"
#include "lighting.h"
#include "display3d.h"
#include "effects.h"
#include "atmos.h"
#include "environ.h"


// These values determine the fog when fully zoomed in
// Determine these when fully zoomed in
#define FOG_DEPTH 1000
#define FOG_END 6500

// These values are multiplied by the camera distance
// to obtain the optimal settings when fully zoomed out
// Determine these when fully zoomed out
#define FOG_BEGIN_SCALE 0.7
#define FOG_END_SCALE 1.2

/*	The vector that holds the sun's lighting direction - planar */
static Vector3f theSun;
uint32_t fogStatus = 0;

/*	Module function Prototypes */
static void colourTile(int32_t xIndex, int32_t yIndex, LIGHT_COLOUR colour, uint8_t percent);
static uint32_t calcDistToTile(uint32_t tileX, uint32_t tileY, Vector3i *pos);
static void calcTileIllum(uint32_t tileX, uint32_t tileY);

void setTheSun(Vector3f newSun)
{
	theSun = Vector3f_Mult(Vector3f_Normalise(newSun), FP12_MULTIPLIER);
}

Vector3f getTheSun(void)
{
	return theSun;
}

/*****************************************************************************/
/*
 * SOURCE
 */
/*****************************************************************************/

//By passing in params - it means that if the scroll limits are changed mid-mission
//we can re-do over the area that hasn't been seen
void initLighting(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
	uint32_t       i, j;

	// quick check not trying to go off the map - don't need to check for < 0 since uint16_t's!!
	if (x1 > mapWidth || x2 > mapWidth || y1 > mapHeight || y2 > mapHeight)
	{
		ASSERT( false, "initLighting: coords off edge of map" );
		return;
	}

	for (i = x1; i < x2; i++)
	{
		for(j = y1; j < y2; j++)
		{
			MAPTILE	*psTile = mapTile(i, j);

			// always make the edge tiles dark
			if (i == 0 || j == 0 || i >= mapWidth - 1 || j >= mapHeight - 1)
			{
				psTile->illumination = 16;

				// give water tiles at edge of map a border
				if (terrainType(psTile) == TER_WATER)
				{
					psTile->texture = 0;
				}
			}
			else
			{
				calcTileIllum(i, j);
			}
			// Basically darkens down the tiles that are outside the scroll
			// limits - thereby emphasising the cannot-go-there-ness of them
			if ((int32_t)i < scrollMinX + 4 || (int32_t)i > scrollMaxX - 4
					|| (int32_t)j < scrollMinY + 4 || (int32_t)j > scrollMaxY - 4)
			{
				psTile->illumination /= 3;
			}
		}
	}
}


static void normalsOnTile(unsigned int tileX, unsigned int tileY, unsigned int quadrant, unsigned int *numNormals, Vector3f normals[])
{
	/* Get a pointer to our tile */
	/* And to the ones to the east, south and southeast of it */
	MAPTILE
	*psTile = mapTile(tileX, tileY),
	 *tileRight = mapTile(tileX + 1, tileY),
	  *tileDownRight = mapTile(tileX + 1, tileY + 1),
	   *tileDown = mapTile(tileX, tileY + 1);
	unsigned int rMod = 0, drMod = 0, dMod = 0, nMod = 0;

	if (terrainType(psTile) == TER_WATER)
	{
		nMod = 100 + (2 * environGetData(tileX, tileY));
		rMod = 100 + (2 * environGetData(tileX + 1, tileY));
		drMod = 100 + (2 * environGetData(tileX + 1, tileY + 1));
		dMod = 100 + (2 * environGetData(tileX, tileY + 1));
	}

	switch(quadrant)
	{
		case 0:
		case 2:
			/* Is it flipped? In this case one triangle  */
			if(TRI_FLIPPED(psTile))
			{
				if(quadrant == 0)
				{
					Vector3f
					corner1 =
					{
						world_coord(tileX + 1),
						world_coord(tileY),
						tileRight->height *ELEVATION_SCALE - rMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY + 1),
						tileDownRight->height *ELEVATION_SCALE - drMod
					},
					corner3 =
					{
						world_coord(tileX),
						world_coord(tileY + 1),
						tileDown->height *ELEVATION_SCALE - dMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}
				else
				{
					Vector3f
					corner1 =
					{
						world_coord(tileX),
						world_coord(tileY),
						psTile->height *ELEVATION_SCALE - nMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY),
						tileRight->height *ELEVATION_SCALE - rMod
					},
					corner3 =
					{
						world_coord(tileX),
						world_coord(tileY + 1),
						tileDown->height *ELEVATION_SCALE - dMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}
			}
			else
			{
				/* Otherwise, it's not flipped and so two triangles*/
				{
					// MSVC hack
					Vector3f
					corner1 =
					{
						world_coord(tileX),
						world_coord(tileY),
						psTile->height *ELEVATION_SCALE - nMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY),
						tileRight->height *ELEVATION_SCALE - rMod
					},
					corner3 =
					{
						world_coord(tileX + 1),
						world_coord(tileY + 1),
						tileDownRight->height *ELEVATION_SCALE - drMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}

				{
					// MSVC hack
					Vector3f
					corner1 =
					{
						world_coord(tileX),
						world_coord(tileY),
						psTile->height *ELEVATION_SCALE - nMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY + 1),
						tileDownRight->height *ELEVATION_SCALE - drMod
					},
					corner3 =
					{
						world_coord(tileX),
						world_coord(tileY + 1),
						tileDown->height *ELEVATION_SCALE - dMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}
			}
			break;
		case 1:
		case 3:
			/* Is it flipped? In this case two triangles  */
			if(TRI_FLIPPED(psTile))
			{
				{
					// MSVC hack
					Vector3f
					corner1 =
					{
						world_coord(tileX),
						world_coord(tileY),
						psTile->height *ELEVATION_SCALE - nMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY),
						tileRight->height *ELEVATION_SCALE - rMod
					},
					corner3 =
					{
						world_coord(tileX),
						world_coord(tileY + 1),
						tileDown->height *ELEVATION_SCALE - dMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}

				{
					// MSVC hack
					Vector3f
					corner1 =
					{
						world_coord(tileX + 1),
						world_coord(tileY),
						tileRight->height *ELEVATION_SCALE - rMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY + 1),
						tileDownRight->height *ELEVATION_SCALE - drMod
					},
					corner3 =
					{
						world_coord(tileX),
						world_coord(tileY + 1),
						tileDown->height *ELEVATION_SCALE - dMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}
			}
			else
			{
				if(quadrant == 1)
				{
					Vector3f
					corner1 =
					{
						world_coord(tileX),
						world_coord(tileY),
						psTile->height *ELEVATION_SCALE - nMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY + 1),
						tileDownRight->height *ELEVATION_SCALE - drMod
					},
					corner3 =
					{
						world_coord(tileX),
						world_coord(tileY + 1),
						tileDown->height *ELEVATION_SCALE - dMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}
				else
				{
					Vector3f
					corner1 =
					{
						world_coord(tileX),
						world_coord(tileY),
						psTile->height *ELEVATION_SCALE - nMod
					},
					corner2 =
					{
						world_coord(tileX + 1),
						world_coord(tileY),
						tileRight->height *ELEVATION_SCALE - rMod
					},
					corner3 =
					{
						world_coord(tileX + 1),
						world_coord(tileY + 1),
						tileDownRight->height *ELEVATION_SCALE - drMod
					};

					normals[(*numNormals)++] = pie_SurfaceNormal3fv( corner1, corner2, corner3);
				}
			}
			break;
		default:
			ASSERT( false, "Invalid quadrant in lighting code" );
	} // end switch
}


static void calcTileIllum(uint32_t tileX, uint32_t tileY)
{
	/* The number or normals that we got is in numNormals*/
	Vector3f finalVector = {0.0f, 0.0f, 0.0f};
	const Vector3f tileSun = { theSun.x, theSun.y, -theSun.z }; /*For matching tile and object shadowing.*/
	unsigned int i, val;
	int dotProduct[2];

	unsigned int numNormals = 0; // How many normals have we got?
	Vector3f normals[8]; // Maximum 8 possible normals

	/* Quadrants look like:-

				  *
				  *
			0	  *    1
				  *
				  *
		**********V**********
				  *
				  *
			3	  *	   2
				  *
				  *
	*/

	/* Do quadrant 0 - tile that's above and left*/
	normalsOnTile(tileX - 1, tileY - 1, 0, &numNormals, normals);

	/* Do quadrant 1 - tile that's above and right*/
	normalsOnTile(tileX, tileY - 1, 1, &numNormals, normals);

	/* Do quadrant 2 - tile that's down and right*/
	normalsOnTile(tileX, tileY, 2, &numNormals, normals);

	/* Do quadrant 3 - tile that's down and left*/
	normalsOnTile(tileX - 1, tileY, 3, &numNormals, normals);

	for(i = 0; i < numNormals; i++)
	{
		finalVector = Vector3f_Add(finalVector, normals[i]);
	}

	dotProduct[0] = Vector3f_ScalarP(Vector3f_Normalise(finalVector), tileSun);
	dotProduct[1] = Vector3f_ScalarP(Vector3f_Normalise(finalVector), theSun);

	val = abs(dotProduct[0]) / 16;
	if (val == 0)
	{
		val = 1;
	}
	else if (val > 254)
	{
		val = 254;
	}
	mapTile(tileX, tileY)->illumination = val;

	val = abs(dotProduct[1]) / 16;
	if (val == 0)
	{
		val = 1;
	}
	else if (val > 254)
	{
		val = 254;
	}
	mapTile(tileX, tileY)->radarIllumination = val;
}


void	processLight(LIGHT *psLight)
{
	int32_t	tileX, tileY;
	int32_t	startX, endX;
	int32_t	startY, endY;
	int32_t	rangeSkip;
	int32_t	i, j;
	int32_t	distToCorner;
	uint32_t	percent;

	/* Firstly - there's no point processing lights that are off the grid */
	if(clipXY(psLight->position.x, psLight->position.z) == false)
	{
		return;
	}

	tileX = psLight->position.x / TILE_UNITS;
	tileY = psLight->position.z / TILE_UNITS;

	rangeSkip = sqrtf(psLight->range * psLight->range * 2) / TILE_UNITS + 1;

	/* Rough guess? */
	startX = tileX - rangeSkip;
	endX = tileX + rangeSkip;
	startY = tileY - rangeSkip;
	endY = tileY + rangeSkip;

	/* Clip to grid limits */
	startX = MAX(startX, 0);
	endX = MAX(endX, 0);
	endX = MIN(endX, mapWidth - 1);
	startX = MIN(startX, endX);
	startY = MAX(startY, 0);
	endY = MAX(endY, 0);
	endY = MIN(endY, mapHeight - 1);
	startY = MIN(startY, endY);

	for(i = startX; i <= endX; i++)
	{
		for(j = startY; j <= endY; j++)
		{
			distToCorner = calcDistToTile(i, j, &psLight->position);

			/* If we're inside the range of the light */
			if (distToCorner < (int32_t)psLight->range)
			{
				/* Find how close we are to it */
				percent = 100 - PERCENT(distToCorner, psLight->range);
				colourTile(i, j, psLight->colour, 2 * percent);
			}
		}
	}
}


static uint32_t calcDistToTile(uint32_t tileX, uint32_t tileY, Vector3i *pos)
{
	uint32_t	x1, y1, z1;
	uint32_t	x2, y2, z2;
	uint32_t	xDif, yDif, zDif;
	uint32_t	total;

	/* The coordinates of the tile corner */
	x1 = tileX * TILE_UNITS;
	y1 = map_TileHeight(tileX, tileY);
	z1 = tileY * TILE_UNITS;

	/* The coordinates of the position */
	x2 = pos->x;
	y2 = pos->y;
	z2 = pos->z;

	xDif = abs(x1 - x2);
	yDif = abs(y1 - y2);
	zDif = abs(z1 - z2);

	total = (xDif * xDif) + (yDif * yDif) + (zDif * zDif);
	return (uint32_t)sqrtf(total);
}

// FIXME: Is the percent variable misnamed here, or is the code wrong? Because we do
// not use it as a percentage!
static void colourTile(int32_t xIndex, int32_t yIndex, LIGHT_COLOUR colouridx, uint8_t percent)
{
	PIELIGHT colour = getTileColour(xIndex, yIndex);

	switch(colouridx)
	{
		case LIGHT_RED:
			/* And add that to the lighting value */
			colour.byte.r = MIN(255, colour.byte.r + percent);
			break;
		case LIGHT_GREEN:
			/* And add that to the lighting value */
			colour.byte.g = MIN(255, colour.byte.g + percent);
			break;
		case LIGHT_BLUE:
			/* And add that to the lighting value */
			colour.byte.b = MIN(255, colour.byte.b + percent);
			break;
		case LIGHT_YELLOW:
			/* And add that to the lighting value */
			colour.byte.r = MIN(255, colour.byte.r + percent);
			colour.byte.g = MIN(255, colour.byte.g + percent);
			break;
		case LIGHT_WHITE:
			colour.byte.r = MIN(255, colour.byte.r + percent);
			colour.byte.g = MIN(255, colour.byte.g + percent);
			colour.byte.b = MIN(255, colour.byte.b + percent);
			break;
		default:
			ASSERT( false, "Weirdy colour of light attempted" );
			break;
	}
	setTileColour(xIndex, yIndex, colour);
}

/// Sets the begin and end distance for the distance fog (mist)
/// It should provide maximum visiblitiy and minimum
/// "popping" tiles
void UpdateFogDistance(float distance)
{
	pie_UpdateFogDistance(FOG_END - FOG_DEPTH + distance * FOG_BEGIN_SCALE, FOG_END + distance * FOG_END_SCALE);
}


#define MIN_DROID_LIGHT_LEVEL	96
#define	DROID_SEEK_LIGHT_SPEED	2

void	calcDroidIllumination(DROID *psDroid)
{
	uint32_t	lightVal;	// sum of light vals
	uint32_t	presVal;
	uint32_t	tileX, tileY;
	uint32_t	retVal;
	float	adjust;

	/* See if the droid's at the edge of the map */
	tileX = psDroid->pos.x / TILE_UNITS;
	tileY = psDroid->pos.y / TILE_UNITS;
	/* Are we at the edge */
	if(tileX <= 1 || tileX >= mapWidth - 2 || tileY <= 1 || tileY >= mapHeight - 2)
	{
		lightVal = mapTile(tileX, tileY)->illumination;
		lightVal += MIN_DROID_LIGHT_LEVEL;
	}
	else
	{
		lightVal = mapTile(tileX, tileY)->illumination +		 //
				   mapTile(tileX - 1, tileY)->illumination +	 //		 *
				   mapTile(tileX, tileY - 1)->illumination +	 //		***		pattern
				   mapTile(tileX + 1, tileY)->illumination +	 //		 *
				   mapTile(tileX + 1, tileY + 1)->illumination;	 //
		lightVal /= 5;
		lightVal += MIN_DROID_LIGHT_LEVEL;
	}

	/* Saturation */
	if(lightVal > 255)
	{
		lightVal = 255;
	}
	presVal = psDroid->illumination;
	adjust = (float)lightVal - (float)presVal;
	adjust *= timeAdjustedIncrement(DROID_SEEK_LIGHT_SPEED, true);
	retVal = presVal + adjust;
	if(retVal > 255)
	{
		retVal = 255;
	}
	psDroid->illumination = (uint8_t)retVal;
}

void	doBuildingLights( void )
{
	STRUCTURE	*psStructure;
	uint32_t	i;
	LIGHT	light;

	for(i = 0; i < MAX_PLAYERS; i++)
	{
		for(psStructure = apsStructLists[i]; psStructure; psStructure = psStructure->psNext)
		{
			light.range = psStructure->pStructureType->baseWidth * TILE_UNITS;
			light.position.x = psStructure->pos.x;
			light.position.z = psStructure->pos.y;
			light.position.y = map_Height(light.position.x, light.position.z);
			light.range = psStructure->pStructureType->baseWidth * TILE_UNITS;
			light.colour = LIGHT_WHITE;
			processLight(&light);
		}
	}
}


/* Experimental moving shadows code */
void	findSunVector( void )
{
	Vector3f val =
	{
		4096 - getStaticTimeValueRange(16384, 8192),
		0 - getStaticTimeValueRange(16384, 4096),
		4096 - getStaticTimeValueRange(49152, 8192)
	};

	setTheSun(val);
}
