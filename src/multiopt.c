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
 * MultiOpt.c
 *
 * Alex Lee,97/98, Pumpkin Studios
 *
 * Routines for setting the game options and starting the init process.
 */
#include "lib/framework/frame.h"			// for everything
#include "map.h"
#include "game.h"			// for loading maps
#include "message.h"		// for clearing messages.
#include "main.h"
#include "display3d.h"		// for changing the viewpoint
#include "power.h"
#include "lib/widget/widget.h"
#include "lib/gamelib/gtime.h"
#include "lib/netplay/netplay.h"
#include "hci.h"
#include "configuration.h"			// lobby cfg.
#include "clparse.h"
#include "lib/ivis_common/piestate.h"

#include "component.h"
#include "console.h"
#include "multiplay.h"
#include "lib/sound/audio.h"
#include "multijoin.h"
#include "frontend.h"
#include "levels.h"
#include "multistat.h"
#include "multiint.h"
#include "multilimit.h"
#include "multigifts.h"
#include "aiexperience.h"	//for beacon messages
#include "multiint.h"
#include "multirecv.h"
#include "spectate.h"
#include <SDL.h>

// ////////////////////////////////////////////////////////////////////////////
// External Variables

extern char	buildTime[8];
extern uint8_t numDrumsNeeded;
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////

// send complete game info set!
void sendOptions()
{
	unsigned int i;

	NETbeginEncode(NET_OPTIONS, NET_ALL_PLAYERS);

	// First send information about the game
	NETuint8_t(&game.type);
	NETstring(game.map, 128);
	NETuint8_t(&game.maxPlayers);
	NETstring(game.name, 128);
	NETbool(&game.fog);
	NETbool(&AllowSpectating);
	NETuint32_t(&game.power);
	NETuint8_t(&game.base);
	NETuint8_t(&game.alliance);
	NETbool(&game.scavengers);

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		NETuint8_t(&game.skDiff[i]);
	}

	// Send the list of who is still joining
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		NETbool(&ingame.JoiningInProgress[i]);
	}

	// Same goes for the alliances
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		unsigned int j;

		for (j = 0; j < MAX_PLAYERS; j++)
		{
			NETuint8_t(&alliances[i][j]);
		}
	}

	// Send the number of structure limits to expect
	NETuint32_t(&ingame.numStructureLimits);
	debug(LOG_NET, "(Host) Structure limits to process on client is %u", ingame.numStructureLimits);
	// Send the structures changed
	for (i = 0; i < ingame.numStructureLimits; i++)
	{
		NETuint8_t(&ingame.pStructureLimits[i].id);
		NETuint8_t(&ingame.pStructureLimits[i].limit);
	}
	updateLimitFlags();
	NETuint8_t(&ingame.flags);

	NETend();
}

// ////////////////////////////////////////////////////////////////////////////
/*!
 * check the wdg files that are being used.
 */
static BOOL checkGameWdg(const char *nm)
{
	LEVEL_DATASET *lev;

	for (lev = psLevels; lev; lev = lev->psNext)
	{
		if (strcmp(lev->pName, nm) == 0)
		{
			return true;
		}
	}

	return false;
}

// ////////////////////////////////////////////////////////////////////////////
// options for a game. (usually recvd in frontend)
void recvOptions()
{
	unsigned int i;

	debug(LOG_NET, "Receiving options from host");
	NETbeginDecode(NET_OPTIONS);

	// Get general information about the game
	NETuint8_t(&game.type);
	NETstring(game.map, 128);
	NETuint8_t(&game.maxPlayers);
	NETstring(game.name, 128);
	NETbool(&game.fog);
	NETbool(&AllowSpectating);
	NETuint32_t(&game.power);
	NETuint8_t(&game.base);
	NETuint8_t(&game.alliance);
	NETbool(&game.scavengers);

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		NETuint8_t(&game.skDiff[i]);
	}

	// Send the list of who is still joining
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		NETbool(&ingame.JoiningInProgress[i]);
	}

	// Alliances
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		unsigned int j;

		for (j = 0; j < MAX_PLAYERS; j++)
		{
			NETuint8_t(&alliances[i][j]);
		}
	}
	netPlayersUpdated = true;

	// Free any structure limits we may have in-place
	if (ingame.numStructureLimits)
	{
		ingame.numStructureLimits = 0;
		free(ingame.pStructureLimits);
		ingame.pStructureLimits = NULL;
	}

	// Get the number of structure limits to expect
	NETuint32_t(&ingame.numStructureLimits);
	debug(LOG_NET, "Host is sending us %u structure limits", ingame.numStructureLimits);
	// If there were any changes allocate memory for them
	if (ingame.numStructureLimits)
	{
		ingame.pStructureLimits = malloc(ingame.numStructureLimits * sizeof(MULTISTRUCTLIMITS));
	}

	for (i = 0; i < ingame.numStructureLimits; i++)
	{
		NETuint8_t(&ingame.pStructureLimits[i].id);
		NETuint8_t(&ingame.pStructureLimits[i].limit);
	}
	NETuint8_t(&ingame.flags);

	// Do the skirmish slider settings if they are up
	for (i = 0; i < MAX_PLAYERS; i++)
	{
		if (widgGetFromID(psWScreen, MULTIOP_SKSLIDE + i))
		{
			widgSetSliderPos(psWScreen, MULTIOP_SKSLIDE + i, game.skDiff[i]);
		}
	}
	debug(LOG_NET, "Rebuilding map list");
	// clear out the old level list.
	levShutDown();
	levInitialise();
	rebuildSearchPath(mod_multiplay, true);	// MUST rebuild search path for the new maps we just got!
	buildMapList();
	// See if we have the map or not
	if (!checkGameWdg(game.map))
	{
		uint32_t player = selectedPlayer;

		debug(LOG_NET, "Map was not found, requesting map %s from host.", game.map);
		// Request the map from the host
		NETbeginEncode(NET_FILE_REQUESTED, NET_HOST_ONLY);
		NETuint32_t(&player);
		NETend();

		addConsoleMessage("MAP REQUESTED!", DEFAULT_JUSTIFY, SYSTEM_MESSAGE);
	}
	else
	{
		loadMapPreview(false);
	}
}


// ////////////////////////////////////////////////////////////////////////////
// Host Campaign.
BOOL hostCampaign(char *sGame, char *sPlayer)
{
	PLAYERSTATS playerStats;
	uint32_t		i;

	debug(LOG_WZ, "Hosting campaign: '%s', player: '%s'", sGame, sPlayer);

	freeMessages();

	// If we had a single player (i.e. campaign) game before this value will
	// have been set to 0. So revert back to the default value.
	if (game.maxPlayers == 0)
	{
		game.maxPlayers = 4;
	}

	if (!NEThostGame(sGame, sPlayer, game.type, 0, 0, 0, game.maxPlayers))
	{
		return false;
	}

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		setPlayerName(i, ""); //Clear custom names (use default ones instead)
	}

	NetPlay.players[selectedPlayer].ready = false;

	ingame.localJoiningInProgress = true;
	ingame.JoiningInProgress[selectedPlayer] = true;
	bMultiPlayer = true;
	bMultiMessages = true; // enable messages

	loadMultiStats(sPlayer, &playerStats);				// stats stuff
	setMultiStats(selectedPlayer, playerStats, false);
	setMultiStats(selectedPlayer, playerStats, true);

	if(!NetPlay.bComms)
	{
		strcpy(NetPlay.players[0].name, sPlayer);
	}
	NetPlay.maxPlayers = game.maxPlayers;

	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// Join Campaign

BOOL joinCampaign(uint32_t gameNumber, char *sPlayer)
{
	PLAYERSTATS	playerStats;

	if(!ingame.localJoiningInProgress)
	{
		if (!NETjoinGame(gameNumber, sPlayer))	// join
		{
			return false;
		}
		ingame.localJoiningInProgress	= true;

		loadMultiStats(sPlayer, &playerStats);
		setMultiStats(selectedPlayer, playerStats, false);
		setMultiStats(selectedPlayer, playerStats, true);
		return true;
	}

	return false;
}

// ////////////////////////////////////////////////////////////////////////////
// Tell the host we are leaving the game 'nicely', (we wanted to) and not
// because we have some kind of error. (dropped or disconnected)
BOOL sendLeavingMsg(void)
{
	debug(LOG_NET, "We are leaving 'nicely'");
	NETbeginEncode(NET_PLAYER_LEAVING, NET_HOST_ONLY);
	{
		BOOL host = NetPlay.isHost;
		uint32_t id = selectedPlayer;

		NETuint32_t(&id);
		NETbool(&host);
	}
	NETend();

	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// called in Init.c to shutdown the whole netgame gubbins.
BOOL multiShutdown(void)
{
	// shut down netplay lib.
	debug(LOG_MAIN, "shutting down networking");
	NETshutdown();

	debug(LOG_MAIN, "free game data (structure limits)");
	if(ingame.numStructureLimits)
	{
		ingame.numStructureLimits = 0;
		free(ingame.pStructureLimits);
		ingame.pStructureLimits = NULL;
	}

	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// copy templates from one player to another.
BOOL addTemplateToList(DROID_TEMPLATE *psNew, DROID_TEMPLATE **ppList)
{
	DROID_TEMPLATE *psTempl = malloc(sizeof(DROID_TEMPLATE));

	if (psTempl == NULL)
	{
		debug(LOG_ERROR, "addTemplate: Out of memory");
		return false;
	}
	memcpy(psTempl, psNew, sizeof(DROID_TEMPLATE));
	psTempl->pName = NULL;

	if (psNew->pName)
	{
		psTempl->pName = strdup(psNew->pName);
	}

	psTempl->psNext = *ppList;
	*ppList = psTempl;

	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// copy templates from one player to another.
BOOL addTemplate(uint32_t player, DROID_TEMPLATE *psNew)
{
	return addTemplateToList(psNew, &apsDroidTemplates[player]);
}

// ////////////////////////////////////////////////////////////////////////////
// setup templates
BOOL multiTemplateSetup(void)
{
	return true;
}



// ////////////////////////////////////////////////////////////////////////////
// remove structures from map before campaign play.
static BOOL cleanMap(uint32_t player)
{
	DROID		*psD, *psD2;
	STRUCTURE	*psStruct;
	BOOL		firstFact, firstRes;

	bMultiPlayer = false;
	bMultiMessages = false;

	firstFact = true;
	firstRes = true;

	// reverse so we always remove the last object. re-reverse afterwards.
//	reverseObjectList((BASE_OBJECT**)&apsStructLists[player]);

	switch(game.base)
	{
		case CAMP_CLEAN:									//clean map
			while(apsStructLists[player])					//strip away structures.
			{
				removeStruct(apsStructLists[player], true);
			}
			psD = apsDroidLists[player];					// remove all but construction droids.
			while(psD)
			{
				psD2 = psD->psNext;
				//if(psD->droidType != DROID_CONSTRUCT)
				if (!(psD->droidType == DROID_CONSTRUCT ||
						psD->droidType == DROID_CYBORG_CONSTRUCT))
				{
					killDroid(psD);
				}
				psD = psD2;
			}
			break;

		case CAMP_BASE:												//just structs, no walls
			psStruct = apsStructLists[player];
			while(psStruct)
			{
				if ( (psStruct->pStructureType->type == REF_WALL)
						|| (psStruct->pStructureType->type == REF_WALLCORNER)
						|| (psStruct->pStructureType->type == REF_DEFENSE)
						|| (psStruct->pStructureType->type == REF_BLASTDOOR)
						|| (psStruct->pStructureType->type == REF_CYBORG_FACTORY)
						|| (psStruct->pStructureType->type == REF_COMMAND_CONTROL)
				   )
				{
					removeStruct(psStruct, true);
					psStruct = apsStructLists[player];			//restart,(list may have changed).
				}

				else if( (psStruct->pStructureType->type == REF_FACTORY)
						 || (psStruct->pStructureType->type == REF_RESEARCH)
						 || (psStruct->pStructureType->type == REF_POWER_GEN))
				{
					if(psStruct->pStructureType->type == REF_FACTORY )
					{
						if(firstFact == true)
						{
							firstFact = false;
							removeStruct(psStruct, true);
							psStruct = apsStructLists[player];
						}
						else	// don't delete, just rejig!
						{
							if(((FACTORY *)psStruct->pFunctionality)->capacity != 0)
							{
								((FACTORY *)psStruct->pFunctionality)->capacity = 0;
								((FACTORY *)psStruct->pFunctionality)->productionOutput = (uint8_t)((PRODUCTION_FUNCTION *)psStruct->pStructureType->asFuncList[0])->productionOutput;

								psStruct->sDisplay.imd	= psStruct->pStructureType->pIMD;
								psStruct->body			= (uint16_t)(structureBody(psStruct));

							}
							psStruct				= psStruct->psNext;
						}
					}
					else if(psStruct->pStructureType->type == REF_RESEARCH)
					{
						if(firstRes == true)
						{
							firstRes = false;
							removeStruct(psStruct, true);
							psStruct = apsStructLists[player];
						}
						else
						{
							if(((RESEARCH_FACILITY *)psStruct->pFunctionality)->capacity != 0)
							{
								// downgrade research
								((RESEARCH_FACILITY *)psStruct->pFunctionality)->capacity = 0;
								((RESEARCH_FACILITY *)psStruct->pFunctionality)->researchPoints = ((RESEARCH_FUNCTION *)psStruct->pStructureType->asFuncList[0])->researchPoints;
								psStruct->sDisplay.imd	= psStruct->pStructureType->pIMD;
								psStruct->body			= (uint16_t)(structureBody(psStruct));
							}
							psStruct = psStruct->psNext;
						}
					}
					else if(psStruct->pStructureType->type == REF_POWER_GEN)
					{
						if(((POWER_GEN *)psStruct->pFunctionality)->capacity != 0)
						{
							// downgrade powergen.
							((POWER_GEN *)psStruct->pFunctionality)->capacity = 0;
							((POWER_GEN *)psStruct->pFunctionality)->power = ((POWER_GEN_FUNCTION *)psStruct->pStructureType->asFuncList[0])->powerOutput;
							((POWER_GEN *)psStruct->pFunctionality)->multiplier += ((POWER_GEN_FUNCTION *)psStruct->pStructureType->asFuncList[0])->powerMultiplier;

							psStruct->sDisplay.imd	= psStruct->pStructureType->pIMD;
							psStruct->body			= (uint16_t)(structureBody(psStruct));
						}
						structurePowerUpgrade(psStruct);
						psStruct = psStruct->psNext;
					}
				}

				else
				{
					psStruct = psStruct->psNext;
				}
			}
			break;


		case CAMP_WALLS:												//everything.
			break;
		default:
			debug( LOG_FATAL, "Unknown Campaign Style" );
			abort();
			break;
	}

	// rerev list to get back to normal.
//	reverseObjectList((BASE_OBJECT**)&apsStructLists[player]);

	bMultiPlayer = true;
	bMultiMessages = true;
	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// setup a campaign game
static BOOL campInit(void)
{
	uint32_t			player;

	// If this is from a savegame, stop here!
	if((getSaveGameType() == GTYPE_SAVE_START)
			|| (getSaveGameType() == GTYPE_SAVE_MIDMISSION)	)
	{
		// these two lines are the biggest hack in the world.
		// the reticule seems to get detached from 'reticuleup'
		// this forces it back in sync...
		intRemoveReticule();
		intAddReticule();

		return true;
	}

	for(player = 0; player < game.maxPlayers; player++)			// clean up only to the player limit for this map..
	{
		cleanMap(player);
	}

	for (player = 1; player < MAX_PLAYERS; player++)
	{
		// we want to remove disabled AI & all the other players that don't belong
		if ((game.skDiff[player] == 0 || player >= game.maxPlayers) && (!game.scavengers || player != 7))
		{
			clearPlayer(player, true);			// do this quietly
			debug(LOG_NET, "removing disabled AI (%d) from map.", player);
		}
	}


	if (NetPlay.isHost)	// add oil drums
	{
		numDrumsNeeded = 0;
		addOilDrum(NetPlay.playercount * 2);
	}

	playerResponding();			// say howdy!

	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// say hi to everyone else....
void playerResponding(void)
{
	ingame.startTime = gameTime;
	ingame.localJoiningInProgress = false; // No longer joining.
	ingame.JoiningInProgress[selectedPlayer] = false;

	// Home the camera to the player
	cameraToHome(selectedPlayer, false);

	// Tell the world we're here
	NETbeginEncode(NET_PLAYERRESPONDING, NET_ALL_PLAYERS);
	NETuint32_t(&selectedPlayer);
	NETend();
}

// ////////////////////////////////////////////////////////////////////////////
//called when the game finally gets fired up.
BOOL multiGameInit(void)
{
	uint32_t player;

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		openchannels[player] = true;								//open comms to this player.
	}

	campInit();

	InitializeAIExperience();
	msgStackReset();	//for multiplayer msgs, reset message stack


	return true;
}

////////////////////////////////
// at the end of every game.
BOOL multiGameShutdown(void)
{
	PLAYERSTATS	st;

	debug(LOG_NET, "%s is shutting down.", getPlayerName(selectedPlayer));

	sendLeavingMsg();							// say goodbye
	updateMultiStatsGames();					// update games played.

	st = getMultiStats(selectedPlayer);	// save stats

	saveMultiStats(getPlayerName(selectedPlayer), getPlayerName(selectedPlayer), &st);

	// if we terminate the socket too quickly, then, it is possible not to get the leave message
	SDL_Delay(1000);
	ingame.TimeEveryoneIsInGame = 0;
	// close game
	NETclose();
	NETremRedirects();

	if (ingame.numStructureLimits)
	{
		ingame.numStructureLimits = 0;
		free(ingame.pStructureLimits);
		ingame.pStructureLimits = NULL;
	}
	ingame.flags = 0;

	ingame.localJoiningInProgress = false; // Clean up
	ingame.localOptionsReceived = false;
	ingame.bHostSetup = false;	// Dont attempt a host
	ingame.startTime = 0;
	NetPlay.isHost					= false;
	bMultiPlayer					= false;	// Back to single player mode
	bMultiMessages					= false;
	selectedPlayer					= 0;		// Back to use player 0 (single player friendly)

	return true;
}
