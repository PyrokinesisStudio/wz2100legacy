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
 *  Functions for the messages shown in the Intelligence Map View
 */

#ifndef __INCLUDED_SRC_MESSAGE_H__
#define __INCLUDED_SRC_MESSAGE_H__

#include "structure.h"
#include "messagedef.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#define NO_AUDIO_MSG		-1

    /** The lists of messages allocated. */
    extern MESSAGE		*apsMessages[MAX_PLAYERS];

    /** The IMD to use for the proximity messages. */
    extern iIMDShape	*pProximityMsgIMD;

    /** The list of proximity displays allocated. */
    extern PROXIMITY_DISPLAY *apsProxDisp[MAX_PLAYERS];

    /** Allocates the viewdata heap. */
    BOOL initViewData(void);

    /** Initialise the message heaps. */
    BOOL initMessage(void);

    /** Release the message heaps. */
    BOOL messageShutdown(void);

    /** Add a message to the list. */
    MESSAGE *addMessage(MESSAGE_TYPE msgType, BOOL proxPos, uint32_t player);

    /** Add a beacon message to the list. */
    MESSAGE *addBeaconMessage(MESSAGE_TYPE msgType, BOOL proxPos, uint32_t player);

    /** Remove a message. */
    void removeMessage(MESSAGE *psDel, uint32_t player);

    /** Remove all Messages. */
    void freeMessages(void);

    /** Removes all the proximity displays. */
    void releaseAllProxDisp(void);

    bool addToViewDataList(VIEWDATA *psViewData, unsigned int numData);

    /** Load the view data for the messages from the file exported from the world editor. */
    VIEWDATA *loadViewData(const char *pViewMsgData, uint32_t bufferSize);

    VIEWDATA *loadResearchViewData(const char *fileName);

    /** Get the view data that contains the text message pointer passed in. */
    VIEWDATA *getViewData(const char *pTextMsg);

    /** Release the viewdata memory. */
    void viewDataShutDown(VIEWDATA *psViewData);

// Unused
    PROXIMITY_DISPLAY *getProximityDisplay(MESSAGE *psMessage);

    /** Looks through the players list of messages to find one with the same viewData
      * pointer and which is the same type of message - used in scriptFuncs. */
    MESSAGE *findMessage(MSG_VIEWDATA *pViewdata, MESSAGE_TYPE type, uint32_t player);

    /** 'Displays' a proximity display. */
    void displayProximityMessage(PROXIMITY_DISPLAY *psProxDisp);

    BOOL messageInitVars(void);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __INCLUDED_SRC_MESSAGE_H__
