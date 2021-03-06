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

#ifndef __INCLUDED_SRC_ASTART_H__
#define __INCLUDED_SRC_ASTART_H__

#include "fpath.h"

/** Reset the A* counters
 *
 *  This function resets astarInner among others.
 *
 *  @ingroup pathfinding
 */
extern void astarResetCounters(void);

/** return codes for astar
 *
 *  @ingroup pathfinding
 */
typedef enum
{
	ASR_OK,         ///< found a route
	ASR_FAILED,     ///< no route could be found
	ASR_NEAREST,    ///< found a partial route to a nearby position
} ASR_RETVAL;

/** Use the A* algorithm to find a path
 *
 *  @ingroup pathfinding
 */
ASR_RETVAL fpathAStarRoute(MOVE_CONTROL *psMove, PATHJOB *psJob);

/** Check LOS (Line Of Sight) between two tiles
 */
extern BOOL fpathTileLOS(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/** Clean up the path finding node table.
 *
 *  @note Call this <em>only</em> on shutdown to prevent memory from leaking.
 *
 *  @ingroup pathfinding
 */
extern void fpathHardTableReset(void);

#endif // __INCLUDED_SRC_ASTART_H__
