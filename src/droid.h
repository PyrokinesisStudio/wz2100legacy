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
 *  Definitions for the droid object.
 */

#ifndef __INCLUDED_SRC_DROID_H__
#define __INCLUDED_SRC_DROID_H__

#include "lib/framework/string_ext.h"
#include "lib/gamelib/gtime.h"

#include "objectdef.h"
#include "stats.h"
#include "visibility.h"

#define OFF_SCREEN 9999		// world->screen check - alex

#define REPAIRLEV_LOW	50	// percentage of body points remaining at which to repair droid automatically.
#define REPAIRLEV_HIGH	75	// ditto, but this will repair much sooner..

#define	DROID_EXPLOSION_SPREAD_X	(TILE_UNITS/2 - (rand()%TILE_UNITS))
#define	DROID_EXPLOSION_SPREAD_Y	(rand()%TILE_UNITS)
#define	DROID_EXPLOSION_SPREAD_Z	(TILE_UNITS/2 - (rand()%TILE_UNITS))

/*defines the % to decrease the illumination of a tile when building - gets set
back when building is destroyed*/
//#define FOUNDATION_ILLUMIN		50

#define DROID_RESISTANCE_FACTOR     30

#define MAX_RECYCLED_DROIDS 100
// Changing this breaks savegames!
#define SAVE_RECYCLED_DROIDS 32

//storage
extern DROID_TEMPLATE			*apsDroidTemplates[MAX_PLAYERS];
extern DROID_TEMPLATE			*apsStaticTemplates;			// for AIs and scripts

extern bool runningMultiplayer(void);

/* The range for neighbouring objects */
#define NAYBOR_RANGE		(TILE_UNITS*9)	//range of lancer, BB, TK etc

//used to stop structures being built too near the edge and droids being placed down - pickATile
#define TOO_NEAR_EDGE	3

/* Define max number of allowed droids per droid type */
#define MAX_COMMAND_DROIDS		10		// max number of commanders a player can have
#define MAX_CONSTRUCTOR_DROIDS	15		// max number of constructors a player can have

/* Experience modifies */
#define EXP_REDUCE_DAMAGE		6		// damage of a droid is reduced by this value per experience level, in %
#define EXP_ACCURACY_BONUS		5		// accuracy of a droid is increased by this value per experience level, in %
#define EXP_SPEED_BONUS			5		// speed of a droid is increased by this value per experience level, in %

/* Misc accuracy modifiers */
#define	FOM_PARTIAL_ACCURACY_PENALTY	50	// penalty for not being fully able to fire while moving, in %
#define	INVISIBLE_ACCURACY_PENALTY		50	// accuracy penalty for the unit firing at a target it can't see, in %

/* Minumum number of droids a commander can control in its group */
#define	MIN_CMD_GROUP_DROIDS	10

/* Info stored for each droid neighbour */
typedef struct _naybor_info
{
	BASE_OBJECT		*psObj;			// The neighbouring object
	uint32_t			distSqr;		// The square of the distance to the object
	//uint32_t			dist;			// The distance to the object
} NAYBOR_INFO;

typedef enum
{
	NO_FREE_TILE,
	FREE_TILE,
	HALF_FREE_TILE
} PICKTILE;

/*Can we build here, now or ever?*/
typedef enum { PermissionDenied, PermissionGranted, PermissionPending } BuildPermissionState;

/* Store for the objects near the droid currently being updated */
#define MAX_NAYBORS		120
extern NAYBOR_INFO		asDroidNaybors[MAX_NAYBORS];
extern uint32_t			numNaybors;

// the structure that was last hit
extern DROID	*psLastDroidHit;

extern uint16_t	aDroidExperience[MAX_PLAYERS][MAX_RECYCLED_DROIDS];


// initialise droid module
extern BOOL droidInit(void);

extern void	removeDroidBase(DROID *psDel);

// refresh the naybor list
// this only does anything if the naybor list is out of date
extern void droidGetNaybors(DROID *psDroid);

extern BOOL loadDroidTemplates(const char *pDroidData, uint32_t bufferSize);
extern BOOL loadDroidWeapons(const char *pWeaponData, uint32_t bufferSize);

/*initialise the template build and power points */
extern void initTemplatePoints(void);

/*Builds an instance of a Structure - the x/y passed in are in world coords.*/
extern DROID *buildDroid(DROID_TEMPLATE *pTemplate, uint32_t x, uint32_t y,
						 uint32_t player, BOOL onMission);

/* Set the asBits in a DROID structure given it's template. */
extern void droidSetBits(DROID_TEMPLATE *pTemplate, DROID *psDroid);

/* Calculate the weight of a droid from it's template */
extern uint32_t calcDroidWeight(DROID_TEMPLATE *psTemplate);

/* Calculate the power points required to build/maintain a droid */
extern uint32_t calcDroidPower(DROID *psDroid);

// Calculate the number of points required to build a droid
extern uint32_t calcDroidPoints(DROID *psDroid);

/* Calculate the body points of a droid from it's template */
extern uint32_t calcTemplateBody(DROID_TEMPLATE *psTemplate, uint8_t player);

/* Calculate the base body points of a droid without upgrades*/
extern uint32_t calcDroidBaseBody(DROID *psDroid);

/* Calculate the base speed of a droid from it's template */
extern uint32_t calcDroidBaseSpeed(DROID_TEMPLATE *psTemplate, uint32_t weight, uint8_t player);

/* Calculate the speed of a droid over a terrain */
extern uint32_t calcDroidSpeed(uint32_t baseSpeed, uint32_t terrainType, uint32_t propIndex, uint32_t level);

/* Calculate the points required to build the template */
extern uint32_t calcTemplateBuild(DROID_TEMPLATE *psTemplate);

/* Calculate the power points required to build/maintain the droid */
extern uint32_t	calcTemplatePower(DROID_TEMPLATE *psTemplate);

// return whether a template is for an IDF droid
BOOL templateIsIDF(DROID_TEMPLATE *psTemplate);

// return whether a droid is IDF
BOOL idfDroid(DROID *psDroid);

/* Do damage to a droid */
extern float droidDamage(DROID *psDroid, uint32_t damage, uint32_t weaponClass, uint32_t weaponSubClass, HIT_SIDE impactSide);

/* The main update routine for all droids */
extern void droidUpdate(DROID *psDroid);

/* Set up a droid to build a structure - returns true if successful */
extern BuildPermissionState droidStartBuild(DROID *psDroid);

/* Sets a droid to start demolishing - returns true if successful */
extern BOOL	droidStartDemolishing( DROID *psDroid );

/* Update a construction droid while it is demolishing
   returns true while demolishing */
extern BOOL	droidUpdateDemolishing( DROID *psDroid );

/* Sets a droid to start repairing - returns true if successful */
extern BOOL	droidStartRepair( DROID *psDroid );

/* Update a construction droid while it is repairing
   returns true while repairing */
extern BOOL	droidUpdateRepair( DROID *psDroid );

/*Start a Repair Droid working on a damaged droid - returns true if successful*/
extern BOOL droidStartDroidRepair( DROID *psDroid );

/*Updates a Repair Droid working on a damaged droid - returns true whilst repairing*/
extern BOOL droidUpdateDroidRepair(DROID *psRepairDroid);

/*checks a droids current body points to see if need to self repair*/
extern void droidSelfRepair(DROID *psDroid);

/* Update a construction droid while it is building
   returns true while building continues */
extern BOOL droidUpdateBuild(DROID *psDroid);

/*Start a EW weapon droid working on a low resistance structure*/
extern BOOL droidStartRestore( DROID *psDroid );

/*continue restoring a structure*/
extern BOOL droidUpdateRestore( DROID *psDroid );

// recycle a droid (retain it's experience and some of it's cost)
extern void recycleDroid(DROID *psDel);

/* Release all resources associated with a droid */
extern void droidRelease(DROID *psDroid);

/* Remove a droid and free it's memory */
extern void destroyDroid(DROID *psDel);

/* Same as destroy droid except no graphical effects */
extern void	vanishDroid(DROID *psDel);

/* Burn a barbarian then destroy it */
extern void droidBurn( DROID *psDroid );

/* Remove a droid from the apsDroidLists so doesn't update or get drawn etc*/
//returns true if successfully removed from the list
extern BOOL droidRemove(DROID *psDroid, DROID *pList[MAX_PLAYERS]);

//free the storage for the droid templates
extern BOOL droidTemplateShutDown(void);

/* Return the type of a droid */
extern DROID_TYPE droidType(DROID *psDroid);

/* Return the type of a droid from it's template */
extern DROID_TYPE droidTemplateType(DROID_TEMPLATE *psTemplate);

//fills the list with Templates that can be manufactured in the Factory - based on size
extern uint32_t fillTemplateList(DROID_TEMPLATE **pList, STRUCTURE *psFactory, uint32_t limit);

extern void assignDroidsToGroup(uint32_t	playerNumber, uint32_t groupNumber);

extern BOOL activateGroup(uint32_t playerNumber, uint32_t groupNumber);

extern uint32_t	getNumDroidsForLevel(uint32_t	level);

extern BOOL activateGroupAndMove(uint32_t playerNumber, uint32_t groupNumber);
/* calculate muzzle tip location in 3d world added int weapon_slot to fix the always slot 0 hack*/
extern BOOL calcDroidMuzzleLocation(DROID *psDroid, Vector3f *muzzle, int weapon_slot);

/* gets a template from its aName (when pName is unknown) */
extern DROID_TEMPLATE	*GetHumanDroidTemplate(char *aName);
extern DROID_TEMPLATE	*GetAIDroidTemplate(char *aName);
/* gets a template from its name - relies on the name being unique */
extern DROID_TEMPLATE *getTemplateFromUniqueName(const char *pName, unsigned int player);
/* gets a template from its name - relies on the name being unique */
extern DROID_TEMPLATE *getTemplateFromTranslatedNameNoPlayer(char *pName);
/*getTemplateFromMultiPlayerID gets template for unique ID  searching all lists */
extern DROID_TEMPLATE *getTemplateFromMultiPlayerID(uint32_t multiPlayerID);

// finds a droid for the player and sets it to be the current selected droid
extern BOOL selectDroidByID(uint32_t id, uint32_t player);

/* Droid experience stuff */
extern unsigned int getDroidLevel(const DROID *psDroid);
extern uint32_t	getDroidEffectiveLevel(DROID *psDroid);
extern const char *getDroidLevelName(DROID *psDroid);

// Get a droid's name.
extern const char *droidGetName(const DROID *psDroid);

// Set a droid's name.
extern void droidSetName(DROID *psDroid, const char *pName);

// Set a templates name.
extern void templateSetName(DROID_TEMPLATE *psTemplate, char *pName);

// returns true when no droid on x,y square.
extern BOOL	noDroid					(uint32_t x, uint32_t y);				// true if no droid at x,y
// returns an x/y coord to place a droid
extern BOOL pickATile				(uint32_t *x0, uint32_t *y0, uint8_t numIterations);
extern PICKTILE pickHalfATile		(uint32_t *x, uint32_t *y, uint8_t numIterations);
extern BOOL	pickATile2				(uint32_t *x, uint32_t *y, uint32_t numIterations);
extern	BOOL	zonedPAT(uint32_t x, uint32_t y);
extern	BOOL	pickATileGen(uint32_t *x, uint32_t *y, uint8_t numIterations,
							 BOOL (*function)(uint32_t x, uint32_t y));
extern	BOOL	pickATileGenThreat(uint32_t *x, uint32_t *y, uint8_t numIterations, int32_t threatRange,
								   int32_t player, BOOL (*function)(uint32_t x, uint32_t y));


//initialises the droid movement model
extern void initDroidMovement(DROID *psDroid);

/* Looks through the players list of droids to see if any of them are
building the specified structure - returns true if finds one*/
extern BOOL checkDroidsBuilding(STRUCTURE *psStructure);

/* Looks through the players list of droids to see if any of them are
demolishing the specified structure - returns true if finds one*/
extern BOOL checkDroidsDemolishing(STRUCTURE *psStructure);

/* checks the structure for type and capacity and orders the droid to build
a module if it can - returns true if order is set */
extern BOOL buildModule(STRUCTURE *psStruct);

/*Deals with building a module - checking if any droid is currently doing this
 - if so, helping to build the current one*/
extern void setUpBuildModule(DROID *psDroid);

/*return the name to display for the interface given a DROID structure*/
extern const char *getDroidName(const DROID *psDroid) WZ_DECL_UNUSED;

/*return the name to display for the interface - we don't know if this is
a string ID or something the user types in*/
extern const char *getTemplateName(const DROID_TEMPLATE *psTemplate);

/* Just returns true if the droid's present body points aren't as high as the original*/
extern BOOL	droidIsDamaged(DROID *psDroid);

/* Returns currently active (selected) group */
extern uint32_t	getSelectedGroup( void );
extern void	setSelectedGroup(uint32_t groupNumber);
extern uint32_t	getSelectedCommander( void );
extern void	setSelectedCommander(uint32_t commander);


extern BOOL getDroidResourceName(char *pName);


/*checks to see if an electronic warfare weapon is attached to the droid*/
extern BOOL electronicDroid(DROID *psDroid);

/*checks to see if the droid is currently being repaired by another*/
extern BOOL droidUnderRepair(DROID *psDroid);

//count how many Command Droids exist in the world at any one moment
extern uint8_t checkCommandExist(uint8_t player);

/* Set up a droid to clear a wrecked building feature - returns true if successful */
extern BOOL droidStartClearing( DROID *psDroid );
/* Update a construction droid while it is clearing
   returns true while continues */
extern BOOL droidUpdateClearing( DROID *psDroid );

/*For a given repair droid, check if there are any damaged droids within
a defined range*/
extern BASE_OBJECT *checkForRepairRange(DROID *psDroid, DROID *psTarget);

/// Returns true iff the droid has VTOL propulsion, and is not a transport.
extern BOOL isVtolDroid(const DROID *psDroid);
/// Returns true iff the droid has VTOL propulsion and is moving.
extern BOOL isFlying(const DROID *psDroid);
/*returns true if a VTOL weapon droid which has completed all runs*/
extern BOOL vtolEmpty(DROID *psDroid);
/*returns true if a VTOL weapon droid which still has full ammo*/
extern BOOL vtolFull(DROID *psDroid);
/*Checks a vtol for being fully armed and fully repaired to see if ready to
leave reArm pad */
extern BOOL  vtolHappy(const DROID *psDroid);
/*this mends the VTOL when it has been returned to home base whilst on an
offworld mission*/
extern void mendVtol(DROID *psDroid);
/*checks if the droid is a VTOL droid and updates the attack runs as required*/
extern void updateVtolAttackRun(DROID *psDroid, int weapon_slot);
/*returns a count of the base number of attack runs for the weapon attached to the droid*/
extern uint16_t   getNumAttackRuns(DROID *psDroid, int weapon_slot);
//assign rearmPad to the VTOL
extern void assignVTOLPad(DROID *psNewDroid, STRUCTURE *psReArmPad);
// true if a vtol is waiting to be rearmed by a particular rearm pad
extern BOOL vtolReadyToRearm(DROID *psDroid, STRUCTURE *psStruct);
// true if a vtol droid currently returning to be rearmed
extern BOOL vtolRearming(DROID *psDroid);
// true if a droid is currently attacking
extern BOOL droidAttacking(DROID *psDroid);
// see if there are any other vtols attacking the same target
// but still rearming
extern BOOL allVtolsRearmed(DROID *psDroid);

/*compares the droid sensor type with the droid weapon type to see if the
FIRE_SUPPORT order can be assigned*/
extern BOOL droidSensorDroidWeapon(BASE_OBJECT *psObj, DROID *psDroid);

// return whether a droid has a CB sensor on it
extern BOOL cbSensorDroid(DROID *psDroid);
// return whether a droid has a standard sensor on it (standard, VTOL strike, or wide spectrum)
extern BOOL standardSensorDroid(DROID *psDroid);

// give a droid from one player to another - used in Electronic Warfare and multiplayer
extern DROID *giftSingleDroid(DROID *psD, uint32_t to);
/*calculates the electronic resistance of a droid based on its experience level*/
extern int16_t   droidResistance(DROID *psDroid);

/*this is called to check the weapon is 'allowed'. Check if VTOL, the weapon is
direct fire. Also check numVTOLattackRuns for the weapon is not zero - return
true if valid weapon*/
extern BOOL checkValidWeaponForProp(DROID_TEMPLATE *psTemplate);

extern const char *getDroidNameForRank(uint32_t rank);

/*called when a Template is deleted in the Design screen*/
extern void deleteTemplateFromProduction(DROID_TEMPLATE *psTemplate, uint8_t player);

// Select a droid and do any necessary housekeeping.
extern void SelectDroid(DROID *psDroid);

// De-select a droid and do any necessary housekeeping.
extern void DeSelectDroid(DROID *psDroid);

/*calculate the power cost to repair a droid*/
extern uint16_t powerReqForDroidRepair(DROID *psDroid);

/*power cost for One repair point*/
extern uint16_t repairPowerPoint(DROID *psDroid);

/* audio finished callback */
extern BOOL droidAudioTrackStopped( void *psObj );

/*returns true if droid type is one of the Cyborg types*/
extern BOOL cyborgDroid(const DROID *psDroid);

// check for illegal references to droid we want to release
BOOL droidCheckReferences(DROID *psVictimDroid);

/** Check if droid is in a legal world position and is not on its way to drive off the map. */
BOOL droidOnMap(const DROID *psDroid);

void droidSetPosition(DROID *psDroid, int x, int y);

static inline int droidSensorRange(const DROID *psDroid)
{
	return objSensorRange((const BASE_OBJECT *)psDroid);
}

static inline int droidSensorPower(const DROID *psDroid)
{
	return objSensorPower((const BASE_OBJECT *)psDroid);
}

static inline int droidJammerRange(const DROID *psDroid)
{
	return objJammerRange((const BASE_OBJECT *)psDroid);
}

static inline int droidJammerPower(const DROID *psDroid)
{
	return objJammerPower((const BASE_OBJECT *)psDroid);
}

static inline int droidConcealment(const DROID *psDroid)
{
	return objConcealment((const BASE_OBJECT *)psDroid);
}

/*
 * Component stat helper functions
 */
static inline BODY_STATS *getBodyStats(DROID *psDroid)
{
	return asBodyStats + psDroid->asBits[COMP_BODY].nStat;
}

static inline BRAIN_STATS *getBrainStats(DROID *psDroid)
{
	return asBrainStats + psDroid->asBits[COMP_BRAIN].nStat;
}

static inline PROPULSION_STATS *getPropulsionStats(DROID *psDroid)
{
	return asPropulsionStats + psDroid->asBits[COMP_PROPULSION].nStat;
}

static inline SENSOR_STATS *getSensorStats(DROID *psDroid)
{
	return asSensorStats + psDroid->asBits[COMP_SENSOR].nStat;
}

static inline ECM_STATS *getECMStats(DROID *psDroid)
{
	return asECMStats + psDroid->asBits[COMP_ECM].nStat;
}

static inline REPAIR_STATS *getRepairStats(DROID *psDroid)
{
	return asRepairStats + psDroid->asBits[COMP_REPAIRUNIT].nStat;
}

static inline CONSTRUCT_STATS *getConstructStats(DROID *psDroid)
{
	return asConstructStats + psDroid->asBits[COMP_CONSTRUCT].nStat;
}

static inline WEAPON_STATS *getWeaponStats(DROID *psDroid, int weapon_slot)
{
	return asWeaponStats + psDroid->asWeaps[weapon_slot].nStat;
}

/** helper functions for future refcount patch **/

#define setDroidTarget(_psDroid, _psNewTarget) _setDroidTarget(_psDroid, _psNewTarget, __LINE__, __FUNCTION__)
static inline void _setDroidTarget(DROID *psDroid, BASE_OBJECT *psNewTarget, int line, const char *func)
{
	psDroid->psTarget = psNewTarget;
	ASSERT(psNewTarget == NULL || !psNewTarget->died, "setDroidTarget: Set dead target");
	ASSERT(psNewTarget == NULL || !psNewTarget->died || (psNewTarget->died == NOT_CURRENT_LIST && psDroid->died == NOT_CURRENT_LIST),
		   "setDroidTarget: Set dead target");
#ifdef DEBUG
	psDroid->targetLine = line;
	sstrcpy(psDroid->targetFunc, func);
#else
	// Prevent warnings about unused parameters
	(void)line;
	(void)func;
#endif
}

#define setDroidActionTarget(_psDroid, _psNewTarget, _idx) _setDroidActionTarget(_psDroid, _psNewTarget, _idx, __LINE__, __FUNCTION__)
static inline void _setDroidActionTarget(DROID *psDroid, BASE_OBJECT *psNewTarget, uint16_t idx, int line, const char *func)
{
	psDroid->psActionTarget[idx] = psNewTarget;
	ASSERT(psNewTarget == NULL || !psNewTarget->died || (psNewTarget->died == NOT_CURRENT_LIST && psDroid->died == NOT_CURRENT_LIST),
		   "setDroidActionTarget: Set dead target");
#ifdef DEBUG
	psDroid->actionTargetLine[idx] = line;
	sstrcpy(psDroid->actionTargetFunc[idx], func);
#else
	// Prevent warnings about unused parameters
	(void)line;
	(void)func;
#endif
}

#define setDroidBase(_psDroid, _psNewTarget) _setDroidBase(_psDroid, _psNewTarget, __LINE__, __FUNCTION__)
static inline void _setDroidBase(DROID *psDroid, STRUCTURE *psNewBase, int line, const char *func)
{
	psDroid->psBaseStruct = psNewBase;
	ASSERT(psNewBase == NULL || !psNewBase->died, "setDroidBase: Set dead target");
#ifdef DEBUG
	psDroid->baseLine = line;
	sstrcpy(psDroid->baseFunc, func);
#else
	// Prevent warnings about unused parameters
	(void)line;
	(void)func;
#endif
}

static inline void setSaveDroidTarget(DROID *psSaveDroid, BASE_OBJECT *psNewTarget)
{
	psSaveDroid->psTarget = psNewTarget;
#ifdef DEBUG
	psSaveDroid->targetLine = 0;
	sstrcpy(psSaveDroid->targetFunc, "savegame");
#endif
}

static inline void setSaveDroidActionTarget(DROID *psSaveDroid, BASE_OBJECT *psNewTarget, uint16_t idx)
{
	psSaveDroid->psActionTarget[idx] = psNewTarget;
#ifdef DEBUG
	psSaveDroid->actionTargetLine[idx] = 0;
	sstrcpy(psSaveDroid->actionTargetFunc[idx], "savegame");
#endif
}

static inline void setSaveDroidBase(DROID *psSaveDroid, STRUCTURE *psNewBase)
{
	psSaveDroid->psBaseStruct = psNewBase;
#ifdef DEBUG
	psSaveDroid->baseLine = 0;
	sstrcpy(psSaveDroid->baseFunc, "savegame");
#endif
}

void checkDroid(const DROID *droid, const char *const location_description, const char *function, const int recurse);

/* assert if droid is bad */
#define CHECK_DROID(droid) checkDroid(droid, AT_MACRO, __FUNCTION__, max_check_object_recursion)

// Minimum damage a weapon will deal to its target
#define	MIN_WEAPON_DAMAGE	1
void templateSetParts(const DROID *psDroid, DROID_TEMPLATE *psTemplate);

#endif // __INCLUDED_SRC_DROID_H__
