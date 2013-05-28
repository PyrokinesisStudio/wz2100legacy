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
 *  Definitions for projectiles.
 */

#ifndef __INCLUDED_PROJECTILEDEF_H__
#define __INCLUDED_PROJECTILEDEF_H__

#include "basedef.h"

typedef enum PROJ_STATE
{
	PROJ_INFLIGHTDIRECT,
	PROJ_INFLIGHTINDIRECT,
	PROJ_IMPACT,
	PROJ_POSTIMPACT
} PROJ_STATE;

typedef struct PROJECTILE
{
	/* Use only simple object elements */
	SIMPLE_ELEMENTS( struct PROJECTILE );

	UBYTE           state;                  ///< current projectile state

	UBYTE           player;                 ///< needed because damange and radDamage vary from base stat per player because of upgrades

	UBYTE           bVisible;               ///< whether the selected player should see the projectile

	WEAPON_STATS*   psWStats;               ///< firing weapon stats

	BASE_OBJECT*    psSource;               ///< what fired the projectile
	BASE_OBJECT*    psDest;                 ///< target of this projectile
	BASE_OBJECT **  psDamaged;              ///< the targets that have already been dealt damage to (don't damage the same target twice)
	unsigned        psNumDamaged;

	UDWORD          startX, startY;         ///< Where projectile started
	UDWORD          tarX, tarY;             ///< The target coordinates
	SDWORD          vXY, vZ;                ///< axis velocities
	UDWORD          srcHeight;              ///< Height of origin
	SDWORD          altChange;              ///< Change in altitude
	UDWORD          born;
	UDWORD          died;
} PROJECTILE;

#endif // __INCLUDED_PROJECTILEDEF_H__
