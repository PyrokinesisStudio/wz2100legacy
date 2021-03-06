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
/**
 * @file
 *
 * Handles fixed point operations for degrees and matrix math.
 * @todo Get rid of this file and use floats instead.
 */

#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include "wzglobal.h"


//! PSX-style float emulation: 12 digit semi-floats stored in an int
// FIXME!
#define FP12_SHIFT 12
#define FP12_MULTIPLIER (1 << FP12_SHIFT)


/*
 *	Global Definitions (CONSTANTS)
 */
static const int DEG_360 = 65536;
static const float DEG_1 = (float)65536 / 360.f;


static inline WZ_DECL_CONST int DEG(const float degree)
{
	return (int)(degree * DEG_1);
}

#endif // FIXEDPOINT_H
