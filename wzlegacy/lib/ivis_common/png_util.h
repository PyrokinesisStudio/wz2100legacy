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

#ifndef _LIBIVIS_COMMON_PNG_H_
#define _LIBIVIS_COMMON_PNG_H_

#include "pietypes.h"

/*!
 * Load a PNG from file into image
 *
 * \param fileName input file to load from
 * \param image Sprite to read into
 * \return true on success, false otherwise
 */
BOOL iV_loadImage_PNG(const char *fileName, iV_Image *image);

/*!
 * Save a PNG from image into file
 *
 * \param fileName output file to save to
 * \param image Texture to read from
 * \return true on success, false otherwise
 */
void iV_saveImage_PNG(const char *fileName, const iV_Image *image);

void iV_saveImage_JPEG(const char *fileName, const iV_Image *image);
#endif // _LIBIVIS_COMMON_PNG_H_
