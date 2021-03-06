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
/** @file
 *  Definitions for the map structure
 */

#ifndef __INCLUDED_SRC_MAP_H__
#define __INCLUDED_SRC_MAP_H__

#include "lib/framework/frame.h"
#include "lib/framework/debug.h"
#include "objects.h"
#include "multiplay.h"
#include "display.h"

/* The different types of terrain as far as the game is concerned */
typedef enum _terrain_type
{
	TER_SAND,
	TER_SANDYBRUSH,
	TER_BAKEDEARTH,
	TER_GREENMUD,
	TER_REDBRUSH,
	TER_PINKROCK,
	TER_ROAD,
	TER_WATER,
	TER_CLIFFFACE,
	TER_RUBBLE,
	TER_SHEETICE,
	TER_SLUSH,

	TER_MAX,
} TYPE_OF_TERRAIN;

#define TILESET_ARIZONA 0
#define TILESET_URBAN	1
#define TILESET_ROCKIES 2

#define TALLOBJECT_YMAX		(200)
#define TALLOBJECT_ADJUST	(300)

/* Flags for whether texture tiles are flipped in X and Y or rotated */
#define TILE_XFLIP		0x8000
#define TILE_YFLIP		0x4000
#define TILE_ROTMASK	0x3000
#define TILE_ROTSHIFT	12
#define TILE_TRIFLIP	0x0800	// This bit describes the direction the tile is split into 2 triangles (same as triangleFlip)
#define TILE_HILIGHT	0x0400	// set when the tile has the structure cursor over it

#define TILE_NUMMASK	0x01ff


static inline unsigned short TileNumber_tile(unsigned short tilenumber)
{
	return tilenumber & TILE_NUMMASK;
}


static inline unsigned short TileNumber_texture(unsigned short tilenumber)
{
	return tilenumber & ~TILE_NUMMASK;
}

#define BITS_NOTBLOCKING	0x01	///< Units can drive on this even if there is a structure or feature on it
#define BITS_EXPLORED		0x02	///< Tile has been explored
#define BITS_FPATHBLOCK		0x10	///< Bit set temporarily by find path to mark a blocking tile
#define BITS_ON_FIRE		0x20	///< Cache whether tile is burning
#define BITS_GATEWAY		0x40	///< Bit set to show a gateway on the tile
#define BITS_TALLSTRUCTURE	0x80	///< Bit set to show a tall structure which camera needs to avoid.

/* Information stored with each tile */
typedef struct _maptile
{
	uint8_t			tileInfoBits;
	uint8_t			tileVisBits;	// COMPRESSED - bit per player
	uint8_t			height;			// The height at the top left of the tile
	uint8_t			illumination;	// How bright is this tile?
	uint8_t			radarIllumination; // Tile Illumination inverted to show proper heights on minimap.
	uint16_t			texture;		// Which graphics texture is on this tile
	uint8_t			watchers[MAX_PLAYERS];		// player sees through fog of war here with this many objects
	float			level;
	BASE_OBJECT		*psObject;		// Any object sitting on the location (e.g. building)
	PIELIGHT		colour;
	short			limitedContinent;	/** For land or sea limited propulsion types */
	short			hoverContinent;		/** For hover type propulsions */
	uint16_t firecount; // how many fire effects are on this tile?

//	TYPE_OF_TERRAIN	type;			// The terrain type for the tile
} MAPTILE;


/**
 * Check if tile contains a structure or feature. Function is thread-safe,
 * but do not rely on the result if you mean to alter the object pointer.
 */
static inline bool TileIsOccupied(const MAPTILE *tile)
{
	return tile->psObject != NULL;
}

/** Check if tile contains a structure. Function is NOT thread-safe. */
static inline bool TileHasStructure(const MAPTILE *tile)
{
	return TileIsOccupied(tile)
		   && tile->psObject->type == OBJ_STRUCTURE;
}

/** Check if tile contains a feature. Function is NOT thread-safe. */
static inline bool TileHasFeature(const MAPTILE *tile)
{
	return TileIsOccupied(tile)
		   && tile->psObject->type == OBJ_FEATURE;
}

/** Check if tile contains a wall structure. Function is NOT thread-safe. */
static inline bool TileHasWall(const MAPTILE *tile)
{
	return TileHasStructure(tile)
		   && (((STRUCTURE *)tile->psObject)->pStructureType->type == REF_WALL
			   || ((STRUCTURE *)tile->psObject)->pStructureType->type == REF_WALLCORNER);
}

/** Check if tile is burning. */
static inline bool TileIsBurning(const MAPTILE *tile)
{
	return tile->tileInfoBits & BITS_ON_FIRE;
}

/** Check if tile has been explored. */
static inline bool tileIsExplored(const MAPTILE *psTile)
{
	return psTile->tileInfoBits & BITS_EXPLORED;
}

/** Check if tile is highlighted by the user. Function is thread-safe. */
static inline bool TileIsHighlighted(const MAPTILE *tile)
{
	return tile->texture & TILE_HILIGHT;
}

/** Check if tile is not blocking, even if structure or feature on it */
static inline bool TileIsNotBlocking(const MAPTILE *tile)
{
	return tile->tileInfoBits & BITS_NOTBLOCKING;
}

/** Check if tile contains a tall structure. Function is thread-safe. */
static inline WZ_DECL_PURE bool TileHasTallStructure(const MAPTILE *tile)
{
	return tile->tileInfoBits & BITS_TALLSTRUCTURE;
}

/** Check if tile contains a small structure. Function is NOT thread-safe. */
static inline bool TileHasSmallStructure(const MAPTILE *tile)
{
	return TileHasStructure(tile)
		   && ((STRUCTURE *)tile->psObject)->pStructureType->height == 1;
}

#define SET_TILE_EXPLORED(x) ((x)->tileInfoBits |= BITS_EXPLORED)
#define CLEAR_TILE_EXPLORED(x) ((x)->tileInfoBits &= ~BITS_EXPLORED)

#define SET_TILE_NOTBLOCKING(x)	((x)->tileInfoBits |= BITS_NOTBLOCKING)
#define CLEAR_TILE_NOTBLOCKING(x)	((x)->tileInfoBits &= ~BITS_NOTBLOCKING)

#define SET_TILE_HIGHLIGHT(x)	((x)->texture |= TILE_HILIGHT)
#define CLEAR_TILE_HIGHLIGHT(x)	((x)->texture &= ~TILE_HILIGHT)

#define SET_TILE_TALLSTRUCTURE(x)	((x)->tileInfoBits |= BITS_TALLSTRUCTURE)
#define CLEAR_TILE_TALLSTRUCTURE(x)	((x)->tileInfoBits &= ~BITS_TALLSTRUCTURE)

// Multiplier for the tile height
#define	ELEVATION_SCALE	2

/* Allows us to do if(TRI_FLIPPED(psTile)) */
#define TRI_FLIPPED(x)		((x)->texture & TILE_TRIFLIP)
/* Flips the triangle partition on a tile pointer */
#define TOGGLE_TRIFLIP(x)	((x)->texture ^= TILE_TRIFLIP)

/* Can player number p see tile t? */
#define TEST_TILE_VISIBLE(p,t)	((t)->tileVisBits & (1<<(p)))

/* Set a tile to be visible for a player */
#define SET_TILE_VISIBLE(p,t) ((t)->tileVisBits |= 1<<(p))

/* Arbitrary maximum number of terrain textures - used in look up table for terrain type */
#define MAX_TILE_TEXTURES	255

extern uint8_t terrainTypes[MAX_TILE_TEXTURES];

static inline unsigned char terrainType(const MAPTILE *tile)
{
	return terrainTypes[TileNumber_tile(tile->texture)];
}


/* The maximum map size */

#define MAP_MAXWIDTH	256
#define MAP_MAXHEIGHT	256
#define MAP_MAXAREA		(256*256)

#define TILE_MAX_HEIGHT (255 * ELEVATION_SCALE)
#define TILE_MIN_HEIGHT 0

/* The size and contents of the map */
extern uint32_t	mapWidth, mapHeight;
extern MAPTILE *psMapTiles;

/*
 * Usage-Example:
 * tile_coordinate = (world_coordinate / TILE_UNITS) = (world_coordinate >> TILE_SHIFT)
 * world_coordinate = (tile_coordinate * TILE_UNITS) = (tile_coordinate << TILE_SHIFT)
 */

/* The shift on a world coordinate to get the tile coordinate */
#define TILE_SHIFT 7

/* The mask to get internal tile coords from a full coordinate */
#define TILE_MASK 0x7f

/* The number of units accross a tile */
#define TILE_UNITS (1<<TILE_SHIFT)

static inline int32_t world_coord(int32_t mapCoord)
{
	return mapCoord << TILE_SHIFT;
}

static inline int32_t map_coord(int32_t worldCoord)
{
	return worldCoord >> TILE_SHIFT;
}

/* Make sure world coordinates are inside the map */
/** Clip world coordinates to make sure they're inside the map's boundaries
 *  \param worldX a pointer to a X coordinate inside the map
 *  \param worldY a pointer to a Y coordinate inside the map
 *  \post 1 <= *worldX <= world_coord(mapWidth)-1 and
 *        1 <= *worldy <= world_coord(mapHeight)-1
 */
static inline void clip_world_offmap(int *worldX, int *worldY)
{
	// x,y must be > 0
	*worldX = MAX(1, *worldX);
	*worldY = MAX(1, *worldY);
	*worldX = MIN(world_coord(mapWidth) - 1, *worldX);
	*worldY = MIN(world_coord(mapHeight) - 1, *worldY);
}

/* maps a position down to the corner of a tile */
#define map_round(coord) ((coord) & (TILE_UNITS - 1))

/* Shutdown the map module */
extern BOOL mapShutdown(void);

/* Create a new map of a specified size */
extern BOOL mapNew(uint32_t width, uint32_t height);

/* Load the map data */
extern BOOL mapLoad(char *pFileData, uint32_t fileSize);

/* Save the map data */
extern BOOL mapSave(char **ppFileData, uint32_t *pFileSize);

/* Return a pointer to the tile structure at x,y */
static inline WZ_DECL_PURE MAPTILE *mapTile(int32_t x, int32_t y)
{
	// Clamp x and y values to actual ones
	// Give one tile worth of leeway before asserting, for units/transporters coming in from off-map.
	ASSERT(x >= -1, "mapTile: x value is too small (%d,%d) in %dx%d", x, y, mapWidth, mapHeight);
	ASSERT(y >= -1, "mapTile: y value is too small (%d,%d) in %dx%d", x, y, mapWidth, mapHeight);
	x = (x < 0 ? 0 : x);
	y = (y < 0 ? 0 : y);
	ASSERT(x < mapWidth + 1, "mapTile: x value is too big (%d,%d) in %dx%d", x, y, mapWidth, mapHeight);
	ASSERT(y < mapHeight + 1, "mapTile: y value is too big (%d,%d) in %dx%d", x, y, mapWidth, mapHeight);
	x = (x >= mapWidth ? mapWidth - 1 : x);
	y = (y >= mapHeight ? mapHeight - 1 : y);

	return &psMapTiles[x + (y * mapWidth)];
}

/* Return height of tile at x,y */
static inline WZ_DECL_PURE int16_t map_TileHeight(uint32_t x, uint32_t y)
{
	if ( x >= mapWidth || y >= mapHeight )
	{
		return 0;
	}
	return (int16_t)(psMapTiles[x + (y * mapWidth)].height * ELEVATION_SCALE);
}

/*sets the tile height */
static inline void setTileHeight(uint32_t x, uint32_t y, uint32_t height)
{
	ASSERT_OR_RETURN( , x < mapWidth, "x coordinate %u bigger than map width %u", x, mapWidth);
	ASSERT_OR_RETURN( , y < mapHeight, "y coordinate %u bigger than map height %u", y, mapHeight);

	psMapTiles[x + (y * mapWidth)].height = (uint8_t) (height / ELEVATION_SCALE);
}

/* Return whether a tile coordinate is on the map */
static inline BOOL tileOnMap(int32_t x, int32_t y)
{
	return (x >= 0) && (x < (int32_t)mapWidth) && (y >= 0) && (y < (int32_t)mapHeight);
}

/* Return true if a tile is not too near the map edge and not outside of the map */
static inline BOOL tileInsideBuildRange(int32_t x, int32_t y)
{
	return (x >= TOO_NEAR_EDGE) && (x < ((int32_t)mapWidth - TOO_NEAR_EDGE)) &&
		   (y >= TOO_NEAR_EDGE) && (y < ((int32_t)mapHeight - TOO_NEAR_EDGE));
}

/* Return whether a world coordinate is on the map */
static inline BOOL worldOnMap(int x, int y)
{
	return (x >= 0) && (x < ((int32_t)mapWidth << TILE_SHIFT)) &&
		   (y >= 0) && (y < ((int32_t)mapHeight << TILE_SHIFT));
}


/* Return whether a world coordinate is on the map */
static inline bool worldOnMap2i(Vector2i pos)
{
	return worldOnMap(pos.x, pos.y);
}


/* Return whether a world coordinate is on the map */
static inline bool worldOnMap3i(Vector3i pos)
{
	return worldOnMap(pos.x, pos.y);
}


/* Return whether a world coordinate is on the map */
static inline bool worldOnMap3f(Vector3f pos)
{
	return worldOnMap(pos.x, pos.y);
}


/* Store a map coordinate and it's associated tile */
typedef struct _tile_coord
{
	uint32_t	x, y;
	MAPTILE	*psTile;
} TILE_COORD;

/* Return height of x,y */
extern int16_t map_Height(int x, int y);

/* returns true if object is above ground */
extern BOOL mapObjIsAboveGround( BASE_OBJECT *psObj );

/* returns the max and min height of a tile by looking at the four corners
   in tile coords */
extern void getTileMaxMin(uint32_t x, uint32_t y, uint32_t *pMax, uint32_t *pMin);

uint32_t GetHeightOfMap(void);
uint32_t GetWidthOfMap(void);
extern bool readVisibilityData(const char *fileName);
extern bool	writeVisibilityData(const char *fileName);

//scroll min and max values
extern int32_t		scrollMinX, scrollMaxX, scrollMinY, scrollMaxY;

void mapFloodFillContinents(void);

extern void mapTest(void);

extern bool fireOnLocation(unsigned int x, unsigned int y);

/**
 * Transitive sensor check for tile. Has to be here rather than
 * visibility.h due to header include order issues.
 */
static inline bool hasSensorOnTile(MAPTILE *psTile, int player)
{
	int k;

	// if a player has a SAT_UPLINK structure, or has godMode enabled,
	// they can see everything!
	if (getSatUplinkExists(player) || (player == selectedPlayer && godMode))
	{
		return true;
	}

	if (psTile->watchers[selectedPlayer] == 0)
	{
		if (game.type != CAMPAIGN && game.alliance == ALLIANCES_TEAMS)
		{
			// Check if an ally can provide us with vision on this tile
			for (k = 0;  k < MAX_PLAYERS; k++)
			{
				if (aiCheckAlliances(k, selectedPlayer) && (getSatUplinkExists(k) || psTile->watchers[k] > 0))
				{
					return true;
				}
			}
		}
		return false;
	}
	return true;
}

#endif // __INCLUDED_SRC_MAP_H__
