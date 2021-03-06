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
 * FrontEnd.c
 *
 * front end title and options screens.
 * Alex Lee. Pumpkin Studios. Eidos PLC 98,
 */

#include "lib/framework/frame.h"

#if defined(WZ_OS_WIN)
#   include <shellapi.h> /* For ShellExecute */
#endif

#include "lib/framework/input.h"
#include "lib/ivis_common/rendmode.h"
#include "lib/sound/mixer.h"
#include "lib/widget/button.h"
#include "lib/widget/label.h"
#include "lib/widget/slider.h"

#include "advvis.h"
#include "challenge.h"
#include "component.h"
#include "configuration.h"
#include "difficulty.h"
#include "display.h"
#include "frend.h"
#include "frontend.h"
#include "hci.h"
#include "init.h"
#include "intdisplay.h"
#include "keyedit.h"
#include "loadsave.h"
#include "main.h"
#include "modding.h"
#include "multiint.h"
#include "multilimit.h"
#include "multiplay.h"
#include "radar.h"
#include "seqdisp.h"
#include "texture.h"
#include "version.h"
#include "warzoneconfig.h"
#include "wrappers.h"

// ////////////////////////////////////////////////////////////////////////////
// Global variables

// Widget code and non-constant strings do not get along
static char resolution[WIDG_MAXSTR];
static char textureSize[WIDG_MAXSTR];

tMode titleMode; // the global case
tMode lastTitleMode; // Since skirmish and multiplayer setup use the same functions, we use this to go back to the corresponding menu.

char			aLevelName[MAX_LEVEL_NAME_SIZE + 1];	//256];			// vital! the wrf file to use.

BOOL			bLimiterLoaded = false;

#define DEFAULT_LEVEL "CAM_1A"
#define TUTORIAL_LEVEL "TUTORIAL3"


// Returns true if escape key pressed.
//
BOOL CancelPressed(void)
{
	return keyPressed(KEY_ESC);
}


// ////////////////////////////////////////////////////////////////////////////
// Title Screen
static BOOL startTitleMenu(void)
{
//	widgDelete(psWScreen,1);	// close reticule if it's open. MAGIC NUMBERS?
	intRemoveReticule();

	addBackdrop();
	addTopForm();
	addBottomForm();

	addTextButton(FRONTEND_SINGLEPLAYER, FRONTEND_POS2X, FRONTEND_POS2Y, _("Single Player"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_MULTIPLAYER, FRONTEND_POS3X, FRONTEND_POS3Y, _("Multi Player"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_TUTORIAL, FRONTEND_POS4X, FRONTEND_POS4Y, _("Tutorial"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_OPTIONS, FRONTEND_POS5X, FRONTEND_POS5Y, _("Options"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_PLAYINTRO, FRONTEND_POS6X, FRONTEND_POS6Y, _("View Intro"), WBUT_TXTCENTRE);

	addTextButton(FRONTEND_QUIT, FRONTEND_POS7X, FRONTEND_POS7Y, _("Quit Game"), WBUT_TXTCENTRE);

	addSideText(FRONTEND_SIDETEXT, FRONTEND_SIDEX, FRONTEND_SIDEY, _("MAIN MENU"));

	return true;
}

BOOL runTitleMenu(void)
{
	uint32_t id;

	id = widgRunScreen(psWScreen); // Run the current set of widgets

	switch(id)
	{
		case FRONTEND_QUIT:
			changeTitleMode(CREDITS);
			break;
		case FRONTEND_MULTIPLAYER:
			changeTitleMode(MULTI);
			break;
		case FRONTEND_SINGLEPLAYER:
			changeTitleMode(SINGLE);
			break;
		case FRONTEND_OPTIONS:
			changeTitleMode(OPTIONS);
			break;
		case FRONTEND_TUTORIAL:
			changeTitleMode(TUTORIAL);
			break;
		case FRONTEND_PLAYINTRO:
			changeTitleMode(SHOWINTRO);
			break;
			break;
		default:
			break;
	}

	widgDisplayScreen(psWScreen); // show the widgets currently running

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Tutorial Menu
static BOOL startTutorialMenu(void)
{
	addBackdrop();
	addTopForm();
	addBottomForm();


	addTextButton(FRONTEND_TUTORIAL, FRONTEND_POS3X, FRONTEND_POS3Y, _("Tutorial"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_FASTPLAY, FRONTEND_POS4X, FRONTEND_POS4Y, _("Fast Play"), WBUT_TXTCENTRE);
	addSideText	 (FRONTEND_SIDETEXT , FRONTEND_SIDEX, FRONTEND_SIDEY, _("TUTORIALS"));
	// TRANSLATORS: "Return", in this context, means "return to previous screen/menu"
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	return true;
}

BOOL runTutorialMenu(void)
{
	uint32_t id;

	id = widgRunScreen(psWScreen);						// Run the current set of widgets
	switch(id)
	{
		case FRONTEND_TUTORIAL:
			NetPlay.players[0].allocated = true;
			game.skDiff[0] = uint8_t_MAX;
			sstrcpy(aLevelName, TUTORIAL_LEVEL);
			changeTitleMode(STARTGAME);
			break;

		case FRONTEND_FASTPLAY:
			NetPlay.players[0].allocated = true;
			game.skDiff[0] = uint8_t_MAX;
			sstrcpy(aLevelName, "FASTPLAY");
			changeTitleMode(STARTGAME);
			break;

		case FRONTEND_QUIT:
			changeTitleMode(TITLE);
			break;

		default:
			break;
	}

	// If close button pressed then return from this menu.
	if(CancelPressed())
	{
		changeTitleMode(TITLE);
	}

	widgDisplayScreen(psWScreen);						// show the widgets currently running

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Single Player Menu
static void startSinglePlayerMenu(void)
{
	addBackdrop();
	addTopForm();
	addBottomForm();

	addTextButton(FRONTEND_NEWGAME,  FRONTEND_POS2X, FRONTEND_POS2Y, _("New Campaign") , WBUT_TXTCENTRE);
	addTextButton(FRONTEND_SKIRMISH, FRONTEND_POS3X, FRONTEND_POS3Y, _("Start Skirmish Game"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_CHALLENGES, FRONTEND_POS4X, FRONTEND_POS4Y, _("Challenges"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_LOADGAME, FRONTEND_POS5X, FRONTEND_POS5Y, _("Load Game"), WBUT_TXTCENTRE);

	addSideText	 (FRONTEND_SIDETEXT , FRONTEND_SIDEX, FRONTEND_SIDEY, _("SINGLE PLAYER"));
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);
}

static void frontEndNewGame( void )
{
	sstrcpy(aLevelName, DEFAULT_LEVEL);
	seq_ClearSeqList();
	seq_AddSeqToList("cam1/c001.ogg", NULL, "cam1/c001.txa", false);
	seq_StartNextFullScreenVideo();

	changeTitleMode(STARTGAME);
}

static void loadOK( void )
{
	if(strlen(sRequestResult))
	{
		sstrcpy(saveGameName, sRequestResult);
		changeTitleMode(LOADSAVEGAME);
	}
}

BOOL runSinglePlayerMenu(void)
{
	uint32_t id;

	if(bLoadSaveUp)
	{
		if(runLoadSave(false))// check for file name.
		{
			loadOK();
		}
	}
	else if (challengesUp)
	{
		runChallenges();
	}
	else
	{
		uint8_t playercolor = 0;

		// FIXME: We should do a SPinit() to make sure all the variables are reset correctly.
		// game.type is switched to SKIRMISH in startMultiOptions()
		NetPlay.bComms = false;
		bMultiPlayer = false;
		bMultiMessages = false;
		game.type = CAMPAIGN;
		NetPlay.players[0].allocated = true;
		game.skDiff[0] = uint8_t_MAX;
		// make sure we have a valid color choice for our SP game. Valid values are 0, 4-7
		playercolor = getPlayerColour(0);
		if (playercolor >= 1 && playercolor <= 3)
		{
			setPlayerColour(0, 0);		// default is green
		}

		id = widgRunScreen(psWScreen);						// Run the current set of widgets

		switch(id)
		{
			case FRONTEND_NEWGAME:
				frontEndNewGame();
				break;

			case FRONTEND_LOADCAM2:
				sstrcpy(aLevelName, "CAM_2A");
				changeTitleMode(STARTGAME);
				initLoadingScreen(true);
				break;

			case FRONTEND_LOADCAM3:
				sstrcpy(aLevelName, "CAM_3A");
				changeTitleMode(STARTGAME);
				initLoadingScreen(true);
				break;
			case FRONTEND_LOADGAME:
				addLoadSave(LOAD_FRONTEND, SaveGamePath, "gam", _("Load Saved Game"));	// change mode when loadsave returns
				break;

			case FRONTEND_SKIRMISH:
				ingame.bHostSetup = true;
				lastTitleMode = SINGLE;
				changeTitleMode(MULTIOPTION);
				break;

			case FRONTEND_QUIT:
				changeTitleMode(TITLE);
				break;

			case FRONTEND_CHALLENGES:
				addChallenges();
				break;

			default:
				break;
		}

		if(CancelPressed())
		{
			changeTitleMode(TITLE);
		}
	}

	if (!bLoadSaveUp && !challengesUp)						// if save/load screen is up
	{
		widgDisplayScreen(psWScreen);						// show the widgets currently running
	}
	if (bLoadSaveUp)								// if save/load screen is up
	{
		displayLoadSave();
	}
	else if (challengesUp)
	{
		displayChallenges();
	}

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Multi Player Menu
static BOOL startMultiPlayerMenu(void)
{
	addBackdrop();
	addTopForm();
	addBottomForm();

	addSideText	 (FRONTEND_SIDETEXT ,	FRONTEND_SIDEX, FRONTEND_SIDEY, _("MULTI PLAYER"));

	addTextButton(FRONTEND_HOST,     FRONTEND_POS2X, FRONTEND_POS2Y, _("Host Game"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_JOIN,     FRONTEND_POS3X, FRONTEND_POS3Y, _("Join Game"), WBUT_TXTCENTRE);

	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	return true;
}

BOOL runMultiPlayerMenu(void)
{
	uint32_t id;

	id = widgRunScreen(psWScreen);						// Run the current set of widgets
	switch(id)
	{
		case FRONTEND_HOST:
			// don't pretend we are running a network game. Really do it!
			NetPlay.bComms = true; // use network = true
			NETdiscoverUPnPDevices();
			ingame.bHostSetup = true;
			bMultiPlayer = true;
			bMultiMessages = true;
			game.type = SKIRMISH;		// needed?
			NetPlay.bComms = true;
			lastTitleMode = MULTI;
			changeTitleMode(MULTIOPTION);
			break;
		case FRONTEND_JOIN:
			ingame.bHostSetup = false;
			bMultiPlayer = true;
			bMultiMessages = true;
			if (getLobbyError() != ERROR_CHEAT)
			{
				setLobbyError(ERROR_NOERROR);
			}
			changeTitleMode(PROTOCOL);
			break;

		case FRONTEND_QUIT:
			changeTitleMode(TITLE);
			break;
		default:
			break;
	}

	widgDisplayScreen(psWScreen); // show the widgets currently running

	if (CancelPressed())
	{
		changeTitleMode(TITLE);
	}

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Options Menu
static BOOL startOptionsMenu(void)
{
	sliderEnableDrag(true);

	addBackdrop();
	addTopForm();
	addBottomForm();

	addSideText	 (FRONTEND_SIDETEXT ,	FRONTEND_SIDEX, FRONTEND_SIDEY, _("OPTIONS"));
	addTextButton(FRONTEND_GAMEOPTIONS,	FRONTEND_POS2X, FRONTEND_POS2Y, _("Game Options"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_GRAPHICSOPTIONS, FRONTEND_POS3X, FRONTEND_POS3Y, _("Graphics Options"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_VIDEOOPTIONS, FRONTEND_POS4X, FRONTEND_POS4Y, _("Video Options"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_AUDIOOPTIONS, FRONTEND_POS5X, FRONTEND_POS5Y, _("Audio Options"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_MOUSEOPTIONS, FRONTEND_POS6X, FRONTEND_POS6Y, _("Mouse Options"), WBUT_TXTCENTRE);
	addTextButton(FRONTEND_KEYMAP,		FRONTEND_POS7X, FRONTEND_POS7Y, _("Key Mappings"), WBUT_TXTCENTRE);
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	return true;
}

BOOL runOptionsMenu(void)
{
	uint32_t id;

	id = widgRunScreen(psWScreen);						// Run the current set of widgets
	switch(id)
	{

		case FRONTEND_GAMEOPTIONS:
			changeTitleMode(GAME);
			break;
		case FRONTEND_GRAPHICSOPTIONS:
			changeTitleMode(GRAPHICS_OPTIONS);
			break;
		case FRONTEND_AUDIOOPTIONS:
			changeTitleMode(AUDIO_OPTIONS);
			break;
		case FRONTEND_VIDEOOPTIONS:
			changeTitleMode(VIDEO_OPTIONS);
			break;
		case FRONTEND_MOUSEOPTIONS:
			changeTitleMode(MOUSE_OPTIONS);
			break;
		case FRONTEND_KEYMAP:
			changeTitleMode(KEYMAP);
			break;

		case FRONTEND_QUIT:
			changeTitleMode(TITLE);
			break;
		default:
			break;
	}

	// If close button pressed then return from this menu.
	if (CancelPressed())
	{
		changeTitleMode(TITLE);
	}

	widgDisplayScreen(psWScreen);						// show the widgets currently running

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Graphics Options
static BOOL startGraphicsOptionsMenu(void)
{
	addBackdrop();
	addTopForm();
	addBottomForm();

	////////////
	//FMV mode.
	addTextButton(FRONTEND_FMVMODE,	FRONTEND_POS2X - 35, FRONTEND_POS2Y, _("Video Playback"), 0);
	switch (war_GetFMVmode())
	{
		case FMV_1X:
			addTextButton(FRONTEND_FMVMODE_R, FRONTEND_POS2M - 55, FRONTEND_POS2Y, _("1X"), 0);
			break;

		case FMV_2X:
			addTextButton(FRONTEND_FMVMODE_R, FRONTEND_POS2M - 55, FRONTEND_POS2Y, _("2X"), 0);
			break;

		case FMV_FULLSCREEN:
			addTextButton(FRONTEND_FMVMODE_R, FRONTEND_POS2M - 55, FRONTEND_POS2Y, _("Fullscreen"), 0);
			break;

		default:
			ASSERT(!"invalid FMV mode", "Invalid FMV mode: %u", (unsigned int)war_GetFMVmode());
			break;
	}

	// Scanlines
	addTextButton(FRONTEND_SCANLINES, FRONTEND_POS3X - 35, FRONTEND_POS3Y, _("Scanlines"), 0);
	switch (war_getScanlineMode())
	{
		case SCANLINES_OFF:
			addTextButton(FRONTEND_SCANLINES_R, FRONTEND_POS3M - 55, FRONTEND_POS3Y, _("Off"), 0);
			break;

		case SCANLINES_50:
			addTextButton(FRONTEND_SCANLINES_R, FRONTEND_POS3M - 55, FRONTEND_POS3Y, _("50%"), 0);
			break;

		case SCANLINES_BLACK:
			addTextButton(FRONTEND_SCANLINES_R, FRONTEND_POS3M - 55, FRONTEND_POS3Y, _("Black"), 0);
			break;
	}

	////////////
	// screenshake
	addTextButton(FRONTEND_SSHAKE,	 FRONTEND_POS4X - 35,   FRONTEND_POS4Y, _("Screen Shake"), 0);
	if(getShakeStatus())
	{
		// shaking on
		addTextButton(FRONTEND_SSHAKE_R, FRONTEND_POS4M - 55,  FRONTEND_POS4Y, _("On"), 0);
	}
	else
	{
		//shaking off.
		addTextButton(FRONTEND_SSHAKE_R, FRONTEND_POS4M - 55,  FRONTEND_POS4Y, _("Off"), 0);
	}

	////////////
	// fog
	addTextButton(FRONTEND_FOGTYPE,	 FRONTEND_POS5X - 35,   FRONTEND_POS5Y, _("Fog"), 0);
	if(war_GetFog())
	{
		addTextButton(FRONTEND_FOGTYPE_R, FRONTEND_POS5M - 55, FRONTEND_POS5Y, _("Mist"), 0);
	}
	else
	{
		addTextButton(FRONTEND_FOGTYPE_R, FRONTEND_POS5M - 55, FRONTEND_POS5Y, _("Fog Of War"), 0);
	}

	////////////
	//subtitle mode.
	addTextButton(FRONTEND_SUBTITLES, FRONTEND_POS6X - 35, FRONTEND_POS6Y, _("Subtitles"), 0);
	if (!seq_GetSubtitles())
	{
		addTextButton(FRONTEND_SUBTITLES_R, FRONTEND_POS6M - 55, FRONTEND_POS6Y, _("Off"), 0);
	}
	else
	{
		addTextButton(FRONTEND_SUBTITLES_R, FRONTEND_POS6M - 55, FRONTEND_POS6Y, _("On"), 0);
	}

	////////////
	//shadows
	addTextButton(FRONTEND_SHADOWS, FRONTEND_POS7X - 35, FRONTEND_POS7Y, _("Shadows"), 0);
	if (getDrawShadows())
	{
		addTextButton(FRONTEND_SHADOWS_R, FRONTEND_POS7M - 55,  FRONTEND_POS7Y, _("On"), 0);
	}
	else
	{
		// not flipped
		addTextButton(FRONTEND_SHADOWS_R, FRONTEND_POS7M - 55,  FRONTEND_POS7Y, _("Off"), 0);
	}

	// Add some text down the side of the form
	addSideText(FRONTEND_SIDETEXT, FRONTEND_SIDEX, FRONTEND_SIDEY, _("GRAPHICS OPTIONS"));

	////////////
	// quit.
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	return true;
}

BOOL runGraphicsOptionsMenu(void)
{
	uint32_t id;
	int mode = 0;

	id = widgRunScreen(psWScreen);						// Run the current set of widgets
	switch(id)
	{
		case FRONTEND_SSHAKE:
		case FRONTEND_SSHAKE_R:
			if( getShakeStatus() )
			{
				setShakeStatus(false);
				widgSetString(psWScreen, FRONTEND_SSHAKE_R, _("Off"));
			}
			else
			{
				setShakeStatus(true);
				widgSetString(psWScreen, FRONTEND_SSHAKE_R, _("On"));
			}
			break;

		case FRONTEND_FOGTYPE:
		case FRONTEND_FOGTYPE_R:
			if (war_GetFog())
			{
				// turn off crap fog, turn on vis fog.
				debug(LOG_FOG, "runGameOptions2Menu: Fog of war ON, visual fog OFF");
				war_SetFog(false);
				widgSetString(psWScreen, FRONTEND_FOGTYPE_R, _("Fog Of War"));
			}
			else
			{
				// turn off vis fog, turn on normal crap fog.
				debug(LOG_FOG, "runGameOptions2Menu: Fog of war OFF, visual fog ON");
				war_SetFog(true);
				widgSetString(psWScreen, FRONTEND_FOGTYPE_R, _("Mist"));
			}
			break;

		case FRONTEND_QUIT:
			changeTitleMode(OPTIONS);
			break;

		case FRONTEND_SUBTITLES:
		case FRONTEND_SUBTITLES_R:
			if(	seq_GetSubtitles())
			{
				// turn off
				seq_SetSubtitles(false);
				widgSetString(psWScreen, FRONTEND_SUBTITLES_R, _("Off"));
			}
			else
			{
				// turn on
				seq_SetSubtitles(true);
				widgSetString(psWScreen, FRONTEND_SUBTITLES_R, _("On"));
			}
			break;

		case FRONTEND_SHADOWS:
		case FRONTEND_SHADOWS_R:
			setDrawShadows(!getDrawShadows());
			if (getDrawShadows())
			{
				widgSetString(psWScreen, FRONTEND_SHADOWS_R, _("On"));
			}
			else
			{
				widgSetString(psWScreen, FRONTEND_SHADOWS_R, _("Off"));
			}
			break;

		case FRONTEND_FMVMODE:
		case FRONTEND_FMVMODE_R:
			switch (mode = war_GetFMVmode())
			{
				case FMV_1X:
					war_SetFMVmode(mode + 1);
					widgSetString(psWScreen, FRONTEND_FMVMODE_R, _("2X"));
					break;

				case FMV_2X:
					war_SetFMVmode(mode + 1);
					widgSetString(psWScreen, FRONTEND_FMVMODE_R, _("Fullscreen"));
					break;

				case FMV_FULLSCREEN:
					war_SetFMVmode(mode + 1);
					widgSetString(psWScreen, FRONTEND_FMVMODE_R, _("1X"));
					break;

				default:
					ASSERT(!"invalid FMV mode", "Invalid FMV mode: %u", (unsigned int)mode);
					break;
			}
			break;

		case FRONTEND_SCANLINES:
		case FRONTEND_SCANLINES_R:
			switch (mode = war_getScanlineMode())
			{
				case SCANLINES_OFF:
					war_setScanlineMode(SCANLINES_50);
					widgSetString(psWScreen, FRONTEND_SCANLINES_R, _("50%"));
					break;

				case SCANLINES_50:
					war_setScanlineMode(SCANLINES_BLACK);
					widgSetString(psWScreen, FRONTEND_SCANLINES_R, _("Black"));
					break;

				case SCANLINES_BLACK:
					war_setScanlineMode(SCANLINES_OFF);
					widgSetString(psWScreen, FRONTEND_SCANLINES_R, _("Off"));
					break;
			}

		default:
			break;
	}


	// If close button pressed then return from this menu.
	if(CancelPressed())
	{
		changeTitleMode(OPTIONS);
	}

	widgDisplayScreen(psWScreen);						// show the widgets currently running

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Audio Options Menu
static BOOL startAudioOptionsMenu(void)
{
	addBackdrop();
	addTopForm();
	addBottomForm();

	// 2d audio
	addTextButton(FRONTEND_FX, FRONTEND_POS2X - 25, FRONTEND_POS2Y, _("Voice Volume"), 0);
	addFESlider(FRONTEND_FX_SL, FRONTEND_BOTFORM, FRONTEND_POS2M, FRONTEND_POS2Y + 5, AUDIO_VOL_MAX, sound_GetUIVolume() * 100.0);

	// 3d audio
	addTextButton(FRONTEND_3D_FX, FRONTEND_POS3X - 25, FRONTEND_POS3Y, _("FX Volume"), 0);
	addFESlider(FRONTEND_3D_FX_SL, FRONTEND_BOTFORM, FRONTEND_POS3M, FRONTEND_POS3Y + 5, AUDIO_VOL_MAX, sound_GetEffectsVolume() * 100.0);

	// cd audio
	addTextButton(FRONTEND_MUSIC, FRONTEND_POS4X - 25, FRONTEND_POS4Y, _("Music Volume"), 0);
	addFESlider(FRONTEND_MUSIC_SL, FRONTEND_BOTFORM, FRONTEND_POS4M, FRONTEND_POS4Y + 5, AUDIO_VOL_MAX, sound_GetMusicVolume() * 100.0);

	// quit.
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	//add some text down the side of the form
	addSideText	 (FRONTEND_SIDETEXT ,	FRONTEND_SIDEX, FRONTEND_SIDEY, _("AUDIO OPTIONS"));


	return true;
}

BOOL runAudioOptionsMenu(void)
{
	uint32_t id;

	id = widgRunScreen(psWScreen);						// Run the current set of widgets
	switch(id)
	{

		case FRONTEND_FX:
		case FRONTEND_3D_FX:
		case FRONTEND_MUSIC:
			break;

		case FRONTEND_FX_SL:
			sound_SetUIVolume((float)widgGetSliderPos(psWScreen, FRONTEND_FX_SL) / 100.0);
			break;

		case FRONTEND_3D_FX_SL:
			sound_SetEffectsVolume((float)widgGetSliderPos(psWScreen, FRONTEND_3D_FX_SL) / 100.0);
			break;

		case FRONTEND_MUSIC_SL:
			sound_SetMusicVolume((float)widgGetSliderPos(psWScreen, FRONTEND_MUSIC_SL) / 100.0);
			break;


		case FRONTEND_QUIT:
			changeTitleMode(OPTIONS);
			break;

		default:
			break;
	}

	// If close button pressed then return from this menu.
	if(CancelPressed())
	{
		changeTitleMode(OPTIONS);
	}

	widgDisplayScreen(psWScreen);						// show the widgets currently running

	return true;
}

// ////////////////////////////////////////////////////////////////////////////
// Video Options
static BOOL startVideoOptionsMenu(void)
{
	// Generate the resolution string
	snprintf(resolution, WIDG_MAXSTR, "%d x %d",
			 war_GetWidth(), war_GetHeight());
	// Generate texture size string
	snprintf(textureSize, WIDG_MAXSTR, "%d", getTextureSize());

	addBackdrop();
	addTopForm();
	addBottomForm();

	// Add a note about changes taking effect on restart for certain options
	addTextHint(FRONTEND_TAKESEFFECT, FRONTEND_POS1X + 48, FRONTEND_POS1Y + 24, _("* Takes effect on game restart"));

	// Fullscreen/windowed
	addTextButton(FRONTEND_WINDOWMODE, FRONTEND_POS2X - 35, FRONTEND_POS2Y, _("Graphics Mode*"), 0);

	if (war_getFullscreen())
	{
		addTextButton(FRONTEND_WINDOWMODE_R, FRONTEND_POS2M - 55, FRONTEND_POS2Y, _("Fullscreen"), 0);
	}
	else
	{
		addTextButton(FRONTEND_WINDOWMODE_R, FRONTEND_POS2M - 55, FRONTEND_POS2Y, _("Windowed"), 0);
	}

	// Resolution
	addTextButton(FRONTEND_RESOLUTION, FRONTEND_POS3X - 35, FRONTEND_POS3Y, _("Resolution*"), WBUT_SECONDARY);
	addTextButton(FRONTEND_RESOLUTION_R, FRONTEND_POS3M - 55, FRONTEND_POS3Y, resolution, WBUT_SECONDARY);
	widgSetString(psWScreen, FRONTEND_RESOLUTION_R, resolution);

	// Texture size
	addTextButton(FRONTEND_TEXTURESZ, FRONTEND_POS4X - 35, FRONTEND_POS4Y, _("Texture size"), 0);
	addTextButton(FRONTEND_TEXTURESZ_R, FRONTEND_POS4M - 55, FRONTEND_POS4Y, textureSize, 0);

	// Vsync
	addTextButton(FRONTEND_VSYNC, FRONTEND_POS5X - 35, FRONTEND_POS5Y, _("Vertical sync*"), 0);

	if (war_GetVsync())
	{
		addTextButton(FRONTEND_VSYNC_R, FRONTEND_POS5M - 55, FRONTEND_POS5Y, _("On"), 0);
	}
	else
	{
		addTextButton(FRONTEND_VSYNC_R, FRONTEND_POS5M - 55, FRONTEND_POS5Y, _("Off"), 0);
	}

	addTextButton(FRONTEND_TEXTURECOMPRESSION, FRONTEND_POS6X - 35, FRONTEND_POS6Y, _("Compress textures"), 0);

	if (war_GetTextureCompression())
	{
		addTextButton(FRONTEND_TEXTURECOMPRESSION_R, FRONTEND_POS6M - 55, FRONTEND_POS6Y, _("On"), 0);
	}
	else
	{
		addTextButton(FRONTEND_TEXTURECOMPRESSION_R, FRONTEND_POS6M - 55, FRONTEND_POS6Y, _("Off"), 0);
	}

	// Add some text down the side of the form
	addSideText(FRONTEND_SIDETEXT, FRONTEND_SIDEX, FRONTEND_SIDEY, _("VIDEO OPTIONS"));

	// Quit/return
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	return true;
}

BOOL runVideoOptionsMenu(void)
{
	SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);
	uint32_t id = widgRunScreen(psWScreen);

	switch (id)
	{
		case FRONTEND_WINDOWMODE:
		case FRONTEND_WINDOWMODE_R:
			if (war_getFullscreen())
			{
				war_setFullscreen(false);
				widgSetString(psWScreen, FRONTEND_WINDOWMODE_R, _("Windowed"));
			}
			else
			{
				war_setFullscreen(true);
				widgSetString(psWScreen, FRONTEND_WINDOWMODE_R, _("Fullscreen"));
			}
			break;

		case FRONTEND_RESOLUTION:
		case FRONTEND_RESOLUTION_R:
		{
			int current, count, oldcurrent;

			// Get the current mode offset
			for (count = 0, current = 0; modes[count]; count++)
			{
				if (war_GetWidth() == modes[count]->w
						&& war_GetHeight() == modes[count]->h)
				{
					current = count;
				}
			}

			// Increment and clip if required
			// Hide resolutions lower than Warzone can support
			oldcurrent = current;
			do
			{
				if (!mouseReleased(MOUSE_RMB))
				{
					if (--current < 0)
					{
						current = count - 1;
					}
				}
				else
				{
					if (++current == count)
					{
						current = 0;
					}
				}
			}
			while ((modes[current]->w < 640 || modes[current]->h < 480)
					&& current != oldcurrent);

			// Set the new width and height (takes effect on restart)
			war_SetWidth(modes[current]->w);
			war_SetHeight(modes[current]->h);

			// Generate the textual representation of the new width and height
			snprintf(resolution, WIDG_MAXSTR, "%d x %d", modes[current]->w,
					 modes[current]->h);

			// Update the widget
			widgSetString(psWScreen, FRONTEND_RESOLUTION_R, resolution);

			break;
		}

		case FRONTEND_TRAP:
		case FRONTEND_TRAP_R:
			if (war_GetTrapCursor())
			{
				war_SetTrapCursor(false);
				widgSetString(psWScreen, FRONTEND_TRAP_R, _("Off"));
			}
			else
			{
				war_SetTrapCursor(true);
				widgSetString(psWScreen, FRONTEND_TRAP_R, _("On"));
			}
			break;

		case FRONTEND_TEXTURESZ:
		case FRONTEND_TEXTURESZ_R:
		{
			int newTexSize = getTextureSize() * 2;

			// Clip such that 32 <= size <= 512
			if (newTexSize > 512)
			{
				newTexSize = 32;
			}

			// Set the new size
			setTextureSize(newTexSize);

			// Generate the string representation of the new size
			snprintf(textureSize, WIDG_MAXSTR, "%d", newTexSize);

			// Update the widget
			widgSetString(psWScreen, FRONTEND_TEXTURESZ_R, textureSize);

			break;
		}

		case FRONTEND_VSYNC:
		case FRONTEND_VSYNC_R:
		{
			if (war_GetVsync())
			{
				war_SetVsync(false);
				widgSetString(psWScreen, FRONTEND_VSYNC_R, _("Off"));
			}
			else
			{
				war_SetVsync(true);
				widgSetString(psWScreen, FRONTEND_VSYNC_R, _("On"));
			}
			break;
		}
		case FRONTEND_TEXTURECOMPRESSION:
		case FRONTEND_TEXTURECOMPRESSION_R:
		{
			if (war_GetTextureCompression())
			{
				war_SetTextureCompression(false);
				widgSetString(psWScreen, FRONTEND_TEXTURECOMPRESSION_R, _("Off"));
			}
			else
			{
				war_SetTextureCompression(true);
				widgSetString(psWScreen, FRONTEND_TEXTURECOMPRESSION_R, _("On"));
			}
			break;
		}

		case FRONTEND_QUIT:
			changeTitleMode(OPTIONS);
			break;

		default:
			break;
	}

	if (CancelPressed())
	{
		changeTitleMode(OPTIONS);
	}

	widgDisplayScreen(psWScreen);

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Mouse Options
static BOOL startMouseOptionsMenu(void)
{
	addBackdrop();
	addTopForm();
	addBottomForm();

	addTextHint(FRONTEND_TAKESEFFECT, FRONTEND_POS1X + 48, FRONTEND_POS1Y + 24, _("* May negatively affect performance"));

	////////////
	// mouseflip
	addTextButton(FRONTEND_MFLIP,	 FRONTEND_POS2X - 35,   FRONTEND_POS2Y, _("Reverse Rotation"), 0);
	if( getInvertMouseStatus() )
	{
		// flipped
		addTextButton(FRONTEND_MFLIP_R, FRONTEND_POS2M - 25,  FRONTEND_POS2Y, _("On"), 0);
	}
	else
	{
		// not flipped
		addTextButton(FRONTEND_MFLIP_R, FRONTEND_POS2M - 25,  FRONTEND_POS2Y, _("Off"), 0);
	}

	// Cursor trapping
	addTextButton(FRONTEND_TRAP, FRONTEND_POS3X - 35, FRONTEND_POS3Y, _("Trap Cursor"), 0);

	if (war_GetTrapCursor())
	{
		addTextButton(FRONTEND_TRAP_R, FRONTEND_POS3M - 25, FRONTEND_POS3Y, _("On"), 0);
	}
	else
	{
		addTextButton(FRONTEND_TRAP_R, FRONTEND_POS3M - 25, FRONTEND_POS3Y, _("Off"), 0);
	}

	// Hardware / software cursor toggle
	addTextButton(FRONTEND_CURSORMODE, FRONTEND_POS4X - 35, FRONTEND_POS4Y, _("Colored Cursors*"), 0);

	if (war_GetColouredCursor())
	{
		addTextButton(FRONTEND_CURSORMODE_R, FRONTEND_POS4M - 25, FRONTEND_POS4Y, _("On"), 0);
	}
	else
	{
		addTextButton(FRONTEND_CURSORMODE_R, FRONTEND_POS4M - 25, FRONTEND_POS4Y, _("Off"), 0);
	}

	////////////
	// left-click orders
	addTextButton(FRONTEND_MBUTTONS,	 FRONTEND_POS2X - 35,   FRONTEND_POS5Y, _("Switch Mouse Buttons"), 0);
	if( getRightClickOrders() )
	{
		// right-click orders
		addTextButton(FRONTEND_MBUTTONS_R, FRONTEND_POS2M - 25,  FRONTEND_POS5Y, _("On"), 0);
	}
	else
	{
		// left-click orders
		addTextButton(FRONTEND_MBUTTONS_R, FRONTEND_POS2M - 25,  FRONTEND_POS5Y, _("Off"), 0);
	}

	////////////
	// middle-click rotate
	addTextButton(FRONTEND_MMROTATE,	 FRONTEND_POS2X - 35,   FRONTEND_POS6Y, _("Rotate Screen"), 0);
	if( getMiddleClickRotate() )
	{
		// right-click orders
		addTextButton(FRONTEND_MMROTATE_R, FRONTEND_POS2M - 25,  FRONTEND_POS6Y, P_("middle mouse button", "Middle Button"), 0);
	}
	else
	{
		// left-click orders
		addTextButton(FRONTEND_MMROTATE_R, FRONTEND_POS2M - 25,  FRONTEND_POS6Y, P_("right mouse button", "Right Button"), 0);
	}

	// Add some text down the side of the form
	addSideText(FRONTEND_SIDETEXT, FRONTEND_SIDEX, FRONTEND_SIDEY, _("MOUSE OPTIONS"));

	// Quit/return
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	return true;
}

BOOL runMouseOptionsMenu(void)
{
	uint32_t id = widgRunScreen(psWScreen);

	switch (id)
	{

		case FRONTEND_MFLIP:
		case FRONTEND_MFLIP_R:
			if( getInvertMouseStatus() )
			{
				//	 flipped
				setInvertMouseStatus(false);
				widgSetString(psWScreen, FRONTEND_MFLIP_R, _("Off"));
			}
			else
			{
				// not flipped
				setInvertMouseStatus(true);
				widgSetString(psWScreen, FRONTEND_MFLIP_R, _("On"));
			}
			break;
		case FRONTEND_TRAP:
		case FRONTEND_TRAP_R:
			if (war_GetTrapCursor())
			{
				war_SetTrapCursor(false);
				widgSetString(psWScreen, FRONTEND_TRAP_R, _("Off"));
			}
			else
			{
				war_SetTrapCursor(true);
				widgSetString(psWScreen, FRONTEND_TRAP_R, _("On"));
			}
			break;

		case FRONTEND_CURSORMODE:
		case FRONTEND_CURSORMODE_R:
			if (war_GetColouredCursor())
			{
				war_SetColouredCursor(false);
				widgSetString(psWScreen, FRONTEND_CURSORMODE_R, _("Off"));
			}
			else
			{
				war_SetColouredCursor(true);
				widgSetString(psWScreen, FRONTEND_CURSORMODE_R, _("On"));
			}
			break;

		case FRONTEND_MBUTTONS:
		case FRONTEND_MBUTTONS_R:
			if( getRightClickOrders() )
			{
				setRightClickOrders(false);
				widgSetString(psWScreen, FRONTEND_MBUTTONS_R, _("Off"));
			}
			else
			{
				setRightClickOrders(true);
				widgSetString(psWScreen, FRONTEND_MBUTTONS_R, _("On"));
			}
			break;

		case FRONTEND_MMROTATE:
		case FRONTEND_MMROTATE_R:
			if( getMiddleClickRotate() )
			{
				setMiddleClickRotate(false);
				widgSetString(psWScreen, FRONTEND_MMROTATE_R, P_("right mouse button", "Right Button"));
			}
			else
			{
				setMiddleClickRotate(true);
				widgSetString(psWScreen, FRONTEND_MMROTATE_R, P_("middle mouse button", "Middle Button"));
			}
			break;

		case FRONTEND_QUIT:
			changeTitleMode(OPTIONS);
			break;

		default:
			break;
	}

	if (CancelPressed())
	{
		changeTitleMode(OPTIONS);
	}

	widgDisplayScreen(psWScreen);

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// Game Options Menu
static BOOL startGameOptionsMenu(void)
{
	uint32_t	w, h;
	int playercolor;

	addBackdrop();
	addTopForm();
	addBottomForm();

	// Difficulty
	addTextButton(FRONTEND_DIFFICULTY, FRONTEND_POS2X - 25, FRONTEND_POS2Y, _("Difficulty"), 0);
	switch (getDifficultyLevel())
	{
		case DL_EASY:
			addTextButton(FRONTEND_DIFFICULTY_R, FRONTEND_POS2M - 25, FRONTEND_POS2Y, _("Easy"), 0);
			break;
		case DL_NORMAL:
			addTextButton(FRONTEND_DIFFICULTY_R, FRONTEND_POS2M - 25, FRONTEND_POS2Y, _("Normal"), 0);
			break;
		case DL_HARD:
		default:
			addTextButton(FRONTEND_DIFFICULTY_R, FRONTEND_POS2M - 25, FRONTEND_POS2Y, _("Hard"), 0);
			break;
	}

	// Scroll speed
	addTextButton(FRONTEND_SCROLLSPEED, FRONTEND_POS3X - 25, FRONTEND_POS3Y, _("Scroll Speed"), 0);
	addFESlider(FRONTEND_SCROLLSPEED_SL, FRONTEND_BOTFORM, FRONTEND_POS3M, FRONTEND_POS3Y + 5, 16, scroll_speed_accel / 100);

	// Colour stuff
	w = iV_GetImageWidth(FrontImages, IMAGE_PLAYER0);
	h = iV_GetImageHeight(FrontImages, IMAGE_PLAYER0);

	addMultiBut(psWScreen, FRONTEND_BOTFORM, FE_P0, FRONTEND_POS4M + (0 * (w + 6)), FRONTEND_POS4Y, w, h, NULL, IMAGE_PLAYER0, IMAGE_PLAYERX, true);
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FE_P4, FRONTEND_POS4M + (1 * (w + 6)), FRONTEND_POS4Y, w, h, NULL, IMAGE_PLAYER4, IMAGE_PLAYERX, true);
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FE_P5, FRONTEND_POS4M + (2 * (w + 6)), FRONTEND_POS4Y, w, h, NULL, IMAGE_PLAYER5, IMAGE_PLAYERX, true);
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FE_P6, FRONTEND_POS4M + (3 * (w + 6)), FRONTEND_POS4Y, w, h, NULL, IMAGE_PLAYER6, IMAGE_PLAYERX, true);
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FE_P7, FRONTEND_POS4M + (4 * (w + 6)), FRONTEND_POS4Y, w, h, NULL, IMAGE_PLAYER7, IMAGE_PLAYERX, true);

	// language
	addTextButton(FRONTEND_LANGUAGE,  FRONTEND_POS2X - 25, FRONTEND_POS5Y, _("Language"), 0);
	addTextButton(FRONTEND_LANGUAGE_R,  FRONTEND_POS2M - 25, FRONTEND_POS5Y, getLanguageName(), 0);

	// FIXME: if playercolor = 1-3, then we Assert in widgSetButtonState() since we don't define FE_P1 - FE_P3
	// I assume the reason is that in SP games, those are reserved for the AI?  Valid values are 0, 4-7.
	// This is a workaround, until we find what is setting that to 1-3.  See configuration.c:701
	playercolor = getPlayerColour(0);
	if (playercolor >= 1 && playercolor <= 3)
	{
		playercolor = 0;
	}
	widgSetButtonState(psWScreen, FE_P0 + playercolor, WBUT_LOCK);
	addTextButton(FRONTEND_COLOUR, FRONTEND_POS4X - 25, FRONTEND_POS4Y, _("Unit Colour"), 0);

	// Radar
	addTextButton(FRONTEND_RADAR, FRONTEND_POS6X - 25, FRONTEND_POS6Y, _("Radar"), 0);
	addTextButton(FRONTEND_RADAR_R, FRONTEND_POS6M - 25, FRONTEND_POS6Y, rotateRadar ? _("Rotating") : _("Fixed"), 0);

	// Quit
	addMultiBut(psWScreen, FRONTEND_BOTFORM, FRONTEND_QUIT, 10, 10, 30, 29, P_("menu", "Return"), IMAGE_RETURN, IMAGE_RETURN_HI, IMAGE_RETURN_HI);

	// Add some text down the side of the form
	addSideText(FRONTEND_SIDETEXT, FRONTEND_SIDEX, FRONTEND_SIDEY, _("GAME OPTIONS"));

	return true;
}

BOOL runGameOptionsMenu(void)
{
	uint32_t id;

	id = widgRunScreen(psWScreen);						// Run the current set of widgets
	switch(id)
	{
		case FRONTEND_LANGUAGE_R:
			setNextLanguage();
			widgSetString(psWScreen, FRONTEND_LANGUAGE_R, getLanguageName());
			/* Hack to reset current menu text, which looks fancy. */
			widgSetString(psWScreen, FRONTEND_LANGUAGE, _("Language"));
			widgSetString(psWScreen, FRONTEND_COLOUR,  _("Unit Colour"));
			widgSetString(psWScreen, FRONTEND_DIFFICULTY, _("Difficulty"));
			widgSetString(psWScreen, FRONTEND_SCROLLSPEED, _("Scroll Speed"));
			widgSetString(psWScreen, FRONTEND_SIDETEXT, _("GAME OPTIONS"));
			// FIXME: Changing the below return button tooltip does not work.
			//widgSetString(psWScreen, FRONTEND_BOTFORM, P_("menu", "Return"));
			switch( getDifficultyLevel() )
			{
				case DL_EASY:
					widgSetString(psWScreen, FRONTEND_DIFFICULTY_R, _("Easy"));
					break;
				case DL_NORMAL:
					widgSetString(psWScreen, FRONTEND_DIFFICULTY_R, _("Normal"));
					break;
				case DL_HARD:
					widgSetString(psWScreen, FRONTEND_DIFFICULTY_R, _("Hard") );
					break;
				case DL_TOUGH:
				case DL_KILLER:
					debug(LOG_ERROR, "runGameOptionsMenu: Unused difficulty level selected!");
					break;
			}
			break;

		case FRONTEND_RADAR_R:
			rotateRadar = !rotateRadar;
			widgSetString(psWScreen, FRONTEND_RADAR_R, rotateRadar ? _("Rotating") : _("Fixed"));
			break;

		case FRONTEND_SCROLLSPEED:
			break;

		case FRONTEND_DIFFICULTY:
		case FRONTEND_DIFFICULTY_R:
			switch( getDifficultyLevel() )
			{
				case DL_EASY:
					setDifficultyLevel(DL_NORMAL);
					widgSetString(psWScreen, FRONTEND_DIFFICULTY_R, _("Normal"));
					break;
				case DL_NORMAL:
					setDifficultyLevel(DL_HARD);
					widgSetString(psWScreen, FRONTEND_DIFFICULTY_R, _("Hard") );
					break;
				case DL_HARD:
					setDifficultyLevel(DL_EASY);
					widgSetString(psWScreen, FRONTEND_DIFFICULTY_R, _("Easy"));
					break;
				default: // DL_TOUGH and DL_KILLER
					break;
			}
			break;

		case FRONTEND_SCROLLSPEED_SL:
			scroll_speed_accel = widgGetSliderPos(psWScreen, FRONTEND_SCROLLSPEED_SL) * 100; //0-1600
			if(scroll_speed_accel == 0)		// make sure you CAN scroll.
			{
				scroll_speed_accel = 100;
			}
			break;

		case FRONTEND_QUIT:
			changeTitleMode(OPTIONS);
			break;

		case FE_P0:
			widgSetButtonState(psWScreen, FE_P0, WBUT_LOCK);
			widgSetButtonState(psWScreen, FE_P4, 0);
			widgSetButtonState(psWScreen, FE_P5, 0);
			widgSetButtonState(psWScreen, FE_P6, 0);
			widgSetButtonState(psWScreen, FE_P7, 0);
			setPlayerColour(0, 0);
			break;
		case FE_P4:
			widgSetButtonState(psWScreen, FE_P0, 0);
			widgSetButtonState(psWScreen, FE_P4, WBUT_LOCK);
			widgSetButtonState(psWScreen, FE_P5, 0);
			widgSetButtonState(psWScreen, FE_P6, 0);
			widgSetButtonState(psWScreen, FE_P7, 0);
			setPlayerColour(0, 4);
			break;
		case FE_P5:
			widgSetButtonState(psWScreen, FE_P0, 0);
			widgSetButtonState(psWScreen, FE_P4, 0);
			widgSetButtonState(psWScreen, FE_P5, WBUT_LOCK);
			widgSetButtonState(psWScreen, FE_P6, 0);
			widgSetButtonState(psWScreen, FE_P7, 0);
			setPlayerColour(0, 5);
			break;
		case FE_P6:
			widgSetButtonState(psWScreen, FE_P0, 0);
			widgSetButtonState(psWScreen, FE_P4, 0);
			widgSetButtonState(psWScreen, FE_P5, 0);
			widgSetButtonState(psWScreen, FE_P6, WBUT_LOCK);
			widgSetButtonState(psWScreen, FE_P7, 0);
			setPlayerColour(0, 6);
			break;
		case FE_P7:
			widgSetButtonState(psWScreen, FE_P0, 0);
			widgSetButtonState(psWScreen, FE_P4, 0);
			widgSetButtonState(psWScreen, FE_P5, 0);
			widgSetButtonState(psWScreen, FE_P6, 0);
			widgSetButtonState(psWScreen, FE_P7, WBUT_LOCK);
			setPlayerColour(0, 7);
			break;

		default:
			break;
	}

	// If close button pressed then return from this menu.
	if(CancelPressed())
	{
		changeTitleMode(OPTIONS);
	}

	widgDisplayScreen(psWScreen);						// show the widgets currently running

	return true;
}


// ////////////////////////////////////////////////////////////////////////////
// drawing functions

// show a background piccy
static void displayTitleBitmap(WZ_DECL_UNUSED WIDGET *psWidget, WZ_DECL_UNUSED uint32_t xOffset, WZ_DECL_UNUSED uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	iV_SetFont(font_regular);
	iV_SetTextColour(WZCOL_GREY);
	iV_DrawTextRotated(GetVersionInfo(), pie_GetVideoBufferWidth() - 9, pie_GetVideoBufferHeight() - 14, 270.f);
	iV_SetTextColour(WZCOL_TEXT_BRIGHT);
	iV_DrawTextRotated(GetVersionInfo(), pie_GetVideoBufferWidth() - 10, pie_GetVideoBufferHeight() - 15, 270.f);
}

// ////////////////////////////////////////////////////////////////////////////
// show warzone logo
static void displayLogo(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	iV_DrawImage(FrontImages, IMAGE_FE_LOGO, xOffset + psWidget->x, yOffset + psWidget->y);

	if (*getModList())
	{
		// show the mod list
		char modListText[WIDG_MAXSTR] = "";
		sstrcat(modListText, _("Mod: "));
		sstrcat(modListText, getModList());
		iV_SetFont(font_regular);
		iV_SetTextColour(WZCOL_GREY);
		iV_DrawText(modListText, xOffset + psWidget->x - 9, yOffset + psWidget->y - 19);
		iV_SetTextColour(WZCOL_TEXT_BRIGHT);
		iV_DrawText(modListText, xOffset + psWidget->x - 10, yOffset + psWidget->y - 20);
	}
}


// ////////////////////////////////////////////////////////////////////////////
// show, well have a guess..
static void displayBigSlider(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	W_SLIDER *Slider = (W_SLIDER *)psWidget;
	uint32_t x = xOffset + psWidget->x;
	uint32_t y = yOffset + psWidget->y;
	int16_t sx;

	iV_DrawImage(IntImages, IMAGE_SLIDER_BIG, x + STAT_SLD_OX, y + STAT_SLD_OY);			// draw bdrop

	sx = (int16_t)((Slider->width - 3 - Slider->barSize) * Slider->pos / Slider->numStops);	// determine pos.
	iV_DrawImage(IntImages, IMAGE_SLIDER_BIGBUT, x + 3 + sx, y + 3);								//draw amount
}

static void displayAISlider(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	W_SLIDER *Slider = (W_SLIDER *)psWidget;
	uint32_t x = xOffset + psWidget->x;
	uint32_t y = yOffset + psWidget->y;
	int16_t sx;

	iV_DrawImage(IntImages, IMAGE_SLIDER_AI, x + STAT_SLD_OX, y + STAT_SLD_OY);			// draw bdrop

	sx = (int16_t)((Slider->width - 3 - Slider->barSize) * Slider->pos / Slider->numStops);	// determine pos.
	iV_DrawImage(IntImages, IMAGE_SLIDER_BIGBUT, x + 3 + sx, y + 3);								//draw amount
}


// ////////////////////////////////////////////////////////////////////////////
// show text.
static void displayText(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	int32_t			fx, fy, fw;
	W_LABEL		*psLab;

	psLab = (W_LABEL *)psWidget;
	iV_SetFont(psLab->FontID);

	fw = iV_GetTextWidth(psLab->aText);
	fy = yOffset + psWidget->y;

	if (psWidget->style & WLAB_ALIGNCENTRE)	//check for centering, calculate offset.
	{
		fx = xOffset + psWidget->x + ((psWidget->width - fw) / 2);
	}
	else
	{
		fx = xOffset + psWidget->x;
	}

	iV_SetTextColour(WZCOL_TEXT_BRIGHT);
	iV_DrawText( psLab->aText, fx, fy);

	return;
}

// ////////////////////////////////////////////////////////////////////////////
// show text written on its side.
static void displayTextAt270(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	int32_t		fx, fy;
	W_LABEL		*psLab;

	psLab = (W_LABEL *)psWidget;

	iV_SetFont(font_large);

	fx = xOffset + psWidget->x;
	fy = yOffset + psWidget->y + iV_GetTextWidth(psLab->aText) ;

	iV_SetTextColour(WZCOL_GREY);
	iV_DrawTextRotated(psLab->aText, fx + 2, fy + 2, 270.f);
	iV_SetTextColour(WZCOL_TEXT_BRIGHT);
	iV_DrawTextRotated(psLab->aText, fx, fy, 270.f);
}

// ////////////////////////////////////////////////////////////////////////////
// show a text option.
void displayTextOption(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, WZ_DECL_UNUSED PIELIGHT *pColours)
{
	int32_t			fx, fy, fw;
	W_BUTTON		*psBut;
	BOOL			hilight = false;
	BOOL			greyOut = psWidget->UserData; // if option is unavailable.

	psBut = (W_BUTTON *)psWidget;
	iV_SetFont(psBut->FontID);

	if(widgGetMouseOver(psWScreen) == psBut->id)					// if mouse is over text then hilight.
	{
		hilight = true;
	}

	fw = iV_GetTextWidth(psBut->pText);
	fy = yOffset + psWidget->y + (psWidget->height - iV_GetTextLineSize()) / 2 - iV_GetTextAboveBase();

	if (psWidget->style & WBUT_TXTCENTRE)							//check for centering, calculate offset.
	{
		fx = xOffset + psWidget->x + ((psWidget->width - fw) / 2);
	}
	else
	{
		fx = xOffset + psWidget->x;
	}

	if(greyOut)														// unavailable
	{
		iV_SetTextColour(WZCOL_TEXT_DARK);
	}
	else															// available
	{
		if(hilight)													// hilight
		{
			iV_SetTextColour(WZCOL_TEXT_BRIGHT);
		}
		else														// dont highlight
		{
			iV_SetTextColour(WZCOL_TEXT_MEDIUM);
		}
	}

	iV_DrawText( psBut->pText, fx, fy);

	return;
}


// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// common widgets.

void addBackdrop(void)
{
	W_FORMINIT		sFormInit;

	memset(&sFormInit, 0, sizeof(W_FORMINIT));				// Backdrop
	sFormInit.formID = 0;
	sFormInit.id = FRONTEND_BACKDROP;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = (int16_t)( (pie_GetVideoBufferWidth() - HIDDEN_FRONTEND_WIDTH) / 2);
	sFormInit.y = (int16_t)( (pie_GetVideoBufferHeight() - HIDDEN_FRONTEND_HEIGHT) / 2);
	sFormInit.width = HIDDEN_FRONTEND_WIDTH - 1;
	sFormInit.height = HIDDEN_FRONTEND_HEIGHT - 1;
	sFormInit.pDisplay = displayTitleBitmap;
	widgAddForm(psWScreen, &sFormInit);
}

// ////////////////////////////////////////////////////////////////////////////
void addTopForm(void)
{
	W_FORMINIT		sFormInit;

	memset(&sFormInit, 0, sizeof(W_FORMINIT));

	sFormInit.formID = FRONTEND_BACKDROP;
	sFormInit.id = FRONTEND_TOPFORM;
	sFormInit.style = WFORM_PLAIN;

	if(titleMode == MULTIOPTION)
	{
		sFormInit.x		= FRONTEND_TOPFORM_WIDEX;
		sFormInit.y		= FRONTEND_TOPFORM_WIDEY;
		sFormInit.width = FRONTEND_TOPFORM_WIDEW;
		sFormInit.height = FRONTEND_TOPFORM_WIDEH;
	}
	else

	{
		sFormInit.x		= FRONTEND_TOPFORMX;
		sFormInit.y		= FRONTEND_TOPFORMY;
		sFormInit.width = FRONTEND_TOPFORMW;
		sFormInit.height = FRONTEND_TOPFORMH;
	}
	sFormInit.pDisplay = intDisplayPlainForm;
	widgAddForm(psWScreen, &sFormInit);

	sFormInit.formID = FRONTEND_TOPFORM;
	sFormInit.id	= FRONTEND_LOGO;
	sFormInit.x		= (short)((sFormInit.width / 2) - (FRONTEND_LOGOW / 2)); //115;
	sFormInit.y		= (short)((sFormInit.height / 2) - (FRONTEND_LOGOH / 2)); //18;
	sFormInit.width = FRONTEND_LOGOW;
	sFormInit.height = FRONTEND_LOGOH;
	sFormInit.pDisplay = displayLogo;
	widgAddForm(psWScreen, &sFormInit);
}

// ////////////////////////////////////////////////////////////////////////////
void addBottomForm(void)
{
	W_FORMINIT		sFormInit;
	memset(&sFormInit, 0, sizeof(W_FORMINIT));

	sFormInit.formID = FRONTEND_BACKDROP;
	sFormInit.id = FRONTEND_BOTFORM;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = FRONTEND_BOTFORMX;
	sFormInit.y = FRONTEND_BOTFORMY;
	sFormInit.width = FRONTEND_BOTFORMW;
	sFormInit.height = FRONTEND_BOTFORMH;

	sFormInit.pDisplay = intOpenPlainForm;
	sFormInit.disableChildren = true;

	widgAddForm(psWScreen, &sFormInit);
}

// ////////////////////////////////////////////////////////////////////////////
void addTextHint(uint32_t id, uint32_t PosX, uint32_t PosY, const char *txt)
{
	W_LABINIT		sLabInit;
	memset(&sLabInit, 0, sizeof(W_LABINIT));

	sLabInit.formID = FRONTEND_BOTFORM;
	sLabInit.id = id;
	sLabInit.x = (short)PosX;
	sLabInit.y = (short)PosY;
	sLabInit.style = WLAB_PLAIN;

	sLabInit.width = MULTIOP_READY_WIDTH;
	sLabInit.height = FRONTEND_BUTHEIGHT;
	sLabInit.pDisplay = displayText;
	sLabInit.FontID = font_regular;
	sLabInit.pText = txt;
	widgAddLabel(psWScreen, &sLabInit);
}

// ////////////////////////////////////////////////////////////////////////////
void addText(uint32_t id, uint32_t PosX, uint32_t PosY, const char *txt, uint32_t formID)
{
	W_LABINIT		sLabInit;
	memset(&sLabInit, 0, sizeof(W_LABINIT));

	sLabInit.formID = formID;
	sLabInit.id = id;
	sLabInit.x = (short)PosX;
	sLabInit.y = (short)PosY;
	sLabInit.style = (WLAB_PLAIN | WLAB_ALIGNCENTRE);

	// Align
	sLabInit.width = MULTIOP_READY_WIDTH;
	//sButInit.x+=35;

	sLabInit.height = FRONTEND_BUTHEIGHT;
	sLabInit.pDisplay = displayText;
	sLabInit.FontID = font_small;
	sLabInit.pText = txt;
	widgAddLabel(psWScreen, &sLabInit);
}

// ////////////////////////////////////////////////////////////////////////////
void addSideText(uint32_t id,  uint32_t PosX, uint32_t PosY, const char *txt)
{
	W_LABINIT	sLabInit;
	memset(&sLabInit, 0, sizeof(W_LABINIT));

	sLabInit.formID = FRONTEND_BACKDROP;
	sLabInit.id = id;
	sLabInit.style = WLAB_PLAIN;
	sLabInit.x = (short) PosX;
	sLabInit.y = (short) PosY;
	sLabInit.width = 30;
	sLabInit.height = FRONTEND_BOTFORMH;

	sLabInit.FontID = font_large;

	sLabInit.pDisplay = displayTextAt270;
	sLabInit.pText = txt;
	widgAddLabel(psWScreen, &sLabInit);
}

// ////////////////////////////////////////////////////////////////////////////
void addTextButton(uint32_t id,  uint32_t PosX, uint32_t PosY, const char *txt, unsigned int style)
{
	W_BUTINIT		sButInit;
	memset(&sButInit, 0, sizeof(W_BUTINIT));

	sButInit.formID = FRONTEND_BOTFORM;
	sButInit.id = id;
	sButInit.x = (short)PosX;
	sButInit.y = (short)PosY;
	sButInit.style = WBUT_PLAIN;

	// Align
	if ( !(style & WBUT_TXTCENTRE) )
	{
		sButInit.width = (short)(iV_GetTextWidth(txt) + 10);
		sButInit.x += 35;
	}
	else
	{
		sButInit.style |= WBUT_TXTCENTRE;
		sButInit.width = FRONTEND_BUTWIDTH;
	}

	// Enable right clicks
	if (style & WBUT_SECONDARY)
	{
		sButInit.style |= WBUT_SECONDARY;
	}

	sButInit.UserData = (style & WBUT_DISABLE); // store disable state

	sButInit.height = FRONTEND_BUTHEIGHT;
	sButInit.pDisplay = displayTextOption;
	sButInit.FontID = font_large;
	sButInit.pText = txt;
	widgAddButton(psWScreen, &sButInit);

	// Disable button
	if (style & WBUT_DISABLE)
	{
		widgSetButtonState(psWScreen, id, WBUT_DISABLE);
	}
}

void addSmallTextButton(uint32_t id,  uint32_t PosX, uint32_t PosY, const char *txt, unsigned int style)
{
	W_BUTINIT sButInit;
	memset(&sButInit, 0, sizeof(W_BUTINIT));

	sButInit.formID = FRONTEND_BOTFORM;
	sButInit.id = id;
	sButInit.x = (short)PosX;
	sButInit.y = (short)PosY;

	// Align
	if ( !(style & WBUT_TXTCENTRE) )
	{
		iV_SetFont(font_small);
		sButInit.width = (short)(iV_GetTextWidth(txt) + 10);
		sButInit.x += 35;
	}
	else
	{
		sButInit.style |= WBUT_TXTCENTRE;
		sButInit.width = FRONTEND_BUTWIDTH;
	}

	// Enable right clicks
	if (style & WBUT_SECONDARY)
	{
		sButInit.style |= WBUT_SECONDARY;
	}

	sButInit.UserData = (style & WBUT_DISABLE); // store disable state

	sButInit.height = FRONTEND_BUTHEIGHT;
	sButInit.pDisplay = displayTextOption;
	sButInit.FontID = font_small;
	sButInit.pText = txt;
	widgAddButton(psWScreen, &sButInit);

	// Disable button
	if (style & WBUT_DISABLE)
	{
		widgSetButtonState(psWScreen, id, WBUT_DISABLE);
	}
}
// ////////////////////////////////////////////////////////////////////////////
void addFESlider(uint32_t id, uint32_t parent, uint32_t x, uint32_t y, uint32_t stops, uint32_t pos)
{
	W_SLDINIT		sSldInit;

	memset(&sSldInit, 0, sizeof(W_SLDINIT));
	sSldInit.formID		= parent;
	sSldInit.id			= id;
	sSldInit.style		= WSLD_PLAIN;
	sSldInit.x			= (short)x;
	sSldInit.y			= (short)y;
	sSldInit.width		= iV_GetImageWidth(IntImages, IMAGE_SLIDER_BIG);
	sSldInit.height		= iV_GetImageHeight(IntImages, IMAGE_SLIDER_BIG);
	sSldInit.orientation = WSLD_LEFT;
	sSldInit.numStops	= (uint8_t) stops;
	sSldInit.barSize	= iV_GetImageHeight(IntImages, IMAGE_SLIDER_BIG);
	sSldInit.pos		= (uint8_t) pos;
	sSldInit.pDisplay	= displayBigSlider;
	sSldInit.pCallback  = intUpdateQuantitySlider;
	widgAddSlider(psWScreen, &sSldInit);
}

void addFEAISlider(uint32_t id, uint32_t parent, uint32_t x, uint32_t y, uint32_t stops, uint32_t pos)
{
	W_SLDINIT		sSldInit;

	memset(&sSldInit, 0, sizeof(W_SLDINIT));
	sSldInit.formID		= parent;
	sSldInit.id			= id;
	sSldInit.style		= WSLD_PLAIN;
	sSldInit.x			= (short)x;
	sSldInit.y			= (short)y;
	sSldInit.width		= iV_GetImageWidth(IntImages, IMAGE_SLIDER_BIG);
	sSldInit.height		= iV_GetImageHeight(IntImages, IMAGE_SLIDER_BIG);
	sSldInit.orientation = WSLD_LEFT;
	sSldInit.numStops	= (uint8_t) stops;
	sSldInit.barSize	= iV_GetImageHeight(IntImages, IMAGE_SLIDER_BIG);
	sSldInit.pos		= (uint8_t) pos;
	sSldInit.pDisplay	= displayAISlider;
	sSldInit.pCallback  = intUpdateQuantitySlider;
	widgAddSlider(psWScreen, &sSldInit);
}


// ////////////////////////////////////////////////////////////////////////////
// Change Mode
void changeTitleMode(tMode mode)
{
	tMode oldMode;

	widgDelete(psWScreen, FRONTEND_BACKDROP);		// delete backdrop.

	oldMode = titleMode;							// store old mode
	titleMode = mode;								// set new mode

	switch(mode)
	{
		/*	case DEMOMODE:// demo case. remove for release
				startDemoMenu();
				break;
			case VIDEO:
				startVideoOptionsMenu();
				break;
		*/
		case SINGLE:
			startSinglePlayerMenu();
			break;
		case GAME:
			startGameOptionsMenu();
			break;

		case GRAPHICS_OPTIONS:
			startGraphicsOptionsMenu();
			break;

		case AUDIO_OPTIONS:
			startAudioOptionsMenu();
			break;

		case VIDEO_OPTIONS:
			startVideoOptionsMenu();
			break;

		case MOUSE_OPTIONS:
			startMouseOptionsMenu();
			break;

		case TUTORIAL:
			startTutorialMenu();
			break;
		case OPTIONS:
			startOptionsMenu();
			break;
		case TITLE:
			startTitleMenu();
			break;

//	case GRAPHICS:
//		startGraphicsOptionsMenu();
//		break;
		case CREDITS:
			startCreditsScreen();
			break;

		case MULTI:
			startMultiPlayerMenu();		// goto multiplayer menu
			break;
		case PROTOCOL:
			startConnectionScreen();
			break;
		case MULTIOPTION:
			if(oldMode == MULTILIMIT)
			{
				startMultiOptions(true);
			}
			else
			{
				startMultiOptions(false);
			}
			break;
		case GAMEFIND:
			startGameFind();
			break;
		case MULTILIMIT:
			startLimitScreen();
			break;
		case KEYMAP:
			startKeyMapEditor(true);
			break;

		case STARTGAME:
		case QUIT:
		case LOADSAVEGAME:
			bLimiterLoaded = false;
		case SHOWINTRO:
			break;

		default:
			debug( LOG_FATAL, "Unknown title mode requested" );
			abort();
			break;
	}

	/* Set default frame rate limit */
	setDefaultFrameRateLimit();

	return;
}

