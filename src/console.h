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

#ifndef __INCLUDED_SRC_CONSOLE_H__
#define __INCLUDED_SRC_CONSOLE_H__

#include "lib/ivis_common/piedef.h"

#define MAX_CONSOLE_MESSAGES			(64)
#define MAX_CONSOLE_STRING_LENGTH		(255)
#define MAX_CONSOLE_TMP_STRING_LENGTH	(255)

#define	DEFAULT_MESSAGE_DURATION		GAME_TICKS_PER_SEC * 8

#define CON_BORDER_WIDTH				4
#define CON_BORDER_HEIGHT				4

typedef enum
{
	LEFT_JUSTIFY,
	RIGHT_JUSTIFY,
	CENTRE_JUSTIFY,
	DEFAULT_JUSTIFY
} CONSOLE_TEXT_JUSTIFICATION;

typedef struct _console
{
	uint32_t	topX;
	uint32_t	topY;
	uint32_t	width;
	uint32_t	textDepth;
	BOOL	permanent;
} CONSOLE;

/* ID to use for addConsoleMessage() in case of a system message */
#define	SYSTEM_MESSAGE				(-1)
#define NOTIFY_MESSAGE				(-2)		// mainly used for lobby messages
#define CMD_MESSAGE					(-3)		// Used for console command system.

/* Definition of a message */
typedef struct	_console_message
{
	char				text[MAX_CONSOLE_STRING_LENGTH];		// Text of the message
	uint32_t				timeAdded;								// When was it added to our list?
	//uint32_t			screenIndex;							// Info for justification
	uint32_t				JustifyType;
	uint32_t				id;
	int32_t				player;						// Player who sent this message or SYSTEM_MESSAGE
	struct _console_message *psNext;
} CONSOLE_MESSAGE;

extern char ConsoleString[MAX_CONSOLE_TMP_STRING_LENGTH];

extern void	consolePrintf				( char *layout, ... );
extern BOOL	addConsoleMessage			( const char *messageText, CONSOLE_TEXT_JUSTIFICATION jusType, int32_t player );
extern void	updateConsoleMessages		( void );
extern void	initConsoleMessages			( void );
extern void	setConsoleMessageDuration	( uint32_t time );
extern void	removeTopConsoleMessage		( void );
extern void	displayConsoleMessages		( void );
extern void	flushConsoleMessages		( void );
extern void	setConsoleBackdropStatus	( BOOL state );
extern void	enableConsoleDisplay		( BOOL state );
extern BOOL getConsoleDisplayStatus		( void );
extern void	setDefaultConsoleJust		( CONSOLE_TEXT_JUSTIFICATION defJ );
extern void	setConsoleSizePos			( uint32_t x, uint32_t y, uint32_t width );
extern void	setConsolePermanence		( BOOL state, BOOL bClearOld );
extern BOOL	mouseOverConsoleBox			( void );
extern uint32_t	getNumberConsoleMessages( void );
extern void	setConsoleLineInfo			( uint32_t vis );
extern uint32_t	getConsoleLineInfo		( void );
extern void	permitNewConsoleMessages		( BOOL allow);
extern	void	toggleConsoleDrop( void );
extern void printf_console(const char *pFormat, ...); /// Print to the ingame console in debug mode only
extern void console(const char *pFormat, ...); /// Print allways to the ingame console

/* Basic wrapper to sprintf - allows convenient printf style game info to be displayed */


//	#ifdef DEBUG
//	#define DBCONPRINTF(x)			consolePrintf x
//	#else
//	#define DBCONPRINTF(x)
//	#endif
//
//	#define CONPRINTF(x)			consolePrintf x

/*
 Usage:

  CONPRINTF(StringPointer,(StringPointer,"format",data));
  DBCONPRINTF(StringPointer,(StringPointer,"format",data));

  StringPointer should always be ConsoleString.

 eg.

	CONPRINTF(ConsoleString,(ConsoleString,"Hello %d",123));

	Doing it this way will work on both PC and Playstation.

	Be very carefull that the resulting string is no longer
	than MAX_CONSOLE_TMP_STRING_LENGTH.
*/

#define CONPRINTF(s,x) \
	sprintf x; \
	addConsoleMessage(s,DEFAULT_JUSTIFY,SYSTEM_MESSAGE)

#ifdef DEBUG
#define DBCONPRINTF(s,x) \
	sprintf x; \
	addConsoleMessage(s,DEFAULT_JUSTIFY,SYSTEM_MESSAGE)
#else
#define DBCONPRINTF(s,x)
#endif

#endif // __INCLUDED_SRC_CONSOLE_H__
