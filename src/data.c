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
 * @file data.c
 *
 * Data loading functions used by the framework resource module.
 *
 */

#include <SDL.h>

#include "lib/framework/frame.h"
#include "lib/framework/frameresource.h"
#include "lib/framework/strres.h"
#include "lib/gamelib/parser.h"
#include "lib/ivis_common/bitimage.h"
#include "lib/ivis_common/tex.h"
#include "lib/script/script.h"
#include "lib/sound/audio.h"

#include "data.h"
#include "droid.h"
#include "feature.h"
#include "function.h"
#include "mechanics.h"
#include "message.h"
#include "multiplay.h"
#include "research.h"
#include "scriptvals.h"
#include "stats.h"
#include "text.h"
#include "texture.h"

// whether a save game is currently being loaded
static bool saveFlag = false;

uint32_t	DataHash[DATA_MAXDATA] = {0};

/**
*	hashBuffer()
*	\param pData pointer to our buffer (always a text file)
*	\param size the size of the buffer
*	\return hash calculated from the buffer.
*
*	Note, this is obviously not a very complex hash routine.  This most likely has many collisions possible.
*	This is almost the same routine that Pumpkin had, minus the ugly bug :)
*/
static uint32_t	hashBuffer(uint8_t *pData, uint32_t size)
{
	uint32_t hashval = 0, *val;
	uint32_t pt = 0, newsize, i;
	int fillbytes = 0, CRtoStrip = 0;
	uint8_t *NewData = NULL;
	bool isWindowsFormat = false;

	// find out how many CRs are in the buffer
	for (i = 0; i < size; i++)
	{
		if (pData[i] == '\r')
		{
			CRtoStrip++;
		}
	}

	// Calculate the new size of the buffer we need to create, on a 4 byte boundry
	fillbytes = (size - CRtoStrip) % 4;
	if (fillbytes == 0)
	{
		newsize = size - CRtoStrip;	//don't need to do anything
	}
	else
	{
		newsize = (size - CRtoStrip) + (4 - fillbytes);
		fillbytes = newsize % 4;
	}

	NewData = malloc(newsize * sizeof(uint8_t));
	if (!NewData)
	{
		debug(LOG_FATAL, "Out of memory!");
		abort();
	}
	memset(NewData, 0xff, newsize);		// fill the new buffer with bit pattern 0xff for easier debug purposes

	// convert CRLF (windows) to LF
	for(i = 0; i < size - CRtoStrip; i++, pData++)
	{
		if (*pData == '\r' && *(pData + 1) == '\n')
		{
			NewData[i] = *(++pData);			// for windows change CRLF to LF
			isWindowsFormat = true;
		}
		else
		{
			NewData[i] = *(pData);				// straight copy
		}
	}

	debug(LOG_NEVER, "NewData is {%.10s}\n size is %u bytes \n", NewData, newsize );	// this is a bit spammy...

	ASSERT_OR_RETURN(0, newsize != 0, "Empty file being checked!");
	while (pt < newsize)
	{
		val = (uint32_t *)(NewData + pt);

		hashval ^= (*val);

		// spams a ton--but useful for debugging.
		//	debug(LOG_NET, "hash %08x pt %08x val is %08x", hashval, pt, *val);
		pt += 4;
	}

	free(NewData);

	debug(LOG_NET, "The size of the old buffer (%u bytes - %d stripped ), New buffer size of %u bytes. Format is: %s", size, CRtoStrip, newsize,  isWindowsFormat ? "windows" : "linux/mac");
	return hashval;
}

// create the hash for that data block.
// Data should be converted to Network byte order
static void calcDataHash(uint8_t *pBuffer, uint32_t size, uint32_t index)
{
	const uint32_t oldHash = DataHash[index];

	if (!bMultiPlayer)
	{
		return;
	}

	DataHash[index] ^= SDL_SwapBE32(hashBuffer(pBuffer, size));

	if (!DataHash[index] && oldHash)
	{
		debug(LOG_NET, "The new hash is 0, the old hash was %u. We XOR'ed the same value!", oldHash);
	}

	debug(LOG_NET, "DataHash[%2u] = %08x", index, SDL_SwapBE32(DataHash[index]));

	return;
}

void resetDataHash(void)
{
	uint32_t i;
	for (i = 0; i < DATA_MAXDATA; i++)
	{
		DataHash[i] = 0;
	}
	debug(LOG_NET, "== Hash is reset ==");
}

/**********************************************************/


void dataSetSaveFlag(void)
{
	saveFlag = true;
}
void dataClearSaveFlag(void)
{
	saveFlag = false;
}

/* Load the body stats */
static bool bufferSBODYLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SBODY);

	if (!loadBodyStats(pBuffer, size)
			|| !allocComponentList(COMP_BODY, numBodyStats))
	{
		return false;
	}

	// set a dummy value so the release function gets called
	*ppData = (void *)1;
	return true;
}

static void dataReleaseStats(WZ_DECL_UNUSED void *pData)
{
	freeComponentLists();
	statsShutDown();
}


/* Load the weapon stats */
static bool bufferSWEAPONLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SWEAPON);

	if (!loadWeaponStats(pBuffer, size)
			|| !allocComponentList(COMP_WEAPON, numWeaponStats))
	{
		return false;
	}

	// not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the constructor stats */
static bool bufferSCONSTRLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SCONSTR);

	if (!loadConstructStats(pBuffer, size)
			|| !allocComponentList(COMP_CONSTRUCT, numConstructStats))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the ECM stats */
static bool bufferSECMLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SECM);

	if (!loadECMStats(pBuffer, size)
			|| !allocComponentList(COMP_ECM, numECMStats))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Propulsion stats */
static bool bufferSPROPLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SPROP);

	if (!loadPropulsionStats(pBuffer, size)
			|| !allocComponentList(COMP_PROPULSION, numPropulsionStats))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Sensor stats */
static bool bufferSSENSORLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SSENSOR);

	if (!loadSensorStats(pBuffer, size)
			|| !allocComponentList(COMP_SENSOR, numSensorStats))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Repair stats */
static bool bufferSREPAIRLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SREPAIR);

	if (!loadRepairStats(pBuffer, size)
			|| !allocComponentList(COMP_REPAIRUNIT, numRepairStats))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Brain stats */
static bool bufferSBRAINLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SBRAIN);

	if (!loadBrainStats(pBuffer, size)
			|| !allocComponentList(COMP_BRAIN, numBrainStats))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the PropulsionType stats */
static bool bufferSPROPTYPESLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SPROPTY);

	if (!loadPropulsionTypes(pBuffer, size))
	{
		return false;
	}


	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the propulsion type sound stats */
static bool bufferSPROPSNDLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	if (!loadPropulsionSounds(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the SSPECABIL stats */
static bool bufferSSPECABILLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	if (!loadSpecialAbility(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the STERRTABLE stats */
static bool bufferSTERRTABLELoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_STERRT);

	if (!loadTerrainTable(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the body/propulsion IMDs stats */
static bool bufferSBPIMDLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	if (!loadBodyPropulsionIMDs(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the weapon sound stats */
static bool bufferSWEAPSNDLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	if (!loadWeaponSounds(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Weapon Effect modifier stats */
static bool bufferSWEAPMODLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SWEAPMOD);

	if (!loadWeaponModifiers(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}


/* Load the Template stats */
static bool bufferSTEMPLLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_STEMP);

	if (!loadDroidTemplates(pBuffer, size))
	{
		return false;
	}

	// set a dummy value so the release function gets called
	*ppData = (void *)1;
	return true;
}

// release the templates
static void dataSTEMPLRelease(WZ_DECL_UNUSED void *pData)
{
	//free the storage allocated to the droid templates
	droidTemplateShutDown();
}

/* Load the Template weapons stats */
static bool bufferSTEMPWEAPLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_STEMPWEAP);

	if (!loadDroidWeapons(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Structure stats */
static bool bufferSSTRUCTLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SSTRUCT);

	if (!loadStructureStats(pBuffer, size)
			|| !allocStructLists())
	{
		return false;
	}

	// set a dummy value so the release function gets called
	*ppData = (void *)1;
	return true;
}

// release the structure stats
static void dataSSTRUCTRelease(WZ_DECL_UNUSED void *pData)
{
	freeStructureLists();
	structureStatsShutDown();
}

/* Load the Structure Weapons stats */
static bool bufferSSTRWEAPLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SSTRWEAP);

	if (!loadStructureWeapons(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Structure Functions stats */
static bool bufferSSTRFUNCLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_STRFUNC);

	if (!loadStructureFunctions(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Structure strength modifier stats */
static bool bufferSSTRMODLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SSTRMOD);

	if (!loadStructureStrengthModifiers(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the Feature stats */
static bool bufferSFEATLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SFEAT);

	if (!loadFeatureStats(pBuffer, size))
	{
		return false;
	}

	// set a dummy value so the release function gets called
	*ppData = (void *)1;
	return true;
}

// free the feature stats
static void dataSFEATRelease(WZ_DECL_UNUSED void *pData)
{
	featureStatsShutDown();
}

/* Load the Functions stats */
static bool bufferSFUNCLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_SFUNC);

	if (!loadFunctionStats(pBuffer, size))
	{
		return false;
	}

	//adjust max values of stats used in the design screen due to any possible upgrades
	adjustMaxDesignStats();

	// set a dummy value so the release function gets called
	*ppData = (void *)1;
	return true;
}

// release the function stats
static void dataSFUNCRelease(WZ_DECL_UNUSED void *pData)
{
	FunctionShutDown();
}

// release the research stats
static void dataRESCHRelease(WZ_DECL_UNUSED void *pData)
{
	//free the storage allocated to the stats
	ResearchShutDown();
}

/* Load the Research stats */
static bool bufferRESCHLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RESCH);

	//check to see if already loaded
	if (numResearch > 0)
	{
		//release previous data before loading in the new
		dataRESCHRelease(NULL);
	}

	if (!loadResearch(pBuffer, size))
	{
		return false;
	}


	/* set a dummy value so the release function gets called - the Release
	 * function is now called when load up the next set
	// *ppData = (void *)1;
	 * pass back NULL so that can load the same name file for the next campaign*/
	*ppData = NULL;
	return true;
}

/* Load the research pre-requisites */
static bool bufferRPREREQLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RPREREQ);

	if (!loadResearchPR(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the research components made redundant */
static bool bufferRCOMPREDLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RCOMPRED);

	if (!loadResearchArtefacts(pBuffer, size, RED_LIST))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the research component results */
static bool bufferRCOMPRESLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RCOMPRES);

	if (!loadResearchArtefacts(pBuffer, size, RES_LIST))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the research structures required */
static bool bufferRSTRREQLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RSTRREQ);

	if (!loadResearchStructures(pBuffer, size, REQ_LIST))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the research structures made redundant */
static bool bufferRSTRREDLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RSTRRED);

	if (!loadResearchStructures(pBuffer, size, RED_LIST))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the research structure results */
static bool bufferRSTRRESLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RSTRRES);

	if (!loadResearchStructures(pBuffer, size, RES_LIST))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the research functions */
static bool bufferRFUNCLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	calcDataHash((uint8_t *)pBuffer, size, DATA_RFUNC);

	if (!loadResearchFunctions(pBuffer, size))
	{
		return false;
	}

	//not interested in this value
	*ppData = NULL;
	return true;
}

/* Load the message viewdata */
static bool bufferSMSGLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	VIEWDATA	*pViewData;

	pViewData = loadViewData(pBuffer, size);
	if (!pViewData)
	{
		return false;
	}

	// set the pointer so the release function gets called with it
	*ppData = (void *)pViewData;
	return true;
}

/* Load research message viewdata */
static bool dataResearchMsgLoad(const char *fileName, void **ppData)
{
	VIEWDATA *pViewData = loadResearchViewData(fileName);
	if (!pViewData)
	{
		return false;
	}

	// set the pointer so the release function gets called with it
	*ppData = pViewData;
	return true;
}

// release the message viewdata
static void dataSMSGRelease(void *pData)
{
	viewDataShutDown((VIEWDATA *)pData);
}

/* Load an imd */
static bool dataIMDBufferLoad(const char *pBuffer, uint32_t size, void **ppData)
{
	iIMDShape	*psIMD;
	const char *pBufferPosition = pBuffer;

	psIMD = iV_ProcessIMD( &pBufferPosition, pBufferPosition + size );
	if (psIMD == NULL)
	{
		debug( LOG_ERROR, "IMD load failed - %s", GetLastResourceFilename() );
		return false;
	}

	*ppData = psIMD;
	return true;
}


/*!
 * Load an image from file
 */
static bool dataImageLoad(const char *fileName, void **ppData)
{
	iV_Image *psSprite = malloc(sizeof(iV_Image));
	if (!psSprite)
	{
		return false;
	}

	if (!iV_loadImage_PNG(fileName, psSprite))
	{
		debug( LOG_ERROR, "IMGPAGE load failed" );
		return false;
	}

	*ppData = psSprite;

	return true;
}


// Tertiles (terrain tiles) loader.
static bool dataTERTILESLoad(const char *fileName, void **ppData)
{
	bool status;

	status = texLoad(fileName);
	ASSERT_OR_RETURN(false, status, "Error loading tertiles!");
	debug(LOG_TEXTURE, "HW Tiles loaded");

	*ppData = NULL;	// don't bother calling cleanup

	return true;
}

static void dataTERTILESRelease(WZ_DECL_UNUSED void *pData)
{
}


static bool dataIMGLoad(const char *fileName, void **ppData)
{
	*ppData = iV_LoadImageFile(fileName);
	if(*ppData == NULL)
	{
		return false;
	}

	return true;
}


static void dataIMGRelease(void *pData)
{
	iV_FreeImageFile((IMAGEFILE *)pData);
}


/* Load a texturepage into memory */
static bool dataTexPageLoad(const char *fileName, void **ppData)
{
	char texpage[PATH_MAX] = {'\0'};

	// This hackery is needed, because fileName will include the directory name, whilst the LastResourceFilename will not, and we need a short name to identify the texpage
	sstrcpy(texpage, GetLastResourceFilename());

	pie_MakeTexPageName(texpage);
	if (!dataImageLoad(fileName, ppData))
	{
		return false;
	}

	// see if this texture page has already been loaded
	if (resPresent("TEXPAGE", texpage))
	{
		// replace the old texture page with the new one
		debug(LOG_TEXTURE, "replacing %s with new texture %s", texpage, fileName);
		(void) pie_ReplaceTexPage(*ppData, texpage, getTextureSize());
	}
	else
	{
		debug(LOG_TEXTURE, "adding page %s with texture %s", texpage, fileName);
		SetLastResourceFilename(texpage);
		(void) pie_AddTexPage(*ppData, texpage, 0, getTextureSize());
	}

	return true;
}

/*!
 * Release an Image
 */
static void dataImageRelease(void *pData)
{
	iV_Image *psSprite = (iV_Image *) pData;

	if( psSprite )
	{
		free(psSprite);
	}
}


/* Load an audio file */
static bool dataAudioLoad(const char *fileName, void **ppData)
{
	if ( audio_Disabled() == true )
	{
		*ppData = NULL;
		// No error occurred (sound is just disabled), so we return true
		return true;
	}

	// Load the track from a file
	*ppData = sound_LoadTrackFromFile( fileName );

	return *ppData != NULL;
}

/* Load an audio file */
static bool dataAudioCfgLoad(const char *fileName, void **ppData)
{
	bool success;
	PHYSFS_file *fileHandle;

	*ppData = NULL;

	if (audio_Disabled())
	{
		return true;
	}
	debug(LOG_WZ, "Reading...[directory: %s] %s", PHYSFS_getRealDir(fileName), fileName);
	fileHandle = PHYSFS_openRead(fileName);

	if (fileHandle == NULL)
	{
		return false;
	}

	success = ParseResourceFile(fileHandle);

	PHYSFS_close(fileHandle);

	return success;
}

/* Load an anim file */
static bool dataAnimLoad(const char *fileName, void **ppData)
{
	PHYSFS_file *fileHandle = PHYSFS_openRead(fileName);
	debug(LOG_WZ, "Reading...[directory: %s] %s", PHYSFS_getRealDir(fileName), fileName);
	if (fileHandle == NULL)
	{
		*ppData = NULL;
		return false;
	}

	*ppData = anim_LoadFromFile(fileHandle);

	PHYSFS_close(fileHandle);

	return *ppData != NULL;
}

/* Load an audio config file */
static bool dataAnimCfgLoad(const char *fileName, void **ppData)
{
	bool success;
	PHYSFS_file *fileHandle = PHYSFS_openRead(fileName);
	*ppData = NULL;

	debug(LOG_WZ, "Reading...[directory: %s] %s", PHYSFS_getRealDir(fileName), fileName);
	if (fileHandle == NULL)
	{
		return false;
	}

	success = ParseResourceFile(fileHandle);

	PHYSFS_close(fileHandle);

	return success;
}


static void dataAnimRelease( void *pData )
{
	anim_ReleaseAnim((BASEANIM *)pData);
}

/* Load a string resource file */
static bool dataStrResLoad(const char *fileName, void **ppData)
{
	// recreate the string resource if it was freed by a WRF release
	if (psStringRes == NULL)
	{
		if (!stringsInitialise())
		{
			return false;
		}
	}

	if (!strresLoad(psStringRes, fileName))
	{
		return false;
	}

	*ppData = psStringRes;
	return true;
}

static void dataStrResRelease(WZ_DECL_UNUSED void *pData)
{
	if (psStringRes != NULL)
	{
		strresDestroy(psStringRes);
		psStringRes = NULL;
	}
}


/* Load a script file */
// All scripts, binary or otherwise are now passed through this routine
static bool dataScriptLoad(const char *fileName, void **ppData)
{
	static const bool printHack = false;
	SCRIPT_CODE **psProg = (SCRIPT_CODE **)ppData;
	PHYSFS_file *fileHandle;
	uint8_t *pBuffer;
	PHYSFS_sint64 fileSize = 0;

	debug(LOG_WZ, "COMPILING SCRIPT ...%s", GetLastResourceFilename());

	fileHandle = PHYSFS_openRead(fileName);
	debug(LOG_WZ, "Reading...[directory: %s] %s", PHYSFS_getRealDir(fileName), fileName);
	if (fileHandle == NULL)
	{
		return false;
	}
	// due to the changes in r2531 we must do this routine a bit different.
	fileSize = PHYSFS_fileLength(fileHandle);

	pBuffer = malloc(fileSize * sizeof(char));
	if (pBuffer == NULL)
	{
		debug(LOG_FATAL, "Fatal memory allocation, couldn't allocate %ld buffer", (long)fileSize);
		abort();
	}

	PHYSFS_read(fileHandle, pBuffer, 1, fileSize);

	calcDataHash(pBuffer, fileSize, DATA_SCRIPT);

	free(pBuffer);
	PHYSFS_seek(fileHandle, 0);		//reset position

	*psProg = scriptCompile(fileHandle, SCRIPTTYPE);

	PHYSFS_close(fileHandle);

	if (!*psProg)		// see script.h
	{
		debug(LOG_ERROR, "Script %s did not compile", GetLastResourceFilename());
		return false;
	}

	if (printHack)
	{
		cpPrintProgram(*psProg);
	}

	return true;
}


static void dataScriptRelease(void *pData)
{
	SCRIPT_CODE *psCode = pData;
	scriptFreeCode(psCode);
}


// Load a script variable values file
static bool dataScriptLoadVals(const char *fileName, void **ppData)
{
	bool success;
	PHYSFS_file *fileHandle;
	uint8_t *pBuffer;
	PHYSFS_sint64 fileSize = 0;

	*ppData = NULL;

	// don't load anything if a saved game is being loaded
	if (saveFlag)
	{
		return true;
	}

	debug(LOG_WZ, "Loading script data %s", GetLastResourceFilename());

	fileHandle = PHYSFS_openRead(fileName);
	debug(LOG_WZ, "Reading...[directory: %s] %s", PHYSFS_getRealDir(fileName), fileName);
	if (fileHandle == NULL)
	{
		return false;
	}
	// due to the changes in r2532 we must do this routine a bit different.
	fileSize = PHYSFS_fileLength(fileHandle);

	pBuffer = malloc(fileSize * sizeof(char));
	if (pBuffer == NULL)
	{
		debug(LOG_FATAL, "Fatal memory allocation, couldn't allocate %ld buffer", (long)fileSize);
		abort();
	}

	PHYSFS_read(fileHandle, pBuffer, 1, fileSize);

	calcDataHash(pBuffer, fileSize, DATA_SCRIPTVAL);

	free(pBuffer);

	PHYSFS_seek(fileHandle, 0);		//reset position

	success = scrvLoad(fileHandle);

	if (!success)
	{
		debug(LOG_FATAL, "Script %s did not compile", GetLastResourceFilename());
	}

	PHYSFS_close(fileHandle);

	return success;
}

// New reduced resource type ... specially for PSX
// These are statically defined in data.c
// this is also defined in frameresource.c - needs moving to a .h file
// This basically matches the argument list of resAddBufferLoad in frameresource.c
typedef struct
{
	const char *aType;                      ///< points to the string defining the type (e.g. SCRIPT) - NULL indicates end of list
	RES_BUFFERLOAD buffLoad;                ///< routine to process the data for this type
	RES_FREE release;                       ///< routine to release the data (NULL indicates none)
} RES_TYPE_MIN_BUF;

static const RES_TYPE_MIN_BUF BufferResourceTypes[] =
{
	{"SWEAPON", bufferSWEAPONLoad, NULL},
	{"SBODY", bufferSBODYLoad, dataReleaseStats},
	{"SBRAIN", bufferSBRAINLoad, NULL},
	{"SPROP", bufferSPROPLoad, NULL},
	{"SSENSOR", bufferSSENSORLoad, NULL},
	{"SECM", bufferSECMLoad, NULL},
	{"SREPAIR", bufferSREPAIRLoad, NULL},
	{"SCONSTR", bufferSCONSTRLoad, NULL},
	{"SPROPTYPES", bufferSPROPTYPESLoad, NULL},
	{"SPROPSND", bufferSPROPSNDLoad, NULL},
	{"STERRTABLE", bufferSTERRTABLELoad, NULL},
	{"SSPECABIL", bufferSSPECABILLoad, NULL},
	{"SBPIMD", bufferSBPIMDLoad, NULL},
	{"SWEAPSND", bufferSWEAPSNDLoad, NULL},
	{"SWEAPMOD", bufferSWEAPMODLoad, NULL},
	{"STEMPL", bufferSTEMPLLoad, dataSTEMPLRelease},               //template and associated files
	{"STEMPWEAP", bufferSTEMPWEAPLoad, NULL},
	{"SSTRUCT", bufferSSTRUCTLoad, dataSSTRUCTRelease},            //structure stats and associated files
	{"SSTRFUNC", bufferSSTRFUNCLoad, NULL},
	{"SSTRWEAP", bufferSSTRWEAPLoad, NULL},
	{"SSTRMOD", bufferSSTRMODLoad, NULL},
	{"SFEAT", bufferSFEATLoad, dataSFEATRelease},                  //feature stats file
	{"SFUNC", bufferSFUNCLoad, dataSFUNCRelease},                  //function stats file
	{"RESCH", bufferRESCHLoad, dataRESCHRelease},                  //research stats files
	{"RPREREQ", bufferRPREREQLoad, NULL},
	{"RCOMPRED", bufferRCOMPREDLoad, NULL},
	{"RCOMPRES", bufferRCOMPRESLoad, NULL},
	{"RSTRREQ", bufferRSTRREQLoad, NULL},
	{"RSTRRED", bufferRSTRREDLoad, NULL},
	{"RSTRRES", bufferRSTRRESLoad, NULL},
	{"RFUNC", bufferRFUNCLoad, NULL},
	{"SMSG", bufferSMSGLoad, dataSMSGRelease},
	{"IMD", dataIMDBufferLoad, (RES_FREE)iV_IMDRelease},
};

typedef struct
{
	const char *aType;                      ///< points to the string defining the type (e.g. SCRIPT) - NULL indicates end of list
	RES_FILELOAD fileLoad;                  ///< routine to process the data for this type
	RES_FREE release;                       ///< routine to release the data (NULL indicates none)
} RES_TYPE_MIN_FILE;

static const RES_TYPE_MIN_FILE FileResourceTypes[] =
{
	{"WAV", dataAudioLoad, (RES_FREE)sound_ReleaseTrack},
	{"AUDIOCFG", dataAudioCfgLoad, NULL},
	{"ANI", dataAnimLoad, dataAnimRelease},
	{"ANIMCFG", dataAnimCfgLoad, NULL},
	{"IMGPAGE", dataImageLoad, dataImageRelease},
	{"TERTILES", dataTERTILESLoad, dataTERTILESRelease},
	{"IMG", dataIMGLoad, dataIMGRelease},
	{"TEXPAGE", dataTexPageLoad, dataImageRelease},
	{"SCRIPT", dataScriptLoad, dataScriptRelease},
	{"SCRIPTVAL", dataScriptLoadVals, NULL},
	{"STR_RES", dataStrResLoad, dataStrResRelease},
	{ "RESEARCHMSG", dataResearchMsgLoad, dataSMSGRelease },
};

/* Pass all the data loading functions to the framework library */
bool dataInitLoadFuncs(void)
{
	// init the data integrity hash;
	resetDataHash();
	// Using iterator style: begin iterator (ResourceTypes),
	// end iterator (EndType), and current iterator (CurrentType)

	// iterate through buffer load functions
	{
		const RES_TYPE_MIN_BUF *CurrentType;
		// Points just past the last item in the list
		const RES_TYPE_MIN_BUF *const EndType = &BufferResourceTypes[sizeof(BufferResourceTypes) / sizeof(RES_TYPE_MIN_BUF)];

		for (CurrentType = BufferResourceTypes; CurrentType != EndType; ++CurrentType)
		{
			if(!resAddBufferLoad(CurrentType->aType, CurrentType->buffLoad, CurrentType->release))
			{
				return false; // error whilst adding a buffer load
			}
		}
	}

	// iterate through file load functions
	{
		const RES_TYPE_MIN_FILE *CurrentType;
		// Points just past the last item in the list
		const RES_TYPE_MIN_FILE *const EndType = &FileResourceTypes[sizeof(FileResourceTypes) / sizeof(RES_TYPE_MIN_BUF)];

		for (CurrentType = FileResourceTypes; CurrentType != EndType; ++CurrentType)
		{
			if(!resAddFileLoad(CurrentType->aType, CurrentType->fileLoad, CurrentType->release))
			{
				return false; // error whilst adding a file load
			}
		}
	}

	return true;
}
