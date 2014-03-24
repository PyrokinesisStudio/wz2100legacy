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
	MapDisplay - Renders the world view necessary for the intelligence map
	Alex McLean, Pumpkin Studios, EIDOS Interactive, 1997

	Makes heavy use of the functions available in display3d.c. Could have
	messed about with display3d.c to make to world render dual purpose, but
	it's neater as a separate file, as the intelligence map has special requirements
	and overlays and needs to render to a specified buffer for later use.
*/

#include "lib/framework/frame.h"
#include "lib/ivis_opengl/piematrix.h"

#include "component.h"
#include "intdisplay.h"
#include "mapdisplay.h"

#define ROTATE_TIME	(2*GAME_TICKS_PER_SEC)

/* renders the Research IMDs into the surface - used by message display in
Intelligence Map */
void renderResearchToBuffer(RESEARCH *psResearch,
                            uint32_t OriginX, uint32_t OriginY)
{
    uint32_t   angle = 0;

    BASE_STATS      *psResGraphic;
    uint32_t          compID, IMDType;
    Vector3i Rotation,Position;
    uint32_t          basePlateSize, Radius;
    int32_t          scale = 0;

    // Set identity (present) context
    pie_MatBegin();

    pie_SetGeometricOffset(OriginX+10,OriginY+10);

    // Pitch down a bit
    //pie_MatRotX(DEG(-30));

    // Rotate round
    // full rotation once every 2 seconds..
    angle = (gameTime2 % ROTATE_TIME) * 360 / ROTATE_TIME;

    Position.x = 0;
    Position.y = 0;
    Position.z = BUTTON_DEPTH;

    // Rotate round
    Rotation.x = -30;
    Rotation.y = angle;
    Rotation.z = 0;

    //draw the IMD for the research
    if (psResearch->psStat)
    {
        //we have a Stat associated with this research topic
        if  (StatIsStructure(psResearch->psStat))
        {
            //this defines how the button is drawn
            IMDType = IMDTYPE_STRUCTURESTAT;
            psResGraphic = psResearch->psStat;
            //set up the scale
            basePlateSize= getStructureStatSize((STRUCTURE_STATS *)psResearch->psStat);
            if(basePlateSize == 1)
            {
                scale = RESEARCH_COMPONENT_SCALE / 2;
                /*HACK HACK HACK!
                if its a 'tall thin (ie tower)' structure stat with something on
                the top - offset the position to show the object on top*/
                if (((STRUCTURE_STATS *)psResearch->psStat)->pIMD->nconnectors &&
                        getStructureStatHeight((STRUCTURE_STATS *)psResearch->psStat) > TOWER_HEIGHT)
                {
                    Position.y -= 30;
                }
            }
            else if(basePlateSize == 2)
            {
                scale = RESEARCH_COMPONENT_SCALE / 4;
            }
            else
            {
                scale = RESEARCH_COMPONENT_SCALE / 5;
            }
        }
        else
        {
            compID = StatIsComponent(psResearch->psStat);
            if (compID != COMP_UNKNOWN)
            {
                //this defines how the button is drawn
                IMDType = IMDTYPE_COMPONENT;
                psResGraphic = psResearch->psStat;
                scale = RESEARCH_COMPONENT_SCALE;
            }
            else
            {
                ASSERT( false, "intDisplayMessageButton: invalid stat" );
                IMDType = IMDTYPE_RESEARCH;
                psResGraphic = (BASE_STATS *)psResearch;
            }
        }
    }
    else
    {
        //no Stat for this research topic so use the research topic to define what is drawn
        psResGraphic = (BASE_STATS *)psResearch;
        IMDType = IMDTYPE_RESEARCH;
    }

    //scale the research according to size of IMD
    if (IMDType == IMDTYPE_RESEARCH)
    {
        Radius = getResearchRadius((BASE_STATS *)psResGraphic);
        if(Radius <= 100)
        {
            scale = RESEARCH_COMPONENT_SCALE / 2;
        }
        else if(Radius <= 128)
        {
            scale = RESEARCH_COMPONENT_SCALE / 3;
        }
        else if(Radius <= 256)
        {
            scale = RESEARCH_COMPONENT_SCALE / 4;
        }
        else
        {
            scale = RESEARCH_COMPONENT_SCALE / 5;
        }
    }


    /* display the IMDs */
    if(IMDType == IMDTYPE_COMPONENT)
    {
        displayComponentButton(psResGraphic,&Rotation,&Position,true, scale);
    }
    else if(IMDType == IMDTYPE_RESEARCH)
    {
        displayResearchButton(psResGraphic,&Rotation,&Position,true, scale);
    }
    else if(IMDType == IMDTYPE_STRUCTURESTAT)
    {
        displayStructureStatButton((STRUCTURE_STATS *)psResGraphic, &Rotation, &Position, true, scale);
    }
    else
    {
        ASSERT( false, "renderResearchToBuffer: Unknown PIEType" );
    }

    // close matrix context
    pie_MatEnd();
}
