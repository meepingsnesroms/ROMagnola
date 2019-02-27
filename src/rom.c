#include "os_structs.h"
#include "rom.h"

/**************************************************
 * Include the PRC/database ordering information
 */
#include "SystemResources.h"
#include "350.rom.h"
#include "330.rom.h"
#include "325.rom.h"
#include "310.rom.h"
#include "300.rom.h"
#include "200.rom.h"
#include "100.rom.h"
/**************************************************/

/*
 * Defaults for different version of the PalmOS
 */
ROMVersion ROMVers[romNumROMVers] = 
{
	{0x0350,								// PalmOS version
	 0x10c00000,							// ROM Base
	 0x00008000,							// Small ROM Size (default)
	 0x00004000,							// Small ROM card size (default)
	 small_PRC_ROMS_350_rom,
	 "D,dr+",								// Layout of PRCs in small ROM
	 "D,dr+",								// Layout of PRCs added to small ROM
	 0x00200000 - 0x8000,					// Big ROM Size (default)
	 0x00188000 - 0x8000,					// Big ROM card size (default)
	 large_PRC_ROMS_350_rom,
	 "D,dr+",								// Layout of PRCs in large ROM
	 "D,dr+",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0,										// Card Init Stack
	 0x04,									// Card Header Version
	 memCardHeaderFlagEZ,					// Card Flags
	 0x01,									// Card Version
	 0x06000,								// RO Params offset
	 0x08000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers4 | memHeapFlagReadOnly	// Heap Flags
	},
	{0x0330,								// PalmOS version
	 0x10c00000,							// ROM Base
	 0x00008000,							// Small ROM Size (default)
	 0x00004000,							// Small ROM card size (default)
	 small_PRC_ROMS_330_rom,
	 "r-,1532,d+,D",						// Layout of PRCs in small ROM
	 "r-,d+,D",								// Layout of PRCs added to small ROM
	 0x00200000 - 0x8000,					// Big ROM Size (default)
	 0x00188000 - 0x8000,					// Big ROM card size (default)
	 large_PRC_ROMS_330_rom,
	 "r-,15106,d+,D",						// Layout of PRCs in large ROM
	 "r-,d+,D",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0x3000,								// Card Init Stack
	 0x03,									// Card Header Version
	 memCardHeaderFlagEZ,					// Card Flags
	 0x01,									// Card Version
	 0x06000,								// RO Params offset
	 0x08000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers3 | memHeapFlagReadOnly	// Heap Flags
	},
	/* This 3.25 version is for Palm VII - same for others?? */
	{0x0325,								// PalmOS version
	 0x10c00000,							// ROM Base
	 0x00010000,							// Small ROM Size (default)
	 0x00004000,							// Small ROM card size (default)
	 small_PRC_ROMS_325_rom,
	 "r-,2664,d+,D",						// Layout of PRCs in small ROM
	 "r-,d+,D",								// Layout of PRCs added to small ROM
	 0x00200000 - 0x10000,					// Big ROM Size (default)
	 0x00188000 - 0x10000,					// Big ROM card size (default)
	 large_PRC_ROMS_325_rom,
	 "r-,69440,d+,D",						// Layout of PRCs in large ROM
	 "r-,d+,D",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0x3000,								// Card Init Stack
	 0x03,									// Card Header Version
	 memCardHeaderFlagEZ,					// Card Flags
	 0x01,									// Card Version
	 0x06000,								// RO Params offset
	 0x10000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers3 | memHeapFlagReadOnly	// Heap Flags
	},
	{0x0310,								// PalmOS version
	 0x10c00000,							// ROM Base
	 0x00008000,							// Small ROM Size (default)
	 0x00004000,							// Small ROM card size (default)
	 small_PRC_ROMS_310_rom,
	 "r-,d+,D",								// Layout of PRCs in small ROM
	 "r-,d+,D",								// Layout of PRCs added to small ROM
	 0x00200000 - 0x8000,					// Big ROM Size (default)
	 0x00130000 - 0x8000,					// Observed from 310.rom
	 large_PRC_ROMS_310_rom,
	 "r-,49866,d+,D",						// Layout of PRCs in large ROM
	 "r-,d+,D",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0x3000,								// Card Init Stack
	 0x03,									// Card Header Version
	 memCardHeaderFlagEZ,					// Card Flags
	 0x01,									// Card Version
	 0x06000,								// RO Params offset
	 0x08000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers3 | memHeapFlagReadOnly	// Heap Flags
	},
	{0x0300,								// PalmOS version
	 0x00c00000,							// ROM Base
	 0x00008000,							// Small ROM Size (default)
	 0x00004000,							// Small ROM card size (default)
	 small_PRC_ROMS_300_rom,
	 "r+,d-,D",								// Layout of PRCs in small ROM
	 "r+,d-,D",								// Layout of PRCs added to small ROM
	 0x00200000 - 0x8000,					// Big ROM Size (default)
	 0x00188000 - 0x8000,					// Big ROM card size (default)
	 large_PRC_ROMS_300_rom,
	 "r+,40988,d-,D",						// Layout of PRCs in large ROM
	 "r+,d-,D",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0x3000,								// Card Init Stack
	 0x02,									// Card Header Version
	 0,										// Card Flags
	 0x01,									// Card Version
	 0x06000,								// RO Params offset
	 0x08000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers2 | memHeapFlagReadOnly	// Heap Flags
	},
	{0x0200,								// PalmOS version
	 0x00c00000,							// ROM Base
	 0x00003000,							// Small ROM Size (default)
	 0x00003000,							// Small ROM card size (default)
	 small_PRC_ROMS_200_rom,
	 "r+,472,d+,D",							// Layout of PRCs in small ROM
	 "r+,d+,D",								// Layout of PRCs added to small ROM
	 0x00100000 - 0x3000,					// Big ROM Size (default)
	 0x000EE000 - 0x3000,					// Big ROM card size (default)
	 large_PRC_ROMS_200_rom,
	 "r+,d+,D",								// Layout of PRCs in large ROM
	 "r+,d+,D",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0x3000,								// Card Init Stack
	 0x01,									// Card Header Version
	 0,										// Card Flags
	 0x01,									// Card Version
	 0x00000,								// RO Params offset
	 0x03000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers1 | memHeapFlagReadOnly	// Heap Flags
	},
	{0x0100,								// PalmOS version
	 0x00c00000,							// ROM Base
	 0x00003000,							// Small ROM Size (default)
	 0x00003000,							// Small ROM card size (default)
	 small_PRC_ROMS_100_rom,
	 "D,dr+",								// Layout of PRCs in small ROM
	 "D,dr+",								// Layout of PRCs added to small ROM
	 0x00080000 - 0x3000,					// Big ROM Size (default)
	 0x00080000 - 0x3000,					// Big ROM card size (default)
	 large_PRC_ROMS_100_rom,
	 "D,dr+",								// Layout of PRCs in large ROM
	 "D,dr+",								// Layout of PRCs added to big ROM
	 DEF_CARD_NAME,							// Card Name
	 DEF_CARD_MANU,							// Card Manufacturer Name
	 0x3000,								// Card Init Stack
	 0x01,									// Card Header Version
	 0,										// Card Flags
	 0x01,									// Card Version
	 0x00000,								// RO Params offset
	 0x03000,								// Big ROM offset
	 DEF_STORE_NAME,						// Store Name
	 0x01,									// Store Version
	 0x0,									// Store Flags
	 memHeapFlagVers1 | memHeapFlagReadOnly	// Heap Flags
	}
};

ROMVersion* LocateROMVerByOSVer (UInt16	osver)
{
	int numROMVers = sizeof (ROMVers) / sizeof (ROMVers[0]);
	int idex;
	for (idex = 0; idex < numROMVers; idex++)
	{
		if (ROMVers[idex].palmOSVer == osver)
			return &ROMVers[idex];
	}
	return NULL;
}

ROMVersion*		GuessVersion (ROMPtr pROM)
{
	UInt32 idex = 0;

	while (idex < romNumROMVers && 
		   ROMVers[idex].card_hdrVersion > pROM->pCard->hdrVersion)
		idex++;

	while (idex < romNumROMVers &&
		   memUHeapVerFromFlags (ROMVers[idex].heap_flags) > 
		   memUHeapVer (pROM->pHeapList->heapOffset[0]))
		idex++;
		  
	if (ROMVers[idex].card_hdrVersion != pROM->pCard->hdrVersion ||
		memUHeapVerFromFlags (ROMVers[idex].heap_flags) !=
			memUHeapVer (pROM->pHeapList->heapOffset[0])		 ||
		idex == romNumROMVers)
		return NULL;

	return &ROMVers[idex];
}
