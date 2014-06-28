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
 * @file power.c
 *
 * Store PlayerPower and other power related stuff!
 *
 */

#include "lib/framework/frame.h"

#include "difficulty.h"
#include "intdisplay.h"
#include "mission.h"
#include "power.h"
#include "research.h"

#define EXTRACT_POINTS	    1
#define EASY_POWER_MOD      110
#define NORMAL_POWER_MOD    100
#define HARD_POWER_MOD      90
#define MAX_POWER           100000

//flag used to check for power calculations to be done or not
BOOL	powerCalculated;

PLAYER_POWER		asPower[MAX_PLAYERS];

/*allocate the space for the playerPower*/
BOOL allocPlayerPower(void)
{
	clearPlayerPower();
	powerCalculated = true;
	return true;
}

/*clear the playerPower */
void clearPlayerPower(void)
{
	uint32_t player;

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		memset(&asPower[player], 0, sizeof(PLAYER_POWER));
	}
}

/*check the current power - if enough return true, else return false */
BOOL checkPower(uint32_t player, uint32_t quantity)
{
	ASSERT(player < MAX_PLAYERS, "checkPower: Bad player");

	//if not doing a check on the power - just return true
	if (!powerCalculated)
	{
		return true;
	}

	if (asPower[player].currentPower >= quantity)
	{
		return true;
	}
	return false;
}

/*check the current power - if enough subtracts the amount
 required to perform the task and returns true, else returns
 false */
BOOL usePower(uint32_t player, uint32_t quantity)
{
	ASSERT(player < MAX_PLAYERS, "usePower: Bad player");

	//if not doing a check on the power - just return true
	if (!powerCalculated)
	{
		return true;
	}

	//check there is enough first
	if (asPower[player].currentPower >= quantity)
	{
		asPower[player].currentPower -= quantity;
		return true;
	}
	return false;
}

//return the power when a structure/droid is deliberately destroyed
void addPower(uint32_t player, uint32_t quantity)
{
	ASSERT(player < MAX_PLAYERS, "addPower: Bad player (%u)", player);

	ASSERT(quantity <= MAX_POWER / 2, "Power over or under flow. Tried to add %u.", quantity);
	if (quantity > MAX_POWER)
	{
		debug(LOG_WARNING, "Power over or under flow. Tried to add %u - ignored", quantity);
		return;
	}
	asPower[player].currentPower += quantity;
	if (asPower[player].currentPower > MAX_POWER)
	{
		asPower[player].currentPower = MAX_POWER;
	}
}

/*resets the power calc flag for all players*/
void powerCalc(BOOL on)
{
	powerCalculated = on;
}

/* Each Resource Extractor yields EXTRACT_POINTS per second until there are none
   left in the resource. */
static int32_t updateExtractedPower(STRUCTURE *psBuilding)
{
	RES_EXTRACTOR		*pResExtractor;
	int32_t				pointsToAdd, extractedPoints;
	int32_t				powmodifier;

	pResExtractor = (RES_EXTRACTOR *) psBuilding->pFunctionality;
	extractedPoints = 0;

	//only extracts points whilst its active ie associated with a power gen
	if (pResExtractor->active)
	{
		int overflowDiff;
		// Add modifier according to difficulty level
		if (getDifficultyLevel() == DL_EASY && !bMultiPlayer)
		{
			powmodifier = EASY_POWER_MOD / 10;
		}
		else if (getDifficultyLevel() == DL_HARD && !bMultiPlayer)
		{
			powmodifier = HARD_POWER_MOD / 10;
		}
		else
		{
			powmodifier = NORMAL_POWER_MOD / 10;
		}
		// if the extractor hasn't been updated recently, now would be a good time.
		if (pResExtractor->timeLastUpdated < 20 && gameTime >= 20)
		{
			pResExtractor->timeLastUpdated = gameTime;
		}
		// include modifier as a %
		// Written this way to prevent rounding errors - do not "simplify"
		overflowDiff = pResExtractor->timeLastUpdated - (pResExtractor->timeLastUpdated % (GAME_TICKS_PER_SEC * 10));
		pointsToAdd = ((gameTime - overflowDiff)                       * powmodifier * EXTRACT_POINTS) / (GAME_TICKS_PER_SEC * 10)
					  - ((pResExtractor->timeLastUpdated - overflowDiff) * powmodifier * EXTRACT_POINTS) / (GAME_TICKS_PER_SEC * 10);
		/*
		 * If you are thinking about rewriting the preceding section of code, there are a few things
		 * you should know.
		 *
		 * First of all, division doesn't work intuitively.
		 * 3 / 2 = 1, not 1.5. These are integers, not floats.
		 *
		 * Second of all, timeLastUpdated is NOT necessarily the time you want to base off of.
		 * timeLastUpdated of an extractor that's just been created is some magic number around 12ms.
		 * gameTime, on the other hand, could be around 3 hours.
		 */
		ASSERT(pointsToAdd > -1 && pointsToAdd < 10, "pointsToAdd not in sensical range");

		if ((int)pResExtractor->timeLastUpdated - (int)gameTime > GAME_TICKS_PER_SEC || (int)gameTime - (int)pResExtractor->timeLastUpdated > GAME_TICKS_PER_SEC)
		{
			// extractor is not in the right time zone
			// we have a maximum time skip of less than a second, so this can't be caused by lag
			ASSERT(false, "Oil derrick out of sync.");
			pointsToAdd = 0;
		}
		pResExtractor->timeLastUpdated = gameTime;
		if (pointsToAdd)
		{
			extractedPoints += pointsToAdd;

			// If not having unlimited power, uncomment the following lines.

			/*
			if (pResExtractor->power > pointsToAdd)
			{
				extractedPoints += pointsToAdd;
				pResExtractor->power -= pointsToAdd;
			}
			else
			{
				extractedPoints += pResExtractor->power;
				pResExtractor->power = 0;
			}

			if (pResExtractor->power == 0)
			{
				// set the extractor to be inactive
				pResExtractor->active = false;
				// break the link between the power gen and the res extractor
				releaseResExtractor(psBuilding);
			}
			*/
		}
	}
	return extractedPoints;
}

//returns the relevant list based on OffWorld or OnWorld
static STRUCTURE *powerStructList(uint8_t player)
{
	ASSERT(player < MAX_PLAYERS, "powerStructList: Bad player");
	if (offWorldKeepLists)
	{
		return (mission.apsStructLists[player]);
	}
	else
	{
		return (apsStructLists[player]);
	}
}

/* Updates the current power based on the extracted power and a Power Generator*/
static void updateCurrentPower(POWER_GEN *psPowerGen, uint32_t player)
{
	int32_t		power, i, extractedPower;

	ASSERT(player < MAX_PLAYERS, "updateCurrentPower: Bad player");

	//each power gen can cope with its associated resource extractors
	extractedPower = 0;
	for (i = 0; i < NUM_POWER_MODULES; i++)
	{
		if (psPowerGen->apResExtractors[i])
		{
			//check not died
			if (psPowerGen->apResExtractors[i]->died)
			{
				psPowerGen->apResExtractors[i] = NULL;
			}
			else
			{
				extractedPower += updateExtractedPower(psPowerGen->apResExtractors[i]);
			}
		}
	}

	// Written this way to prevent rounding errors - do not "simplify"
	power = ((asPower[player].extractedPower + extractedPower) * psPowerGen->multiplier) / 100
			- (asPower[player].extractedPower * psPowerGen->multiplier) / 100;
	asPower[player].extractedPower += extractedPower;
	if (asPower[player].extractedPower > 100)
	{
		asPower[player].extractedPower -= 100; // just to prevent integer overflow
	}

	ASSERT_OR_RETURN(, power < 20 && power > -1, "Power out of sane range");
	if (power >= 0)
	{
		asPower[player].currentPower += power;
		if (asPower[player].currentPower > MAX_POWER)
		{
			asPower[player].currentPower = MAX_POWER;
		}
	}
}

/* Update current power based on what Power Generators exist */
void updatePlayerPower(uint32_t player)
{
	STRUCTURE		*psStruct;//, *psList;

	ASSERT(player < MAX_PLAYERS, "updatePlayerPower: Bad player");

	for (psStruct = powerStructList((uint8_t)player); psStruct != NULL; psStruct =
				psStruct->psNext)
	{
		if (psStruct->pStructureType->type == REF_POWER_GEN && psStruct->
				status == SS_BUILT)
		{
			updateCurrentPower((POWER_GEN *)psStruct->pFunctionality, player);
		}
	}
}

// only used in multiplayer games.
void setPower(uint32_t player, uint32_t avail)
{
	ASSERT(player < MAX_PLAYERS, "setPower: Bad player (%u)", player);

	asPower[player].currentPower = avail;
}

uint32_t getPower(uint32_t player)
{
	ASSERT(player < MAX_PLAYERS, "setPower: Bad player (%u)", player);

	return asPower[player].currentPower;
}

/*sets the initial value for the power*/
void setPlayerPower(uint32_t power, uint32_t player)
{
	ASSERT(player < MAX_PLAYERS, "setPlayerPower: Bad player (%u)", player);

	asPower[player].currentPower = power;
}

/*Temp function to give all players some power when a new game has been loaded*/
void newGameInitPower(void)
{
	uint32_t		inc;

	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		addPower(inc, 400);
	}
}

static int useAvailablePower(int player, int powerWanted)
{
	static int powerPerConsumer[MAX_PLAYERS] = {0};         // These values should be reset if starting a new game, but who cares about theoretically possible desynchs like that in 2.3...
	static int powerConsumersThisTick[MAX_PLAYERS] = {0};
	static int powerGameTimeOfThisTick[MAX_PLAYERS] = {0};

	if (powerWanted <= 0)
	{
		return 0;  // Nothing to do.
	}

	if (powerGameTimeOfThisTick[player] != gameTime)
	{
		// We ticked.
		// Each object may consume POWER_PER_SECOND power per second, but not more power than is available.
		powerPerConsumer[player] = gameTime / (GAME_TICKS_PER_SEC / POWER_PER_SECOND) - (gameTime - frameTime) / (GAME_TICKS_PER_SEC / POWER_PER_SECOND);
		if (powerCalculated)
		{
			powerPerConsumer[player] = MIN(powerPerConsumer[player], asPower[player].currentPower / MAX(powerConsumersThisTick[player], 1));
		}
		powerConsumersThisTick[player] = 0;
		powerGameTimeOfThisTick[player] = gameTime;
	}

	++powerConsumersThisTick[player];

	if (checkPower(player, powerPerConsumer[player]))  // Must still check if the power is actually available, since available power per consumer is based on data from the previous tick.
	{
		int availablePower = MIN(powerPerConsumer[player], powerWanted);
		usePower(player, availablePower);
		return availablePower;
	}
	return 0;
}


/*accrue the power in the facilities that require it - returns true if use some power*/
void accruePower(BASE_OBJECT *psObject)
{
	FACTORY					*psFactory;
	RESEARCH_FACILITY		*psResearch;
	REPAIR_FACILITY			*psRepair;
	int32_t					powerDiff;
	uint32_t					count;
	STRUCTURE			*psStructure;
	DROID				*psDroid, *psTarget;

	switch(psObject->type)
	{
		case OBJ_STRUCTURE:
			psStructure = (STRUCTURE *)psObject;
			// See if it needs power
			switch(psStructure->pStructureType->type)
			{
				case REF_FACTORY:
				case REF_CYBORG_FACTORY:
				case REF_VTOL_FACTORY:
					psFactory = (FACTORY *)psStructure->pFunctionality;
					// Check the factory is not on hold
					if (psFactory->timeStartHold)
					{
						break;
					}
					// Check the factory is active
					if (psFactory->psSubject)
					{
						//check needs power
						powerDiff = ((DROID_TEMPLATE *)psFactory->psSubject)->powerPoints -
									psFactory->powerAccrued;
						psFactory->powerAccrued += useAvailablePower(psStructure->player, powerDiff);
					}
					break;
				case REF_RESEARCH:
					//check the structure is active
					psResearch = (RESEARCH_FACILITY *)psStructure->pFunctionality;

					//check the research facility is not on hold
					if (psResearch->timeStartHold)
					{
						break;
					}
					if (psResearch->psSubject)
					{
						//check the research hasn't been cancelled
						count = ((RESEARCH *)psResearch->psSubject)->ref - REF_RESEARCH_START;
						if (IsResearchCancelled(asPlayerResList[selectedPlayer] + count) == false)
						{
							//check needs power
							powerDiff = ((RESEARCH *)psResearch->psSubject)->researchPower -
										psResearch->powerAccrued;
							//use the power if appropriate
							psResearch->powerAccrued += useAvailablePower(psStructure->player, powerDiff);
						}
					}
					break;
				case REF_REPAIR_FACILITY:
					//POWER REQUIRMENTS REMOVED - AB  22/09/98 - BACK IN - AB 07/01/99
					psRepair = (REPAIR_FACILITY *)psStructure->pFunctionality;
					psDroid = (DROID *)psRepair->psObj;
					//check the droid hasn't died in the meantime
					if (psRepair->psObj && psRepair->psObj->died)
					{
						psRepair->psObj = NULL;
					}
					if (psRepair->psObj)
					{
						//check if need power
						powerDiff = powerReqForDroidRepair(psDroid) - psDroid->powerAccrued;
						psDroid->powerAccrued += useAvailablePower(psStructure->player, powerDiff / POWER_FACTOR) * POWER_FACTOR;
					}
					break;
				default:
					//no need for power
					break;
			}
			break;
		case OBJ_DROID:
			psDroid = (DROID *)psObject;
			switch (psDroid->droidType)
			{
				case DROID_CONSTRUCT:
				case DROID_CYBORG_CONSTRUCT:
					//check trying to build something (and that hasn't been blown up)
					if (DroidIsBuilding(psDroid) && psDroid->psTarget && !psDroid->psTarget->died)
					{
						powerDiff = structPowerToBuild((STRUCTURE *)psDroid->psTarget) -
									((STRUCTURE *)psDroid->psTarget)->currentPowerAccrued;
						((STRUCTURE *)psDroid->psTarget)->currentPowerAccrued += useAvailablePower(psDroid->player, powerDiff);
					}
					break;
				case DROID_REPAIR:
				case DROID_CYBORG_REPAIR:
					//check trying to repair something
					psTarget = NULL;
					if (DroidIsRepairing(psDroid))
					{
						psTarget = (DROID *)psDroid->psTarget;
					}
					else
					{
						//might have guard order but action of repair
						if (orderState(psDroid, DORDER_GUARD) && psDroid->action ==
								DACTION_DROIDREPAIR)
						{
							psTarget = (DROID *)psDroid->psActionTarget[0];
						}
					}
					//check the droid hasn't died in the meantime
					if (psTarget && psTarget->died)
					{
						setDroidTarget(psDroid, NULL);
						psTarget = NULL;
					}
					if (psTarget)
					{
						powerDiff = powerReqForDroidRepair(psTarget) - psTarget->powerAccrued;
						psTarget->powerAccrued += useAvailablePower(psDroid->player, powerDiff / POWER_FACTOR) * POWER_FACTOR;
					}
					break;
				default:
					//no need for power
					break;
			}
			break;
		default:
			ASSERT( false, "accruePower: Invalid object type" );
	}
}


STRUCTURE *getRExtractor(STRUCTURE *psStruct)
{
	STRUCTURE	*psCurr;
	STRUCTURE	*psFirst;
	BOOL		bGonePastIt;

	for(psCurr = apsStructLists[selectedPlayer], psFirst = NULL, bGonePastIt = false;
			psCurr; psCurr = psCurr->psNext)
	{
		if( psCurr->pStructureType->type == REF_RESOURCE_EXTRACTOR )
		{

			if(!psFirst)
			{
				psFirst = psCurr;
			}

			if(!psStruct)
			{
				return(psCurr);
			}
			else if(psCurr != psStruct && bGonePastIt)
			{
				return(psCurr);
			}

			if(psCurr == psStruct)
			{
				bGonePastIt = true;
			}


		}
	}
	return(psFirst);
}

/*defines which structure types draw power - returns true if use power*/
BOOL structUsesPower(STRUCTURE *psStruct)
{
	BOOL    bUsesPower = false;

	ASSERT( psStruct != NULL,
			"structUsesPower: Invalid Structure pointer" );

	switch(psStruct->pStructureType->type)
	{
		case REF_FACTORY:
		case REF_CYBORG_FACTORY:
		case REF_VTOL_FACTORY:
		case REF_RESEARCH:
		case REF_REPAIR_FACILITY:
			bUsesPower = true;
			break;
		default:
			bUsesPower = false;
			break;
	}

	return bUsesPower;
}

/*defines which droid types draw power - returns true if use power*/
BOOL droidUsesPower(DROID *psDroid)
{
	BOOL    bUsesPower = false;

	ASSERT(psDroid != NULL,	"droidUsesPower: Invalid unit pointer" );

	switch(psDroid->droidType)
	{
		case DROID_CONSTRUCT:
		case DROID_REPAIR:
		case DROID_CYBORG_CONSTRUCT:
		case DROID_CYBORG_REPAIR:
			bUsesPower = true;
			break;
		default:
			bUsesPower = false;
			break;
	}

	return bUsesPower;
}
