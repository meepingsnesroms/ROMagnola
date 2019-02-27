#include <stdio.h>

#include "relocate.h"
#include "byte_swap.h"
#include "translate.h"
#include "utils.h"

void	Translate_GenericList	(ROMPtr		pROM,
								 void**		ppList,
								 UInt16		numEntries,
								 UInt16		stepSize,
								 TransFunc	translate)
{
	UInt16 idex;
	
	if (! pROM || ! ppList)
		return;
	
	for (idex = 0; idex < numEntries; idex++)
	{
		if (! IsValidPtr (pROM, ppList))
			break;

		if (IsValidPtr (pROM, *ppList))
			translate (pROM, *ppList);
	
		ppList = (void**)((UInt8*)ppList + stepSize);
	}
}

void H2P_Translate_PRC	(PRCPtr pPRC)
{
	OmOverlaySpecType*	pOvly;

	if (!pPRC || ! pPRC->pDB)
		return;

	pOvly = LocateOverlayResource(pPRC->pDB);
	if (pOvly)
		H2P_OverlaySpec	(pOvly);

	Relocate_DatabaseHdr (pPRC->pDB, (UInt32)pPRC->pDB, 0);
	H2P_DatabaseHdr (pPRC->pDB);
}

void P2H_Translate_PRC	(PRCPtr pPRC)
{
	OmOverlaySpecType*	pOvly;

	if (!pPRC || ! pPRC->pDB)
		return;

	P2H_DatabaseHdr (pPRC->pDB);

	Relocate_DatabaseHdr (pPRC->pDB, 0, (UInt32)pPRC->pDB);

	pOvly = LocateOverlayResource(pPRC->pDB);
	if (pOvly)
		P2H_OverlaySpec	(pOvly);
}

void H2P_Translate_DatabaseHdr (ROMPtr pROM, DatabaseHdrPtr pDatabase)
{
	OmOverlaySpecType*	pOvly;

	if (! pROM || ! pDatabase || ! IsValidPtr (pROM, pDatabase))
		return;

	pOvly = LocateOverlayResource(pDatabase);
	if (pOvly)
		H2P_OverlaySpec	(pOvly);

	Relocate_DatabaseHdr (pDatabase, (UInt32) pROM->pROM, pROM->Card_base);
	H2P_DatabaseHdr (pDatabase);
}

void P2H_Translate_DatabaseHdr (ROMPtr pROM, DatabaseHdrPtr pDatabase)
{
	OmOverlaySpecType*	pOvly;

	if (! pROM || ! pDatabase || ! IsValidPtr (pROM, pDatabase))
		return;

	P2H_DatabaseHdr (pDatabase);
	Relocate_DatabaseHdr (pDatabase, pROM->Card_base, (UInt32) pROM->pROM);

	pOvly = LocateOverlayResource(pDatabase);
	if (pOvly)
		P2H_OverlaySpec	(pOvly);
}

void H2P_Translate_DatabaseList (ROMPtr pROM, DatabaseListPtr pDatabaseList)
{
	if (! pROM || ! pDatabaseList || ! IsValidPtr (pROM, pDatabaseList))
		return;

	/*
	 * Follow the pointers in the list, translate them, relocate the database
	 * list, then byte-swap them
	 */
	Translate_GenericList (pROM, 
							(void**)pDatabaseList->databaseOffset,
							pDatabaseList->numDatabases,
							sizeof (*pDatabaseList->databaseOffset),
							(TransFunc)H2P_Translate_DatabaseHdr);
	Relocate_DatabaseList (pDatabaseList, (UInt32) pROM->pROM, pROM->Card_base);
	H2P_DatabaseList (pDatabaseList);
}

void P2H_Translate_DatabaseList (ROMPtr pROM, DatabaseListPtr pDatabaseList)
{
	if (! pROM || ! pDatabaseList || ! IsValidPtr (pROM, pDatabaseList))
		return;

	/*
	 * Byte-swap the pointers, relocate the database
	 * list, then translate
	 */
	P2H_DatabaseList (pDatabaseList);
	Relocate_DatabaseList (pDatabaseList, pROM->Card_base, (UInt32) pROM->pROM);
	Translate_GenericList (pROM, 
							(void**)pDatabaseList->databaseOffset,
							pDatabaseList->numDatabases,
							sizeof (*pDatabaseList->databaseOffset),
							(TransFunc)P2H_Translate_DatabaseHdr);
}


/*
 * This function assumes that the mem heap header is already in host order
 */
void H2P_Translate_MemChunks (ROMPtr					pROM,
							  UInt16					version,
							  MemChunkHeaderUnionType*	pChunk)
{
	MemChunkHeaderUnionType* pNextChunk = NULL;
	
	if (! pROM || ! pChunk || ! IsValidPtr (pROM, pChunk))
		return;

	while (pChunk)
	{
		if (! IsValidPtr (pROM, pChunk))
			break;

		if (memUChunkIsTerminator (pChunk, version))
			break;

		pNextChunk = memUChunkNext (pChunk, version);

		Relocate_MemChunk (pChunk, version,
							(UInt32) pROM->pROM, pROM->Card_base);
	
		H2P_MemChunk (pChunk, version);
		
		pChunk = pNextChunk;
	}
}

/*
 * This function assumes that the mem heap header is already in host order
 */
void P2H_Translate_MemChunks (ROMPtr					pROM,
							  UInt16					version,
							  MemChunkHeaderUnionType*	pChunk)
{
	if (! pROM || ! pChunk || ! IsValidPtr (pROM, pChunk))
		return;

	while (pChunk)
	{
		if (! IsValidPtr (pROM, pChunk))
			break;

		P2H_MemChunk (pChunk, version);
		Relocate_MemChunk (pChunk, version,
							pROM->Card_base, (UInt32) pROM->pROM);

		if (memUChunkIsTerminator (pChunk,version))
		{
			/* Swap it back, since it isn't a whole memChunk */
			H2P_MemChunk (pChunk, version);
			Relocate_MemChunk (pChunk, version, (UInt32) pROM->pROM,
								pROM->Card_base);
			
			break;
		}

		pChunk = memUChunkNext (pChunk, version);
	}
}

void H2P_Translate_HeapHeader (ROMPtr pROM, MemHeapHeaderUnionType* pHeader)
{
	MemChunkHeaderUnionType*	pFirstChunk;

	if (! pROM || ! pHeader || ! IsValidPtr (pROM, pHeader))
		return;

	/*
	 * First, translate all chunks in this heap.
	 * The first chunk immediately follows the heap header.
	 */
	pFirstChunk = memUHeapFirstChunk (pHeader, memUHeapVer (pHeader));
	H2P_Translate_MemChunks(pROM, memUChunkVer(pHeader), pFirstChunk);

	Relocate_HeapHeader (pHeader, (UInt32) pROM->pROM, pROM->Card_base);
	H2P_HeapHeader (pHeader);
}

void P2H_Translate_HeapHeader (ROMPtr pROM, MemHeapHeaderUnionType* pHeader)
{
	MemChunkHeaderUnionType*	pFirstChunk;
	if (! pROM || ! pHeader || ! IsValidPtr (pROM, pHeader))
		return;

	P2H_HeapHeader (pHeader);
	Relocate_HeapHeader (pHeader, pROM->Card_base, (UInt32) pROM->pROM);

	/*
	 * Finally, translate all chunks in this heap.
	 * The first chunk immediately follows the heap header.
	 */
	pFirstChunk = memUHeapFirstChunk (pHeader, memUHeapVer (pHeader));
	P2H_Translate_MemChunks(pROM, memUChunkVer(pHeader), pFirstChunk);
}

void H2P_Translate_HeapList (ROMPtr pROM, HeapListPtr pHeapList)
{
	if (! pROM || ! pHeapList || ! IsValidPtr (pROM, pHeapList))
		return;

	Translate_GenericList (pROM,
							(void**)pHeapList->heapOffset,
							pHeapList->numHeaps,
							sizeof (*pHeapList->heapOffset),
							(TransFunc)H2P_Translate_HeapHeader);
	Relocate_HeapList (pHeapList, (UInt32) pROM->pROM, pROM->Card_base);
	H2P_HeapList (pHeapList);
}

void P2H_Translate_HeapList (ROMPtr pROM, HeapListPtr pHeapList)
{
	if (! pROM || ! pHeapList || ! IsValidPtr (pROM, pHeapList))
		return;

	P2H_HeapList (pHeapList);
	Relocate_HeapList (pHeapList, pROM->Card_base, (UInt32)pROM->pROM);
	Translate_GenericList (pROM,
							(void**)pHeapList->heapOffset,
							pHeapList->numHeaps,
							sizeof (*pHeapList->heapOffset),
							(TransFunc)P2H_Translate_HeapHeader);
}

/*
 * Only need to follow DatabaseDirID
 */
void H2P_Translate_StorageHeader (ROMPtr pROM, StorageHeaderPtr pStore)
{
	if (! pROM || ! pStore || ! IsValidPtr (pROM, pStore))
		return;

	// Translate_DatabaseList checks the validity of pStore->databaseDirID
	H2P_Translate_DatabaseList (pROM, (DatabaseListPtr) pStore->databaseDirID);
	Relocate_StorageHeader (pStore, (UInt32) pROM->pROM, pROM->Card_base);
	H2P_StorageHeader (pStore);
}

/*
 * Only need to follow DatabaseDirID.
 */
void P2H_Translate_StorageHeader (ROMPtr pROM, StorageHeaderPtr pStore)
{
	if (! pROM || ! pStore || ! IsValidPtr (pROM, pStore))
		return;

	P2H_StorageHeader (pStore);
	Relocate_StorageHeader (pStore, pROM->Card_base, (UInt32) pROM->pROM);
	// Translate_DatabaseList checks the validity of pStore->databaseDirID
	P2H_Translate_DatabaseList (pROM, (DatabaseListPtr) pStore->databaseDirID);
}

void H2P_Translate_CardHeader (ROMPtr pROM, CardHeaderPtr pCard)
{
	if (! pROM || ! pCard || ! IsValidPtr (pROM, pCard))
		return;

	Relocate_CardHeader (pCard, (UInt32) pROM->pROM, pROM->Card_base);
	H2P_CardHeader (pCard);
}

void P2H_Translate_CardHeader (ROMPtr pROM, CardHeaderPtr pCard)
{
	if (! pROM || ! pCard || ! IsValidPtr (pROM, pCard))
		return;

	P2H_CardHeader (pCard);
	Relocate_CardHeader (pCard, pROM->Card_base, (UInt32) pROM->pROM);
}

/*
 * Takes a raw ROM/RAM image that has just been read from disk and sets up
 * pointers and translates correctly.  Assumes that the ROM image is in Palm
 * order.
 */
int Setup_ROM (ROMPtr pROM)
{
	if (! pROM || ! pROM->pROM)
		return 1;

	/*************************************************************
	 * Now, byteswap all structures and relocate all offsets
	 * so that they are valid pointers within our address space.
	 */
	//P2H_Translate_ROM(pROM);

 	pROM->pCard       = (CardHeaderPtr)(pROM->pROM);
	P2H_Translate_CardHeader	(pROM, pROM->pCard);

	if (pROM->pCard->signature != sysCardSignature)
	{
		/* For now, assume that this might be a RAM image */
		H2P_Translate_CardHeader	(pROM, pROM->pCard);

		pROM->pCard  = NULL;
		pROM->pStore = (StorageHeaderPtr)(pROM->pROM);
	}
	else
	{
		pROM->pStore      = (StorageHeaderPtr)(pROM->pCard + 1);
	}

	P2H_Translate_StorageHeader (pROM, pROM->pStore);

	if (pROM->pStore->signature != sysStoreSignature)
	{
		/* Invalid Storage signature */
		return(0);
	}

 	pROM->pHeapList     = (HeapListPtr)(pROM->pStore->heapListOffset);
 	pROM->pDatabaseList = (DatabaseListPtr)(pROM->pStore->databaseDirID);


	/* Translate our heap list along with all chunks in each heap */
	P2H_Translate_HeapList		(pROM, pROM->pHeapList);
	
	return 1;

#if	0
	/*
 	 * Locate the first chunk header.
 	 *   pROM->pCard->blockListOffset points to the first chunk header,
 	 *   however this chunk header is special and cannot be used to
 	 *   locate the first REAL chunk header.
 	 *
 	 *   Because of this, we will locate the first chunk header as
 	 *   if it always immediately follows the last heap header from
 	 *   the heap list.
 	 */
 	//pROM->pHeapHdr = (MemHeapHeaderUnionType*)
     //            (pROM->pHeapList->heapOffset[pROM->pHeapList->numHeaps-1]);

 	pROM->pHeapHdr = (MemHeapHeaderUnionType*)
                 (pROM->pHeapList->heapOffset[0]);

	UInt16 version;
	version = memUHeapVer (pROM->pHeapHdr);

	switch (version)
	{
	case 1:
 		pROM->pFirstChunk = (MemChunkHeaderUnionType*)
							(&(pROM->pHeapHdr->header.ver1)+1);
		break;
	case 2:
 		pROM->pFirstChunk = (MemChunkHeaderUnionType*)
							(&(pROM->pHeapHdr->header.ver2)+1);
		break;
	case 3:
	case 4:
 		pROM->pFirstChunk = (MemChunkHeaderUnionType*)
							(&(pROM->pHeapHdr->header.ver3)+1);
		break;
	default:
		fprintf (stderr, "*** Setup_ROM: Unsupported heap list version %d\n", 
					version);
		break;
	}

 	/* Translate ALL memory chunks */
	P2H_Translate_MemChunks		(pROM, pROM->pFirstChunk);
#endif

}

/*
 * This top-level translation routine figures out the
 * translation direction (Host->Card or Card->Host)
 * based upon the card signature.
 */
void Translate_ROM	(ROMPtr pROM)
{
	if (! pROM)
		return;

	/*
	 * Which direction are we going?
	 */
	if (pROM->pCard->signature == sysCardSignature)
	{
		/* Moving from Host to Card order */
		H2P_Translate_ROM(pROM);
	}
	else if (pROM->pCard->signature == BYTE_SWAP_32(sysCardSignature))
	{
		/* Moving from Card to Host order */
		P2H_Translate_ROM(pROM);
	}
	else
	{
		/* Invalid inputs??? */
		return;
	}
}
	

/****************************************************************************
 ****************************************************************************
 *** Component translation routines
 ****************************************************************************
 ****************************************************************************/

void H2P_Translate_ROM (ROMPtr pROM)
{
	if (! pROM)
		return;

#if	0
	H2P_Translate_MemChunks		(pROM, (MemChunkHeaderUnionType*)pROM->pFirstChunk);
#endif

	H2P_Translate_HeapList		(pROM, pROM->pHeapList);
	H2P_Translate_StorageHeader	(pROM, pROM->pStore);
	H2P_Translate_CardHeader	(pROM, pROM->pCard);
}
	
void P2H_Translate_ROM (ROMPtr pROM)
{
	if (! pROM)
		return;

	P2H_Translate_CardHeader	(pROM, pROM->pCard);
	P2H_Translate_StorageHeader	(pROM, pROM->pStore);
	P2H_Translate_HeapList		(pROM, pROM->pHeapList);

#if	0
	P2H_Translate_MemChunks		(pROM, (MemChunkHeaderUnionType*)pROM->pFirstChunk);
#endif
}

