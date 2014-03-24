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
 *  Link droids together into a group for AI etc.
 */

#ifndef __INCLUDED_SRC_GROUP_H__
#define __INCLUDED_SRC_GROUP_H__

#include "order.h"

typedef enum _group_type
{
    GT_NORMAL,			// standard group
    GT_COMMAND,			// command droid group
    GT_TRANSPORTER,		// transporter group
} GROUP_TYPE;

typedef struct _droid_group
{
    int16_t		type;
    int16_t		refCount;
    DROID		*psList;			// list of droids in the group
    DROID		*psCommander;		// the command droid of a command group
    RUN_DATA	sRunData;			// where the group should retreat to
    struct _droid_group *psNext, *psPrev;	// keep linked to destroy all (a workaround hack)
} DROID_GROUP;

// initialise the group system
BOOL grpInitialise(void);

// shutdown the group system
void grpShutDown(void);

// create a new group
BOOL grpCreate(DROID_GROUP	**ppsGroup);

// add a droid to a group
void grpJoin(DROID_GROUP *psGroup, DROID *psDroid);

// add a droid to a group at the end of the list
void grpJoinEnd(DROID_GROUP *psGroup, DROID *psDroid);

// remove a droid from a group
void grpLeave(DROID_GROUP *psGroup, DROID *psDroid);

// count the members of a group
unsigned int grpNumMembers(const DROID_GROUP *psGroup);

// remove all droids from a group
void grpReset(DROID_GROUP *psGroup);

/* Give a group an order */
struct _droid_order_data;
//extern void orderGroupBase(DROID_GROUP *psGroup, struct _droid_order_data *psData);

/* Give a group an order */
void orderGroup(DROID_GROUP *psGroup, DROID_ORDER order);

/* Give a group of droids an order */
void orderGroupLoc(DROID_GROUP *psGroup, DROID_ORDER order, uint32_t x, uint32_t y);

/* Give a group of droids an order */
void orderGroupObj(DROID_GROUP *psGroup, DROID_ORDER order, BASE_OBJECT *psObj);

/* set the secondary state for a group of droids */
void grpSetSecondary(DROID_GROUP *psGroup, SECONDARY_ORDER sec, SECONDARY_STATE state);

#endif // __INCLUDED_SRC_GROUP_H__
