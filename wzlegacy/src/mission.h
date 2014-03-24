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
 * Mission defines for the game
 */

#ifndef __INCLUDED_SRC_MISSION_H__
#define __INCLUDED_SRC_MISSION_H__

#include "map.h"
#include "power.h"
#include "missiondef.h"
#include "group.h"

/*the number of areas that can be defined to prevent buildings being placed -
used for Transporter Landing Zones 0-7 are for players, 8 = LIMBO_LANDING*/
#define		MAX_NOGO_AREAS			9
#define     LIMBO_LANDING           8

// Set by scrFlyInTransporter. True if were currenly tracking the transporter.
extern BOOL		bTrackingTransporter;

extern MISSION		mission;
extern BOOL			offWorldKeepLists;
extern DROID       *apsLimboDroids[MAX_PLAYERS];
// return positions for vtols
extern Vector2i asVTOLReturnPos[MAX_PLAYERS];
extern bool Cheated;
extern void initMission(void);
extern BOOL missionShutDown(void);
extern void missionDestroyObjects(void);
//this is called everytime the game is quit
extern void releaseMission(void);


/*on the PC - sets the countdown played flag*/
extern void setMissionCountDown(void);


//extern BOOL startMission(MISSION_TYPE missionType, char *pGame);
extern BOOL startMission(uint32_t missionType, char *pGame);
extern void endMission(void);
// initialise the mission stuff for a save game
extern BOOL startMissionSave(int32_t missionType);

//sets up the game to start a new mission
//extern BOOL setUpMission(MISSION_TYPE type);
extern BOOL setUpMission(uint32_t type);
//this causes the new mission data to be loaded up
extern void launchMission(void);

/* The update routine for all Structures left back at base during a Mission*/
extern void missionStructureUpdate(STRUCTURE *psBuilding);
/* The update routine for all droids left back at home base
Only interested in Transporters at present*/
extern void missionDroidUpdate(DROID *psDroid);

extern BOOL missionIsOffworld(void);

extern BOOL missionCanReEnforce(void);

extern BOOL missionForReInforcements(void);
//returns true if the mission is a Limbo Expand mission
extern BOOL missionLimboExpand(void);

//this is called mid Limbo mission via the script
extern void resetLimboMission(void);

extern void swapMissionPointers(void);

// mission results.
#define		IDTIMER_FORM			11000
#define		IDTIMER_DISPLAY			11001
#define		IDMISSIONRES_FORM		11002
#define     IDMISSIONRES_QUIT		11007
#define     IDMISSIONRES_SAVE		11006

//timer display for transporter timer
#define		IDTRANTIMER_DISPLAY		11010

// Timer position defines
#define		TIMER_X					(568 + E_W)
#define		TIMER_Y					22

// status of the mission result screens.
extern BOOL MissionResUp;
extern BOOL ClosingMissionRes;

extern void intRemoveMissionResult			(void);
extern void intRemoveMissionResultNoAnim	(void);
extern void intProcessMissionResult			(uint32_t id);
extern void intRunMissionResult				(void);

extern void unloadTransporter(DROID *psTransporter, uint32_t x, uint32_t y,
                              BOOL goingHome);
/*sets the appropriate pause states for when the interface is up but the
game needs to be paused*/
extern void setMissionPauseState(void);
/*resets the pause states */
extern void resetMissionPauseState(void);
//returns the x coord for where the Transporter can land
extern uint16_t getLandingX( int32_t iPlayer );
//returns the y coord for where the Transporter can land
extern uint16_t getLandingY( int32_t iPlayer );
/*checks that the timer has been set and that a Transporter exists before
adding the timer button*/
extern void addTransporterTimerInterface(void);
extern void intRemoveTransporterTimer(void);
/*update routine for mission details */
extern void missionTimerUpdate(void);
/*checks the time has been set and then adds the timer if not already on
the display*/
extern void addMissionTimerInterface(void);
extern void intRemoveMissionTimer(void);


//access functions for bPlayCountDown flag
extern void setPlayCountDown(uint8_t set);
extern BOOL getPlayCountDown(void);


/*	checks the x,y passed in are not within the boundary of the Landing Zone
	x and y in tile coords */
extern BOOL withinLandingZone(uint32_t x, uint32_t y);
//sets the coords for the Transporter to land
extern void setLandingZone(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
extern LANDING_ZONE *getLandingZone(int32_t i);

/*Initialises all the nogo areas to 0*/
extern void initNoGoAreas(void);
//sets the coords for a no go area
extern void setNoGoArea(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t area);
/* fly in transporters at start of level */
extern void missionFlyTransportersIn( int32_t iPlayer, BOOL bTrackTransporter );
/* move transporter offworld */
extern void missionMoveTransporterOffWorld( DROID *psTransporter );
/* pick nearest map edge to point */
extern void missionGetNearestCorner( uint16_t iX, uint16_t iY,
                                     uint16_t *piOffX, uint16_t *piOffY );
extern void missionSetReinforcementTime( uint32_t iTime );
extern uint32_t  missionGetReinforcementTime(void);

/*builds a droid back at the home base whilst on a mission - stored in a list made
available to the transporter interface*/
extern DROID *buildMissionDroid(DROID_TEMPLATE *psTempl, uint32_t x, uint32_t y,
                                uint32_t player);

//This is just a very big number - bigger than a map width/height could ever be!
#define		INVALID_XY				(512 * 127)

extern void missionSetTransporterEntry( int32_t iPlayer, int32_t iEntryTileX, int32_t iEntryTileY );
extern void missionSetTransporterExit( int32_t iPlayer, int32_t iExitTileX, int32_t iExitTileY );
extern void missionGetTransporterEntry( int32_t iPlayer, uint16_t *iX, uint16_t *iY );
extern void missionGetTransporterExit( int32_t iPlayer, uint32_t *iX, uint32_t *iY );

//access functions for droidsToSafety flag
extern void setDroidsToSafetyFlag(BOOL set);
extern BOOL getDroidsToSafetyFlag(void);
//checks to see if the player has any droids (except Transporters left)
extern BOOL missionDroidsRemaining(uint32_t player);
/*called when a Transporter gets to the edge of the world and the droids are
being flown to safety. The droids inside the Transporter are placed into the
mission list for later use*/
extern void moveDroidsToSafety(DROID *psTransporter);

//called when ESC is pressed
extern void clearMissionWidgets(void);
//resets if return to game after an ESC
extern void resetMissionWidgets(void);

extern uint32_t	getCampaignNumber( void );
extern void	setCampaignNumber( uint32_t number );
extern BOOL intAddMissionResult(BOOL result, BOOL bPlaySuccess);
// reset the vtol landing pos
void resetVTOLLandingPos(void);

//this is called via a script function to place the Limbo droids once the mission has started
extern void placeLimboDroids(void);

//bCheating = true == start of cheat, bCheating = false == end of cheat
extern void setMissionCheatTime(BOOL bCheating);



#define		MISSIONRES_X			20	// pos & size of box.
#define		MISSIONRES_Y			380
#define		MISSIONRES_W			600
#define		MISSIONRES_H			80



#define		MISSIONRES_TITLE_X		20
#define		MISSIONRES_TITLE_Y		20
#define		MISSIONRES_TITLE_W		600
#define		MISSIONRES_TITLE_H		40

#endif // __INCLUDED_SRC_MISSION_H__
