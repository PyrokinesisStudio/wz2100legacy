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
 *  Definition for position objects.
 */

#ifndef __INCLUDED_POSITIONDEF_H__
#define __INCLUDED_POSITIONDEF_H__

#include "lib/framework/frame.h"

typedef enum POSITION_TYPE
{
	POS_DELIVERY,		//Delivery Points NOT wayPoints
	POS_PROXDATA,	//proximity messages that are data generated
	POS_PROXOBJ,	//proximity messages that are in game generated
	POS_TEMPDELIVERY //SAVE ONLY delivery point for factory currently assigned to commander
} POSITION_TYPE;

#define POSITION_OBJ \
	POSITION_TYPE	type;				/*the type of position obj - FlagPos or ProxDisp*/ \
	uint32_t			frameNumber;		/*when the Position was last drawn*/ \
	uint32_t			screenX;			/*screen coords and radius of Position imd */ \
	uint32_t			screenY; \
	uint32_t			screenR; \
	uint32_t			player;				/*which player the Position belongs to*/ \
	BOOL			selected			/*flag to indicate whether the Position is to be highlighted*/

typedef struct _object_position
{
	POSITION_OBJ;
} OBJECT_POSITION;

#endif // __INCLUDED_POSITIONDEF_H__
