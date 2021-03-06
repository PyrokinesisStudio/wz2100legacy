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
/* Geometry.c - holds trig/vector deliverance specific stuff for 3D */
/* Alex McLean, Pumpkin Studios, EIDOS Interactive */

#include "lib/framework/frame.h"

#include "lib/ivis_common/ivisdef.h"
#include "lib/ivis_opengl/piematrix.h"
#include "lib/gamelib/gtime.h"

#include "geometry.h"
#include "objectdef.h"
#include "map.h"
#include "display3d.h"
#include "hci.h"
#include "display.h"

#define AMPLITUDE_HEIGHT        100
#define SIZE_SINE_TABLE         100
#define deg (M_PI / SIZE_SINE_TABLE)

/* The arc over which bullets fly */
static uint8_t sineHeightTable[SIZE_SINE_TABLE];

void initBulletTable( void )
{
	uint32_t i;
	uint8_t height;

	for (i = 0; i < SIZE_SINE_TABLE; i++)
	{
		height = AMPLITUDE_HEIGHT * sin(i * deg);
		sineHeightTable[i] = height;
	}
}

/* Angle returned is reflected in line x=0 */
int32_t calcDirection(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
	/* Watch out here - should really be y1-y0, but coordinate system is reversed in Y */
	int32_t	xDif = (x1 - x0), yDif = (y0 - y1);
	double	angle = atan2(yDif, xDif) * 180.0 / M_PI;
	int32_t	angleInt = (int32_t) angle;

	angleInt += 90;
	if (angleInt < 0)
	{
		angleInt += 360;
	}

	ASSERT( angleInt >= 0 && angleInt < 360,
			"calcDirection: droid direction out of range" );

	return(angleInt);
}


/*	A useful function and one that should have been written long ago, assuming of course
	that is hasn't been!!!! Alex M, 24th Sept, 1998. Returns the nearest unit
	to a given world coordinate - we can choose whether we require that the unit be
	selected or not... Makes sending the most logical unit to do something very easy.

  NB*****THIS WON'T PICK A VTOL DROID*****
*/

DROID	*getNearestDroid(uint32_t x, uint32_t y, BOOL bSelected)
{
	DROID	*psDroid, *psBestUnit;
	uint32_t	xDif, yDif, dist;
	uint32_t	bestSoFar;

	/* Go thru' all the droids  - how often have we seen this - a MACRO maybe? */
	for(psDroid = apsDroidLists[selectedPlayer], psBestUnit = NULL, bestSoFar = uint32_t_MAX;
			psDroid; psDroid = psDroid->psNext)
	{
		if (!isVtolDroid(psDroid))
		{
			/* Clever (?) bit that reads whether we're interested in droids being selected or not */
			if( (bSelected ? psDroid->selected : true ) )
			{
				/* Get the differences */
				xDif = abs(psDroid->pos.x - x);
				yDif = abs(psDroid->pos.y - y);
				/* Approximates the distance away - using a sqrt approximation */
				dist = MAX(xDif, yDif) + MIN(xDif, yDif) / 2;	// approximates, but never more than 11% out...
				/* Is this the nearest one we got so far? */
				if(dist < bestSoFar)
				{
					/* Yes, then keep a record of the distance for comparison... */
					bestSoFar = dist;
					/* ..and store away the droid responsible */
					psBestUnit = psDroid;
				}
			}
		}
	}
	return(psBestUnit);
}
// -------------------------------------------------------------------------------------------

/* Returns non-zero if a point is in a 4 sided polygon */
/* See header file for definition of QUAD */
int inQuad(const Vector2i *pt, const QUAD *quad)
{
	int i, j, c = 0;

	for (i = 0, j = 3; i < 4; j = i++)
	{
		if (((quad->coords[i].y <= pt->y && pt->y < quad->coords[j].y) ||
				(quad->coords[j].y <= pt->y && pt->y < quad->coords[i].y)) &&
				(pt->x <
				 (quad->coords[j].x - quad->coords[i].x)
				 * (pt->y - quad->coords[i].y)
				 / (quad->coords[j].y - quad->coords[i].y)
				 + quad->coords[i].x))
		{
			c = !c;
		}
	}

	return c;
}

uint32_t	adjustDirection(int32_t present, int32_t difference)
{
	int32_t	sum;

	sum = present + difference;
	if(sum >= 0 && sum <= 360)
	{
		return(uint32_t)(sum);
	}

	if (sum < 0)
	{
		return(uint32_t)(360 + sum);
	}

	if (sum > 360)
	{
		return(uint32_t)(sum - 360);
	}
	return 0;
}

/**
 * Approximates the euclidian distance function, never moret than 11% out.
 *
 * Mathematically equivalent to sqrt(deltaX * deltaX + deltaY * deltaY).
 *
 * @Deprecated All uses of this function should be replaced by calls to hypot()
 *             or hypotf(), the C99 functions. This because this integer
 *             optimisation is no longer required (due to hardware improvements
 *             since 1997).
 */
unsigned int WZ_DECL_CONST dirtyHypot(int deltaX, int deltaY)
{
	deltaX = abs(deltaX);
	deltaY = abs(deltaY);

	return MAX(deltaX, deltaY) + MIN(deltaX, deltaY) / 2;
}

//-----------------------------------------------------------------------------------
BOOL	droidOnScreen( DROID *psDroid, int32_t tolerance )
{
	int32_t	dX, dY;

	if (DrawnInLastFrame(psDroid->sDisplay.frameNumber) == true)
	{
		dX = psDroid->sDisplay.screenX;
		dY = psDroid->sDisplay.screenY;
		/* Is it on screen */
		if(dX > (0 - tolerance) && dY > (0 - tolerance)
				&& dX < (int32_t)(pie_GetVideoBufferWidth() + tolerance)
				&& dY < (int32_t)(pie_GetVideoBufferHeight() + tolerance))
		{
			return(true);
		}
	}
	return(false);
}
