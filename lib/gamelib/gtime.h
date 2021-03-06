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
/*! \file gtime.h
 * \brief Interface to the game clock.
 *
 */
#ifndef _gtime_h
#define _gtime_h

/** The number of ticks per second for the game clock. */
#define GAME_TICKS_PER_SEC 1000

/** The maximum time for one frame (stops the clock running away when debugging)
 * changed to /6 by ajl. if this needs to go back to ticks/10 then tell me. */
#define GTIME_MAXFRAME (GAME_TICKS_PER_SEC/4)

/** The current time in the game world. */
extern uint32_t gameTime;

extern float frameTimeFraction;		/**< Private performance calculation. Do not use. */
extern float frameTimeFraction2;	/**< Private performance calculation. Do not use. */

/**
 *	The time for the last frame. This tells you the amount of real world time
 *	that is spent by the current slice, or in other words, the time difference
 *	between the current frame and the previous frame.
 */
extern uint32_t frameTime;

/** The current time in the game world - never stops. */
extern uint32_t gameTime2;

/**
 *	The time for the last frame - never stops.
 *	@see frameTime
 */
extern uint32_t frameTime2;

/** Initialise the game clock. */
extern BOOL gameTimeInit(void);

/** Call this each loop to update the game timer. */
extern void gameTimeUpdate(void);

/* Returns true if gameTime is stopped. */
extern BOOL gameTimeIsStopped(void);

/** Call this to stop the game timer. */
extern void gameTimeStop(void);

/** Call this to restart the game timer after a call to gameTimeStop. */
extern void gameTimeStart(void);

/** Call this to reset the game timer. */
extern void gameTimeReset(uint32_t time);

/**
 *	Reset the game time modifiers.
 *	@see gameTimeSetMod
 */
void gameTimeResetMod(void);

/** Set the time modifier. Used to speed up the game. */
void gameTimeSetMod(float mod);

/** Get the current time modifier. */
void gameTimeGetMod(float *pMod);

/**
 * Return a number that climbs over tickFrequency game ticks and ends up in the required range.
 * For instance getTimeValueRange(4096,256) will return a number that cycles through the values
 * 0..256 every 4.096 seconds... Ensure that the first is an integer multiple of the second.
 * Useful for periodical stuff.
 *
 * Unlike getStaticTimeValueRange this function operates on uninterrupted game time.
*/
extern uint32_t	getTimeValueRange(uint32_t tickFrequency, uint32_t requiredRange);

/**
 * Return a number that climbs over tickFrequency game ticks and ends up in the required range.
 * For instance getTimeValueRange(4096,256) will return a number that cycles through the values
 * 0..256 every 4.096 seconds... Ensure that the first is an integer multiple of the second.
 * Useful for periodical stuff.
 *
 * Operates on game time, which is subject to pause.
*/
extern	uint32_t	getStaticTimeValueRange(uint32_t tickFrequency, uint32_t requiredRange);

/** Break down given time into its constituent components. */
extern void	getTimeComponents(uint32_t time, uint32_t *hours, uint32_t *minutes, uint32_t *seconds);

/**
 * Return an incremental value adjusted for the time we have available this frame. Basically
 * multiplies the passed value by delta time.
 * @param value Amount to change something in a second.
 * @param pauseTime If true, adjust also for pause of game time. Generally use true in-game, false for GUI.
 * @return Amount to change this frame.
 */
static inline float timeAdjustedIncrement(float value, BOOL pauseTime)
{
	if (pauseTime)
	{
		return (value * frameTimeFraction);
	}
	else
	{
		return (value * frameTimeFraction2);
	}
}

#endif
