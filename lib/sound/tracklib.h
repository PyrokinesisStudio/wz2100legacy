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
/** \file
 *  Library-specific sound library functions;
 *  these need to be re-written for each library.
 */

#ifndef __INCLUDED_LIB_SOUND_TRACKLIB_H__
#define __INCLUDED_LIB_SOUND_TRACKLIB_H__

#include "track.h"
#include "lib/framework/vector.h"

#if defined(__cplusplus)
extern "C"
{
#endif

BOOL	sound_InitLibrary( void );
void	sound_ShutdownLibrary( void );

void	sound_FreeTrack( TRACK *psTrack );

BOOL	sound_Play2DSample( TRACK *psTrack, AUDIO_SAMPLE *psSample,
							BOOL bQueued );
BOOL	sound_Play3DSample( TRACK *psTrack, AUDIO_SAMPLE *psSample );
void	sound_StopSample(AUDIO_SAMPLE *psSample);
void	sound_PauseSample( AUDIO_SAMPLE *psSample );
void	sound_ResumeSample( AUDIO_SAMPLE *psSample );

AUDIO_STREAM *sound_PlayStream(PHYSFS_file *PHYSFS_fileHandle, float volume, void (*onFinished)(void *), void *user_data);

void	sound_SetSampleFreq( AUDIO_SAMPLE *psSample, int32_t iFreq );
void	sound_SetSampleVol( AUDIO_SAMPLE *psSample, int32_t iVol,
							BOOL bScale3D );

int		sound_GetNumSamples( void );
BOOL	sound_SampleIsFinished( AUDIO_SAMPLE *psSample );
BOOL	sound_QueueSamplePlaying( void );

void	sound_SetPlayerPos(Vector3f pos);
void sound_SetPlayerOrientationVector(Vector3f forward, Vector3f up);
void sound_SetPlayerOrientation(float angle);
void	sound_SetObjectPosition( AUDIO_SAMPLE *psSample );

void 	*sound_GetObject( int32_t iSample );
void	sound_SetObject( int32_t iSample, void *pObj );

void	sound_SetCallback( int32_t iSample, AUDIO_CALLBACK pCallBack );

void	sound_PauseAll( void );
void	sound_ResumeAll( void );
void	sound_StopAll( void );
void	sound_Update( void );
unsigned int sound_GetActiveSamplesCount(void);

uint32_t	sound_GetGameTime( void );

#if defined(__cplusplus)
}
#endif

#endif	// __INCLUDED_LIB_SOUND_TRACKLIB_H__
