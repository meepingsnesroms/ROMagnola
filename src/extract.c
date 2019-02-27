#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "os_structs.h"
#include "rom.h"
#include "extract.h"
#include "byte_swap.h"
#include "translate.h"
#include "utils.h"

#define	ROM_BASE				0x10C00000
#define PALMOS_1_SIZE			0x00080000
#define PALMOS_1_BIG_ROM_OFFSET	0x00003000
#define PALMOS_4_BIG_ROM_OFFSET	0x00008000

/*
 * Read in a Palm RAM, byte swap and relocate relative to the RAM buffer.
 */
ROMPtr	ReadRAM	(int	hROM,
				 UInt16	flags)
{
	StorageHeaderType	Store;
	UInt32				ROM_base;
	UInt32				Card_base;
	UInt32				File_base;
	ROMPtr				pROM		= NULL;
	UInt32				nBytes;

	/* How big is the RAM image? */
	nBytes = lseek(hROM, 0, SEEK_END);
	if (nBytes < sizeof(Store))
	{
		return(NULL);
	}

	if (lseek(hROM, 0, SEEK_SET) != 0)
		return(NULL);

	if (read(hROM, &Store, sizeof(Store)) != sizeof(Store))
	{
		return(NULL);
	}

	P2H_StorageHeader(&Store);
	if (Store.signature != sysStoreSignature)
	{
		/* NOT a valid Store */
		return(NULL);
	}

	//if (Store.version >= 0x00)
	{
		ROM_base = 0x00040000;
	}

	File_base = 0;
	Card_base = ROM_base;


	// Allocate size for the headers
	if (! (pROM = (ROMPtr)malloc(sizeof(ROMType))))
		return(NULL);
	memset(pROM, 0, sizeof(*pROM));

	pROM->ROM_base  = ROM_base;
	pROM->Card_base = Card_base;
	pROM->File_base = File_base;

	
	/*
	 * Allocate a ROM buffer to hold the entire RAM.
	 */
	pROM->ROMSize  = nBytes - File_base;
	pROM->CardSize = pROM->ROMSize;
	pROM->pROM     = (UInt8*)malloc(pROM->ROMSize);
	if (pROM->pROM == NULL)
	{
		FreeROM(pROM);
		return(NULL);
	}

	/*
	 * Read in the entire ROM
	 */
	lseek(hROM, File_base, SEEK_SET);
	if (read(hROM, pROM->pROM, pROM->ROMSize) != pROM->ROMSize)
	{
		FreeROM(pROM);
		return(0);
	}

	if (! Setup_ROM (pROM))
		return (NULL);

	/*
	 * Make a copy of the Database list that we can sort and sort it
	 */
	if (pROM->pDatabaseList)
	{
		UInt32	nBytes	= sizeof(DatabaseHdrPtr) *
						  pROM->pDatabaseList->numDatabases;

		pROM->pSortedDBList = (DatabaseHdrPtr*)malloc(nBytes);
	
		if (pROM->pSortedDBList == NULL)
		{
			FreeROM(pROM);
			return(NULL);
		}

		memcpy(pROM->pSortedDBList,
		       &(pROM->pDatabaseList->databaseOffset[0]), nBytes);

		qsort((void*)pROM->pSortedDBList,
		       pROM->pDatabaseList->numDatabases,
		       sizeof(DatabaseHdrPtr), CompareAddrs);

	}

	return (pROM);
}

/*
 * Read in a Palm ROM, byte swap and relocate relative to the ROM buffer.
 */
ROMPtr	ReadROM	(int	hROM,
				 UInt16 flags)
{
	CardHeaderType		Card;
	StorageHeaderType	Store;
	UInt32				ROM_base;
	UInt32				Card_base;
	UInt32				File_base;
	ROMPtr				pROM		= NULL;

	if (lseek(hROM, 0, SEEK_SET) != 0)
		return(NULL);

	if (read(hROM, &Card, sizeof(Card)) != sizeof(Card))
	{
		return(NULL);
	}

	P2H_CardHeader   (&Card);
	if (Card.signature != sysCardSignature)
	{
		/* NOT a valid Card */
		return(NULL);
	}

	if (read(hROM, &Store, sizeof(Store)) != sizeof(Store))
	{
		return(NULL);
	}

	P2H_StorageHeader(&Store);
	if (Store.signature != sysStoreSignature)
	{
		/* NOT a valid Store */
		return(NULL);
	}

	if (Card.hdrVersion == 0x01 || Card.hdrVersion == 0x02)
	{
		// The ROM_BASE for version 1 and 2 is 0x00C00000.  This is not right
		// for the reset vector -- that must be handled specially.
		ROM_base = 0x00C00000;
	}
	else
	{
		// The ROM_BASE for version 3 is 0x10C00000 (ROM_BASE)
		ROM_base = ROM_BASE;
	}


	/*
	 * Figure out how to get the ROM we're interested in
	 */
	switch (flags & RT_TYPE_MASK)
	{
	case RT_SMALL:
		if ((Store.initCodeOffset1 != 0) || (Store.initCodeOffset2 != 0))
		{
			/* The first card image is NOT a small ROM */
			return(NULL);
		}
		File_base = 0;
		Card_base = ROM_base;
		break;

	case RT_LARGE:
		if ((Store.initCodeOffset1 != 0) || (Store.initCodeOffset2 != 0))
		{
			/* The first card image is large ROM */
			File_base = 0;
			if (Card.hdrVersion == 0x01)
				Card_base = ROM_base + PALMOS_1_BIG_ROM_OFFSET;
			else
				Card_base = Card.bigROMOffset;
		}
		else
		{
			/* The first card image is small ROM */
			if (Card.hdrVersion == 0x01)
			{
				File_base = PALMOS_1_BIG_ROM_OFFSET;
				Card_base = ROM_base + PALMOS_1_BIG_ROM_OFFSET;
			}
			else
			{
				File_base = Card.bigROMOffset - ROM_base;
				Card_base = Card.bigROMOffset;
			}
		}
		break;
	default:
		return(NULL);
		break;
	}

	//fprintf (stdout, "ROM_base [0x%08lX]\n", ROM_base);
	//
	// Allocate size for the headers
	if (! (pROM = (ROMPtr)malloc(sizeof(ROMType))))
		return(NULL);
	memset(pROM, 0, sizeof(*pROM));

	pROM->flags 	= flags & (RT_TYPE_MASK);
	pROM->ROM_base  = ROM_base;
	pROM->Card_base = Card_base;
	pROM->File_base = File_base;

	
	/*
	 * Allocate a ROM buffer to hold the entire ROM.
	 */
	if (flags & RT_LARGE)
	{
		pROM->ROMSize  = lseek(hROM, 0, SEEK_END) - File_base;
	}
	else
	{
		pROM->ROMSize = Card.hdrVersion == 0x01 ? 
						PALMOS_1_BIG_ROM_OFFSET :
						Card.bigROMOffset - Card_base;
	}
	pROM->CardSize = pROM->ROMSize;
	pROM->pROM     = (UInt8*)malloc(pROM->ROMSize);
	if (pROM->pROM == NULL)
	{
		FreeROM(pROM);
		return(NULL);
	}

	/*
	 * Read in the entire ROM
	 */
	lseek(hROM, File_base, SEEK_SET);
	if (read(hROM, pROM->pROM, pROM->ROMSize) != pROM->ROMSize)
	{
		FreeROM(pROM);
		return(0);
	}

	if (! Setup_ROM (pROM))
		return (NULL);

	/*
	 * Make a copy of the Database list that we can sort and sort it
	 */
	if (pROM->pDatabaseList)
	{
		UInt32	nBytes	= sizeof(DatabaseHdrPtr) *
						  pROM->pDatabaseList->numDatabases;

		pROM->pSortedDBList = (DatabaseHdrPtr*)malloc(nBytes);
	
		if (pROM->pSortedDBList == NULL)
		{
			FreeROM(pROM);
			return(NULL);
		}

		memcpy(pROM->pSortedDBList,
		       &(pROM->pDatabaseList->databaseOffset[0]), nBytes);

		qsort((void*)pROM->pSortedDBList,
		       pROM->pDatabaseList->numDatabases,
		       sizeof(DatabaseHdrPtr), CompareAddrs);

	}

	pROM->pVersion = GuessVersion (pROM);
	return (pROM);
}

/*
 * Copy a chunk into our buffer...
 */
static UInt32	CopyChunk	(ROMPtr		pROM,
							 LocalID	offset,
							 char**		ppPRC,
							 UInt32		nBytes)
{
	char*	pNewPRC;
	UInt32	nSize;
	UInt16	version = memUChunkVer(pROM->pHeapList->heapOffset[0]);
	
	// Locate the chunk that this points to and copy it
	// into our buffer.
	MemChunkHeaderUnionType*	pChunk	=
								LocateChunk(pROM->pHeapList, offset);
	if (! pChunk)
	{
		return(0);
	}

	// the sizeAdj in the chunk header seems to indicate how many
	// bytes short of a full chunk the data is
	nSize = memUChunkPayloadSize(pChunk,version);

	pNewPRC = (char*)realloc(*ppPRC, nBytes + nSize);
	if (! pNewPRC)
	{
		return(0);
	}
	*ppPRC = pNewPRC;

	memcpy(*ppPRC + nBytes, 
		   (UInt8*)pChunk + memUSizeOfChunkHeader(version), nSize);

	return(nSize);
}

/*
 * Read a PRC from a file, creating a relocated, byte-swapped database header
 */
PRCPtr	ReadPRC		(char*	pFileName)
{
	int				hPRC;
	UInt32			nRead;
	PRCPtr			pPRC;

	if (! pFileName)
		return(NULL);

	pPRC = (PRCPtr)malloc(sizeof(*pPRC));
	if (! pPRC)
		return (NULL);
	
	if ((hPRC = open(pFileName, O_RDONLY)) < 0)
	{
		free (pPRC);
		return(NULL);
	}
	
	pPRC->nBytes = lseek(hPRC, 0, SEEK_END);
	lseek(hPRC, 0, SEEK_SET);

	pPRC->pDB = (DatabaseHdrPtr)malloc(pPRC->nBytes);
	if (! pPRC->pDB)
	{
		free (pPRC);
		close(hPRC);
		return(NULL);
	}

	if ((nRead = read(hPRC, pPRC->pDB, pPRC->nBytes)) != pPRC->nBytes)
	{
		free (pPRC);
		free(pPRC->pDB);
		close(hPRC);
		return(NULL);
	}

	/*
	 * Now, byte-swap and relocate the database header based upon the
	 * beginning of pPRC->pDB.
	 */
	P2H_Translate_PRC (pPRC);
	close(hPRC);

	return(pPRC);
}

/*
 * Extract the PRC (pDatabase) from the ROM
 */
int	WritePRC	(ROMPtr			pROM,
				 DatabaseHdrPtr	pDatabase)
{
	UInt32			nBytes	= sizeof(DatabaseHdrType);
	char*			pPRC	= NULL;
	UInt32			nSize;
	char			FileName[dmDBNameLength + 8];
	UInt32			hPRC;

	nBytes += (pDatabase->recordList.numRecords *
	           (IsResource(pDatabase) ? sizeof(RsrcEntryType) :
				                        sizeof(RecordEntryType)));

	//fprintf (stdout, "Extracting %ld bytes of '%s'\n",
	//		 nBytes, pDatabase->name);

	fprintf (stdout, "Extracting '%s'\n", pDatabase->name);
	
	pPRC = (char*)malloc(nBytes);
	if (! pPRC)
		return(0);

	memcpy(pPRC, pDatabase, nBytes);

	if (pDatabase->appInfoID != 0)
	{
		nSize = CopyChunk(pROM, pDatabase->appInfoID, &pPRC, nBytes);
		if (! nSize)
		{
			free(pPRC);
			return(0);
		}

		nBytes += nSize;
	}

	if (pDatabase->sortInfoID != 0)
	{
		nSize = CopyChunk(pROM, pDatabase->sortInfoID, &pPRC, nBytes);
		if (! nSize)
		{
			free(pPRC);
			return(0);
		}

		nBytes += nSize;
	}

	if (IsResource(pDatabase))
	{
		RsrcEntryPtr	pItem	= (RsrcEntryPtr)
									(&(pDatabase->recordList.firstEntry));
		RsrcEntryPtr	pItemNew;
		UInt32			idex;

		for (idex = 0; idex < pDatabase->recordList.numRecords; idex++)
		{
			nSize = CopyChunk(pROM, pItem->localChunkID, &pPRC, nBytes);
			if (! nSize)
			{
				free(pPRC);
				return(0);
			}

			// Locate the resource entry in our new PRC
			pItemNew = ((RsrcEntryPtr)
					(&(((DatabaseHdrPtr)pPRC)->recordList.firstEntry))) + idex;

			//	(UInt8*)(&(((DatabaseHdrPtr)pPRC)->recordList.firstEntry)) +
			//	(idex * sizeof(RsrcEntryType));

			// Modify the pointer to the resource's data
			// This value will NOT be correct until relocation
			// occurs (relocation will be done based upon the
			//         base of the database)
			pItemNew->localChunkID = nBytes;

			pItem   = (RsrcEntryPtr)((UInt8*)pItem + sizeof(RsrcEntryType));

			nBytes += nSize;
		}
	}
	else
	{
		RecordEntryPtr	pItem	= (RecordEntryPtr)
									(&(pDatabase->recordList.firstEntry));
		UInt32			idex;
		RecordEntryPtr	pItemNew;

		for (idex = 0; idex < pDatabase->recordList.numRecords; idex++)
		{
			nSize = CopyChunk(pROM, pItem->localChunkID, &pPRC, nBytes);
			if (! nSize)
			{
				free(pPRC);
				return(0);
			}

			pItemNew = ((RecordEntryPtr)
					(&(((DatabaseHdrPtr)pPRC)->recordList.firstEntry))) + idex;

			// Modify the pointer to the resource's data
			// This value will NOT be correct until relocation
			// occurs (relocation will be done based upon the
			//         base of the database)
			pItemNew->localChunkID = nBytes;

			pItem   = (RecordEntryPtr)((UInt8*)pItem + sizeof(RecordEntryType));
			nBytes += nSize;
		}
	}

	// Byte swap it...
	{
		OmOverlaySpecType*	pOvly;
		pOvly = LocateOverlayResource((DatabaseHdrPtr)pPRC);
		if (pOvly)
			H2P_OverlaySpec	((OmOverlaySpecType*)(pPRC + (UInt32)pOvly));
	}
	H2P_DatabaseHdr	((DatabaseHdrPtr)pPRC);

	// Finally, output to a file
	sprintf (FileName, "%s.%s", pDatabase->name, 
	         (IsResource(pDatabase) ? "prc" : "pdb"));
	hPRC = open(FileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (! hPRC)
	{
		free(pPRC);
		return(0);
	}

	if (write(hPRC, pPRC, nBytes) != nBytes)
	{
		free(pPRC);
		return(0);
	}

	close(hPRC);
	free(pPRC);

	return nBytes;
}

/*
 *
 */
int	WritePRCs	(ROMPtr			pROM,
				 TypeCtorType	TypeCtorList[],
				 UInt32			numEntries)
{
	UInt32	idex;

	if ((! pROM) || ((numEntries > 0) && (TypeCtorList == NULL)))
		return(0);

	for (idex = 0; idex < pROM->pDatabaseList->numDatabases ; idex++)
	{
		DatabaseHdrPtr	pDatabase	= (DatabaseHdrPtr)
									pROM->pDatabaseList->databaseOffset[idex];

		if (! IsInTypeCtorList(pDatabase->type, pDatabase->creator,
		                       TypeCtorList, numEntries))
			continue;

		if (! WritePRC(pROM, pDatabase))
			return(0);
	}

	return(1);
}
