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
/***************************************************************************/
/*
 * piedef.h
 *
 * type defines for all pumpkin image library functions.
 *
 */
/***************************************************************************/

#ifndef _piedef_h
#define _piedef_h

/***************************************************************************/

#include "lib/framework/frame.h"
#include "lib/framework/vector.h"
#include "ivisdef.h"
#include "ivispatch.h"

/***************************************************************************/
/*
 *	Global Definitions (CONSTANTS)
 */
/***************************************************************************/
#define STRETCHED_Z_SHIFT		10 // stretchs z range for (1000 to 4000) to (8000 to 32000)
#define MAX_Z				(32000) // raised to 32000 from 6000 when stretched
#define MIN_STRETCHED_Z			256
#define LONG_WAY			(1<<15)
#define INTERFACE_DEPTH		(MAX_Z - 1)
#define BUTTON_DEPTH		2000 // will be stretched to 16000

#define OLD_TEXTURE_SIZE_FIX 256.0f

//Render style flags for all pie draw functions
#define pie_TRANSLUCENT         0x2
#define pie_ADDITIVE            0x4
#define pie_NO_BILINEAR         0x8
#define pie_HEIGHT_SCALED       0x10
#define pie_RAISE               0x20
#define pie_BUTTON              0x40
#define pie_SHADOW              0x80
#define pie_STATIC_SHADOW       0x100

#define pie_RAISE_SCALE			256

#define pie_MAX_VERTICES		768
#define pie_MAX_POLYGONS		512
#define pie_MAX_VERTICES_PER_POLYGON	6

/***************************************************************************/
/*
 *	Global Definitions (STRUCTURES)
 */
/***************************************************************************/

typedef struct
{
	uint8_t r, g, b, a;
} PIELIGHTint8_tS;

/** Our basic colour type. Use whenever you want to define a colour.
 *  Set bytes separetely, and do not assume a byte order between the components. */
typedef union
{
	PIELIGHTint8_tS byte;
	uint32_t rgba;
	uint8_t vector[4];
} PIELIGHT;

typedef struct
{
	Vector3i pos;
	float u, v;
	PIELIGHT light;
	Vector3i screen; //! Screenspace tile coordinates
} TERRAIN_VERTEX;

typedef struct
{
	int16_t x, y, w, h;
} PIERECT;				/**< Screen rectangle. */
typedef struct
{
	int32_t texPage;
	int16_t tu, tv, tw, th;
} PIEIMAGE;	/**< An area of texture. */

/***************************************************************************/
/*
 *	Global ProtoTypes
 */
/***************************************************************************/
extern void pie_Draw3DShape(iIMDShape *shape, int frame, int team, PIELIGHT colour, PIELIGHT specular, int pieFlag, int pieData);
extern void pie_DrawImage(const PIEIMAGE *image, const PIERECT *dest);

void pie_TerrainInit(int sizex, int sizey);
void pie_TerrainCleanup(void);
void pie_DrawTerrain(int x1, int y1, int x2, int y2);
void pie_DrawTerrainTriangle(int x, int y, int triangle, const TERRAIN_VERTEX *aVrts);
void pie_DrawWaterTriangle(const TERRAIN_VERTEX *aVrts);

extern void pie_GetResetCounts(unsigned int *pPieCount, unsigned int *pTileCount, unsigned int *pPolyCount, unsigned int *pStateCount);

/** Setup stencil shadows and OpenGL lighting. */
void pie_BeginLighting(const Vector3f *light);

/* Stop using stencil shadows and OpenGL lighting (if enabled). */
void pie_EndLighting(void);

void pie_RemainingPasses(void);

void pie_CleanUp( void );

#endif // _piedef_h
