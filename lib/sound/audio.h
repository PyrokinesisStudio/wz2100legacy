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

#ifndef __INCLUDED_LIB_SOUND_AUDIO_H__
#define __INCLUDED_LIB_SOUND_AUDIO_H__

#include "track.h"

#if defined(__cplusplus)
extern "C"
{
#endif

extern BOOL		audio_Init( AUDIO_CALLBACK pStopTrackCallback );
extern void		audio_Update(void);
extern BOOL		audio_Shutdown(void);
extern BOOL		audio_Disabled( void );

extern BOOL		audio_LoadTrackFromFile( char szFileName[] );
extern unsigned int audio_SetTrackVals(const char *fileName, BOOL loop, unsigned int volume, unsigned int audibleRadius);

extern BOOL		audio_PlayStaticTrack( int32_t iX, int32_t iY, int iTrack );
extern BOOL		audio_PlayObjStaticTrack( void *psObj, int iTrack );
extern BOOL		audio_PlayObjStaticTrackCallback( void *psObj, int iTrack,
		AUDIO_CALLBACK pUserCallback );
extern BOOL		audio_PlayObjDynamicTrack( void *psObj, int iTrack,
		AUDIO_CALLBACK pUserCallback );
extern BOOL		audio_PlayClusterDynamicTrack( void *psClusterObj,
		int iTrack, AUDIO_CALLBACK pUserCallback );
extern void		audio_StopObjTrack( void *psObj, int iTrack );
extern void		audio_PlayTrack( int iTrack );
extern void		audio_PlayCallbackTrack( int iTrack,
		AUDIO_CALLBACK pUserCallback );
extern AUDIO_STREAM *audio_PlayStream(const char *fileName, float volume, void (*onFinished)(void *), void *user_data);
extern void		audio_QueueTrack( int32_t iTrack );
extern void		audio_QueueTrackMinDelay( int32_t iTrack, uint32_t iMinDelay );
extern void		audio_QueueTrackMinDelayPos( int32_t iTrack, uint32_t iMinDelay,
		int32_t iX, int32_t iY, int32_t iZ);
extern void		audio_QueueTrackGroup( int32_t iTrack, int32_t iGroup );
extern void		audio_QueueTrackPos( int32_t iTrack, int32_t iX, int32_t iY,
									 int32_t iZ );
extern void		audio_QueueTrackGroupPos( int32_t iTrack, int32_t iGroup,
		int32_t iX, int32_t iY, int32_t iZ );
extern BOOL		audio_GetPreviousQueueTrackPos( int32_t *iX, int32_t *iY,
		int32_t *iZ );
extern BOOL		audio_GetPreviousQueueTrackRadarBlipPos( int32_t *iX, int32_t *iY);
extern void		audio_PauseAll( void );
extern void		audio_ResumeAll( void );
extern void		audio_StopAll( void );

extern int32_t	audio_GetTrackID( const char *fileName );
extern void audio_RemoveObj(const void *psObj);
extern unsigned int audio_GetSampleQueueCount(void);
extern unsigned int audio_GetSampleListCount(void);
extern unsigned int sound_GetActiveSamplesCount(void);

#if defined(__cplusplus)
}
#endif

extern void 		audioTest(void);

#endif // __INCLUDED_LIB_SOUND_AUDIO_H__
