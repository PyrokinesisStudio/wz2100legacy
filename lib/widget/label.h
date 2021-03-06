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
 *  Definitions for the label widget.
 */

#ifndef __INCLUDED_LIB_WIDGET_LABEL_H__
#define __INCLUDED_LIB_WIDGET_LABEL_H__

#include "widget.h"
#include "widgbase.h"
#include "lib/ivis_common/textdraw.h"

// label states.
#define WLABEL_HILITE	0x0004		// label is hilited

typedef struct _w_label
{
	/* The common widget data */
	WIDGET_BASE;

	uint32_t		state;					// The current button state
	char		aText[WIDG_MAXSTR];		// Text on the label
	enum iV_fonts FontID;
	const char	*pTip;					// The tool tip for the button
} W_LABEL;

/* Create a button widget data structure */
extern W_LABEL *labelCreate(const W_LABINIT *psInit);

/* Free the memory used by a button */
extern void labelFree(W_LABEL *psWidget);

/* label display function */
extern void labelDisplay(WIDGET *psWidget, uint32_t xOffset, uint32_t yOffset, PIELIGHT *pColours);

/* Respond to a mouse moving over a label */
extern void labelHiLite(W_LABEL *psWidget, W_CONTEXT *psContext);

/* Respond to the mouse moving off a label */
extern void labelHiLiteLost(W_LABEL *psWidget);

#endif // __INCLUDED_LIB_WIDGET_LABEL_H__
