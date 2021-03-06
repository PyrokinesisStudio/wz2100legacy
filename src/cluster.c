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
/*
 * Cluster.c
 *
 * Form droids and structures into clusters
 *
 */

#include "lib/framework/frame.h"
#include "lib/script/script.h"

#include "cluster.h"
#include "map.h"
#include "scriptcb.h"
#include "scripttabs.h"

// distance between units for them to be in the same cluster
#define CLUSTER_DIST	(TILE_UNITS*8)


// Indirect the cluster ID to an actual cluster number
uint8_t	aClusterMap[CLUSTER_MAX];

// flag to note when a cluster needs the cluster empty callback
static uint8_t aClusterEmpty[CLUSTER_MAX];

// number of droids in a cluster
static uint16_t aClusterUsage[CLUSTER_MAX];

// whether a cluster can be seen by a player
static uint8_t aClusterVisibility[CLUSTER_MAX];

// when a cluster was last attacked
static uint32_t aClusterAttacked[CLUSTER_MAX];

// information about the cluster
uint8_t	aClusterInfo[CLUSTER_MAX];

// initialise the cluster system
void clustInitialise(void)
{
	DROID		*psDroid;
	STRUCTURE	*psStruct;
	int32_t		player;

	ASSERT( CLUSTER_MAX <= uint8_t_MAX,
			"clustInitialse: invalid CLUSTER_MAX, this is a BUILD error" );

	memset(aClusterMap, 0, sizeof(uint8_t) * CLUSTER_MAX);
	memset(aClusterEmpty, 0, sizeof(uint8_t) * CLUSTER_MAX);
	memset(aClusterUsage, 0, sizeof(uint16_t) * CLUSTER_MAX);
	memset(aClusterVisibility, 0, sizeof(uint8_t) * CLUSTER_MAX);
	memset(aClusterAttacked, 0, sizeof(uint32_t) * CLUSTER_MAX);
	memset(aClusterInfo, 0, sizeof(uint8_t) * CLUSTER_MAX);

	for(player = 0; player < MAX_PLAYERS; player++)
	{
		for(psDroid = apsDroidLists[player]; psDroid; psDroid = psDroid->psNext)
		{
			psDroid->cluster = 0;
		}

		for(psStruct = apsStructLists[player]; psStruct; psStruct = psStruct->psNext)
		{
			psStruct->cluster = 0;
		}

		for(psStruct = apsStructLists[player]; psStruct; psStruct = psStruct->psNext)
		{
			if (psStruct->cluster == 0)
			{
				clustUpdateObject((BASE_OBJECT *)psStruct);
			}
		}
	}
}

// update routine for the cluster system
void clusterUpdate(void)
{
	int32_t	i;

	for(i = 1; i < CLUSTER_MAX; i++)
	{
		if (aClusterEmpty[i])
		{
			scrCBEmptyClusterID = i;
			eventFireCallbackTrigger((TRIGGER_TYPE)CALL_CLUSTER_EMPTY);
			aClusterEmpty[i] = false;
		}
	}
}


// update all objects from a list belonging to a specific cluster
void clustUpdateCluster(BASE_OBJECT *psList, int32_t cluster)
{
	BASE_OBJECT		*psCurr;

	if (cluster == 0)
	{
		return;
	}

	for(psCurr = psList; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->cluster == cluster)
		{
			clustUpdateObject(psCurr);
		}
	}
}

// remove an object from the cluster system
void clustRemoveObject(BASE_OBJECT *psObj)
{
	int32_t i;

	ASSERT( psObj->cluster < CLUSTER_MAX,
			"clustRemoveObject: invalid cluster number" );

	// update the usage counter
	if (psObj->cluster != 0)
	{
		ASSERT( aClusterUsage[psObj->cluster] > 0,
				"clustRemoveObject: usage array out of sync" );
		aClusterUsage[psObj->cluster] -= 1;

		if (aClusterUsage[psObj->cluster] == 0)
		{
			// cluster is empty - make sure the cluster map gets emptied
			for (i = 0; i < CLUSTER_MAX; i++)
			{
				if (aClusterMap[i] == psObj->cluster)
				{
					aClusterMap[i] = 0;
					if (i != 0)
					{
// 						debug( LOG_NEVER, "Cluster %d empty: ", i );
// 						debug( LOG_NEVER, "%s ", (psObj->type == OBJ_DROID) ? "Unit" : ( (psObj->type == OBJ_STRUCTURE) ? "Struct" : "Feat") );
// 						debug( LOG_NEVER, "id %d player %d\n", psObj->id, psObj->player );
						aClusterEmpty[i] = true;
					}
				}
			}

			// reset the cluster visibility and attacked
			aClusterVisibility[psObj->cluster] = 0;
			aClusterAttacked[psObj->cluster] = 0;
			aClusterInfo[psObj->cluster] = 0;
		}
	}

	psObj->cluster = 0;
}


// tell a droid to join a cluster
static void clustAddDroid(DROID *psDroid, int32_t cluster)
{
	DROID	*psCurr;
	int32_t	xdiff, ydiff;
	int32_t	player;

	clustRemoveObject((BASE_OBJECT *)psDroid);

	aClusterUsage[cluster] += 1;
	psDroid->cluster = (uint8_t)cluster;
	for(player = 0; player < MAX_PLAYERS; player++)
	{
		if (psDroid->visible[player])
		{
			aClusterVisibility[cluster] |= (1 << player);
		}
	}

	for(psCurr = apsDroidLists[psDroid->player]; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->cluster == (uint8_t)cluster)
		{
			continue;
		}

		xdiff = (int32_t)psDroid->pos.x - (int32_t)psCurr->pos.x;
		ydiff = (int32_t)psDroid->pos.y - (int32_t)psCurr->pos.y;
		if (xdiff * xdiff + ydiff * ydiff < CLUSTER_DIST * CLUSTER_DIST)
		{
			clustAddDroid(psCurr, cluster);
		}
	}
}


// tell the cluster system about a new droid
void clustNewDroid(DROID *psDroid)
{
	DROID	*psCurr;
	int32_t	xdiff, ydiff;

	psDroid->cluster = 0;
	for(psCurr = apsDroidLists[psDroid->player]; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->cluster != 0)
		{
			xdiff = (int32_t)psDroid->pos.x - (int32_t)psCurr->pos.x;
			ydiff = (int32_t)psDroid->pos.y - (int32_t)psCurr->pos.y;
			if (xdiff * xdiff + ydiff * ydiff < CLUSTER_DIST * CLUSTER_DIST)
			{
				clustAddDroid(psDroid, psCurr->cluster);
				return;
			}
		}
	}
}


// tell a structure to join a cluster
static void clustAddStruct(STRUCTURE *psStruct, int32_t cluster)
{
	STRUCTURE	*psCurr;
	int32_t		xdiff, ydiff;
	int32_t		player;

	clustRemoveObject((BASE_OBJECT *)psStruct);

	aClusterUsage[cluster] += 1;
	psStruct->cluster = (uint8_t)cluster;
	for(player = 0; player < MAX_PLAYERS; player++)
	{
		if (psStruct->visible[player])
		{
			aClusterVisibility[cluster] |= (1 << player);
		}
	}

	for(psCurr = apsStructLists[psStruct->player]; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->cluster == (uint8_t)cluster)
		{
			continue;
		}

		xdiff = (int32_t)psStruct->pos.x - (int32_t)psCurr->pos.x;
		ydiff = (int32_t)psStruct->pos.y - (int32_t)psCurr->pos.y;
		if (xdiff * xdiff + ydiff * ydiff < CLUSTER_DIST * CLUSTER_DIST)
		{
			clustAddStruct(psCurr, cluster);
		}
	}
}


// tell the cluster system about a new structure
void clustNewStruct(STRUCTURE *psStruct)
{
	STRUCTURE	*psCurr;
	int32_t		xdiff, ydiff;

	psStruct->cluster = 0;
	for(psCurr = apsStructLists[psStruct->player]; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->cluster != 0)
		{
			xdiff = (int32_t)psStruct->pos.x - (int32_t)psCurr->pos.x;
			ydiff = (int32_t)psStruct->pos.y - (int32_t)psCurr->pos.y;
			if (xdiff * xdiff + ydiff * ydiff < CLUSTER_DIST * CLUSTER_DIST)
			{
				psStruct->cluster = psCurr->cluster;
				aClusterUsage[psCurr->cluster] += 1;
				break;
			}
		}
	}

	clustUpdateObject((BASE_OBJECT *)psStruct);
}


// find an unused cluster number for a droid
static int32_t clustFindUnused(void)
{
	int32_t	cluster;

	for(cluster = 1; cluster < CLUSTER_MAX; cluster ++)
	{
		if (aClusterUsage[cluster] == 0)
		{
			return cluster;
		}
	}

	// no unused cluster return the default
	return 0;
}

// update the cluster information for an object
void clustUpdateObject(BASE_OBJECT *psObj)
{
	int32_t	newCluster, oldCluster, i;
	BOOL	found;
	int32_t	player;

	newCluster = clustFindUnused();
	oldCluster = psObj->cluster;

	// update the cluster map
	found = false;
	if (oldCluster != 0)
	{
		for(i = 0; i < CLUSTER_MAX; i++)
		{
			ASSERT( (aClusterMap[i] == 0) ||
					(aClusterUsage[ aClusterMap[i] ] != 0),
					"clustUpdateObject: cluster map out of sync" );

			if (aClusterMap[i] == oldCluster)
			{
				// found the old cluster - change it to the new one
				aClusterMap[i] = (uint8_t)newCluster;
				aClusterAttacked[newCluster] = aClusterAttacked[oldCluster];
//				aClusterVisibility[newCluster] = aClusterVisibility[oldCluster];
				aClusterVisibility[newCluster] = 0;
				for(player = 0; player < MAX_PLAYERS; player++)
				{
					if (psObj->visible[player])
					{
						aClusterVisibility[newCluster] |= 1 << player;
					}
				}
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		// there is no current cluster map - create a new one
		for(i = 1; i < CLUSTER_MAX; i++)
		{
			if (aClusterMap[i] == 0)
			{
				// found a free cluster
				aClusterMap[i] = (uint8_t)newCluster;
				break;
			}
		}
	}

	// store the information about this cluster
	aClusterInfo[newCluster] = (uint8_t)(psObj->player & CLUSTER_PLAYER_MASK);
	switch (psObj->type)
	{
		case OBJ_DROID:
			aClusterInfo[newCluster] |= CLUSTER_DROID;
			clustAddDroid((DROID *)psObj, newCluster);
			break;
		case OBJ_STRUCTURE:
			aClusterInfo[newCluster] |= CLUSTER_STRUCTURE;
			clustAddStruct((STRUCTURE *)psObj, newCluster);
			break;
		default:
			ASSERT(!"invalid object type", "clustUpdateObject: invalid object type");
			break;
	}
}


// get the cluster ID for a droid
int32_t clustGetClusterID(BASE_OBJECT *psObj)
{
	int32_t	cluster;

	if (psObj->cluster == 0)
	{
		return 0;
	}

	for (cluster = 0; cluster < CLUSTER_MAX; cluster ++)
	{
		if (aClusterMap[cluster] == psObj->cluster)
		{
			return cluster;
		}
	}

	return 0;
}


// variables for the cluster iteration
static int32_t		iterateClusterID;
static BASE_OBJECT	*psIterateList, *psIterateObj;

// initialise iterating a cluster
void clustInitIterate(int32_t clusterID)
{
	int32_t		player, cluster;

	iterateClusterID = clusterID;
	cluster = aClusterMap[clusterID];
	player = aClusterInfo[cluster] & CLUSTER_PLAYER_MASK;

	if (aClusterInfo[cluster] & CLUSTER_DROID)
	{
		psIterateList = (BASE_OBJECT *)apsDroidLists[player];
	}
	else // if (aClusterInfo[cluster] & CLUSTER_STRUCTURE)
	{
		psIterateList = (BASE_OBJECT *)apsStructLists[player];
	}

	psIterateObj = NULL;
}

// iterate a cluster
BASE_OBJECT *clustIterate(void)
{
	BASE_OBJECT	*psStart;
	int32_t		cluster;

	cluster = aClusterMap[iterateClusterID];

	if (psIterateObj == NULL)
	{
		psStart = psIterateList;
	}
	else
	{
		psStart = psIterateObj->psNext;
	}

	for(psIterateObj = psStart;
			psIterateObj && psIterateObj->cluster != cluster;
			psIterateObj = psIterateObj->psNext)
		;

	if (psIterateObj == NULL)
	{
		psIterateList = NULL;
	}

	return psIterateObj;
}

// find the center of a cluster
// NOTE: Unused! void clustGetCenter(BASE_OBJECT *psObj, int32_t *px, int32_t *py)
void clustGetCenter(BASE_OBJECT *psObj, int32_t *px, int32_t *py)
{
	BASE_OBJECT		*psList;
	BASE_OBJECT		*psCurr;
	int32_t			averagex, averagey, num;

	switch (psObj->type)
	{
		case OBJ_DROID:
			psList = (BASE_OBJECT *)apsDroidLists[psObj->player];
			break;
		case OBJ_STRUCTURE:
			psList = (BASE_OBJECT *)apsStructLists[psObj->player];
			break;
		default:
			ASSERT(!"invalid object type", "clustGetCenter: invalid object type");
			psList = NULL;
			break;
	}

	averagex = 0;
	averagey = 0;
	num = 0;
	for (psCurr = psList; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->cluster == psObj->cluster)
		{
			averagex += (int32_t)psCurr->pos.x;
			averagey += (int32_t)psCurr->pos.y;
			num += 1;
		}
	}

	if (num > 0)
	{
		*px = averagex / num;
		*py = averagey / num;
	}
	else
	{
		*px = (int32_t)psObj->pos.x;
		*py = (int32_t)psObj->pos.y;
	}
}


// tell the cluster system that an objects visibility has changed
void clustObjectSeen(BASE_OBJECT *psObj, BASE_OBJECT *psViewer)
{
	int32_t	player;

	for(player = 0; player < MAX_PLAYERS; player++)
	{
		ASSERT(psObj->cluster != (uint8_t)~0, "object not in a cluster");
		if ( (player != (int32_t)psObj->player) &&
				psObj->visible[player] &&
				!(aClusterVisibility[psObj->cluster] & (1 << player)))
		{
//			DBPRINTF(("cluster %d (player %d) seen by player %d\n",
//				clustGetClusterID(psObj), psObj->player, player));
			aClusterVisibility[psObj->cluster] |= 1 << player;

			psScrCBObjSeen = psObj;
			psScrCBObjViewer = psViewer;
			eventFireCallbackTrigger((TRIGGER_TYPE)CALL_OBJ_SEEN);

			switch (psObj->type)
			{
				case OBJ_DROID:
					eventFireCallbackTrigger((TRIGGER_TYPE)CALL_DROID_SEEN);
					break;
				case OBJ_STRUCTURE:
					eventFireCallbackTrigger((TRIGGER_TYPE)CALL_STRUCT_SEEN);
					break;
				case OBJ_FEATURE:
					eventFireCallbackTrigger((TRIGGER_TYPE)CALL_FEATURE_SEEN);
					break;
				default:
					ASSERT(!"invalid object type", "clustObjectSeen: invalid object type");
					break;
			}

			psScrCBObjSeen = NULL;
			psScrCBObjViewer = NULL;
		}
	}
}


// tell the cluster system that an object has been attacked
void clustObjectAttacked(BASE_OBJECT *psObj)
{
	if ((aClusterAttacked[psObj->cluster] + ATTACK_CB_PAUSE) < gameTime)
	{
		psScrCBTarget = psObj;
		eventFireCallbackTrigger((TRIGGER_TYPE)CALL_ATTACKED);

		switch (psObj->type)
		{
			case OBJ_DROID:
				psLastDroidHit = (DROID *)psObj;
				eventFireCallbackTrigger((TRIGGER_TYPE)CALL_DROID_ATTACKED);
				psLastDroidHit = NULL;
				break;
			case OBJ_STRUCTURE:
				psLastStructHit = (STRUCTURE *)psObj;
				eventFireCallbackTrigger((TRIGGER_TYPE)CALL_STRUCT_ATTACKED);
				psLastStructHit = NULL;
				break;
			default:
				ASSERT(!"invalid object type", "clustObjectAttacked: invalid object type");
				return;
		}

		aClusterAttacked[psObj->cluster] = gameTime;
	}
}


// reset the visibility for all clusters for a particular player
void clustResetVisibility(int32_t player)
{
	int32_t	i;

	for(i = 0; i < CLUSTER_MAX; i++)
	{
		aClusterVisibility[i] &= ~(1 << player);
	}
}
