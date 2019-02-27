/*
 * Deal with the assembly of a new Palm ROM given a set of system PRC/PDB
 * files.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "SystemResources.h"
#include "rom.h"
#include "os_structs.h"
#include "utils.h"
#include "assemble.h"
#include "translate.h"
#include "extract.h"

#include "Crc.h"
#include "byte_swap.h"


static void InitializeFreeChunk	(MemChunkHeaderUnionType*	pChunk,
								 UInt16						version,
								 UInt32						size)
{		
	if (! pChunk)
		return;

	memset (pChunk, 0, size);
	if (version == 0x01)
	{
		Mem1ChunkHeaderPtr pChunk1 = &pChunk->header.ver1;
		pChunk1->flags		= memChunkFlagFree;
		pChunk1->size		= size;
		pChunk1->lockOwner	= dmDynOwnerID;
	}
	else
	{
		MemChunkHeaderPtr pChunk2 = &pChunk->header.ver2;
		pChunk2->free		= 1;
		pChunk2->size		= size;
		pChunk2->owner		= dmDynOwnerID;
	}
}

static void InitializeHeap (MemHeapHeaderUnionType*	hdr,
							UInt16					ver,
							UInt32					size)
{
	MemChunkHeaderUnionType*	pChunk;
	UInt16						chunkVer;

	if (! hdr)
		return;
	
	chunkVer = memUChunkVer(hdr);


	// Initialize this entire heap to 0.
	memset (hdr, 0, size);	//memUSizeOfHeapHeader(ver));
	
	memUHeapSizeAssign(hdr, ver, size);
	
	memUHeapFlags(hdr) |= memHeapFlagReadOnly;
	switch (ver)
	{
	case 1:
		break;
	case 2:
		memUHeapFlags(hdr) |= memHeapFlagVers2;
		break;
	case 3:
		memUHeapFlags(hdr) |= memHeapFlagVers3;
		hdr->header.ver3.firstFreeChunkOffset = memUSizeOfHeapHeader(ver) >> 1;
		break;
	case 4:
		memUHeapFlags(hdr) |= memHeapFlagVers4;
		hdr->header.ver3.firstFreeChunkOffset = memUSizeOfHeapHeader(ver) >> 1;
		break;
	}

	pChunk = memUHeapFirstChunk (hdr, ver);
	InitializeFreeChunk (pChunk, chunkVer,
						 size - memUSizeOfHeapHeader(ver) -
						 memUSizeOfHeapTerminator(ver));
	if (ver > 2)
	{
		memUChunkHOffsetAssign(pChunk, chunkVer, memUChunkSize(pChunk, chunkVer) >> 1);
	}
}

/* Find the first point in the rom which is not contained in a heap */
static
void*	FirstUnmanagedAddress (ROMPtr	pROM)
{
	UInt16					idex;
	MemHeapHeaderUnionType*	pHeapHdr;
	void*					first;
	void*					candidate;
	
	if (! pROM || ! pROM->pHeapList || pROM->pHeapList->numHeaps == 0)
		return NULL;

	pHeapHdr = (MemHeapHeaderUnionType*)pROM->pHeapList->heapOffset[0];
	first = (UInt8*)pHeapHdr + memUHeapInterpSize (pHeapHdr, memUHeapVer(pHeapHdr));
	
	for (idex = 1; idex < pROM->pHeapList->numHeaps; idex++)
	{
		pHeapHdr = (MemHeapHeaderUnionType*)pROM->pHeapList->heapOffset[idex];
		candidate = (UInt8*)pHeapHdr + 
					memUHeapInterpSize (pHeapHdr, memUHeapVer(pHeapHdr));

		if (candidate > first)
			first = candidate;
	}

	return first;
}

/* Find the first available spot for a heap and add one there */
static
MemHeapHeaderUnionType*	AddHeap	(ROMPtr	pROM,
			 					 UInt16	ver,
			 					 UInt32	size)
{
	MemHeapHeaderUnionType* hdr;
	
	if (! pROM)
		return NULL;

	/* 
	 * Make sure the requested version of heap header can handle the 
	 * requested size.
	 */
	if (size > 0x010000 && sizeof (memUHeapSize(hdr,ver)) <= 2)
		return NULL;

	/* Make sure we can hold at least a heap header and chunk header */
	if (size < (memUSizeOfHeapHeader(ver) +
				memUSizeOfChunkHeader(memUChunkVerFromHeapVer(ver))))
		return NULL;
	
	hdr = FirstUnmanagedAddress (pROM);
	if (! hdr)
		return NULL;

	/* Make sure the rom can hold the requested heap */
	if (! IsValidPtr (pROM, (UInt8*)hdr + size))
		return NULL;

	InitializeHeap (hdr, ver, size);

	return hdr;	
}

/*
 * A blank ROM consists of:
 *		Card Header
 *		Storage Header
 *		Block List (00 00 00 00 FF FF FF FF)
 *		Heap List
 *		4 mystery bytes
 *		Heap List Header
 *		Memory Chunk Header (first real one)
 *
 * NOTE: We probably want to pass in a hint that tells what sort of
 *       ROM we're building.  Otherwise, we will need to do size adjustments
 *       for the heap and last free chunk when writing out a Small ROM.
 */
ROMPtr	InitializeROM	(ROMVersion*	pVersion,
						 UInt16			flags)
{
	ROMPtr			pROM	= (ROMPtr)malloc(sizeof(ROMType));
	BlockListPtr	pSpecial;

	if (! pROM || ! pVersion)
	{
		return(NULL);
	}
	memset(pROM, 0, sizeof(*pROM));
	pROM->flags = flags;

	/*
	 * Use a maximum size (2M) for both Small and Large ROMS.
	 * This is to guarantee that we never need to realloc
	 * (which would cause all the pointer that we've set up
	 *  to potentially become invalid -- if realloc moved the memory).
	 */
	if (flags & RT_LARGE)
	{
		pROM->ROMSize  = pVersion->big_ROMSize;
		pROM->CardSize = pVersion->big_cardSize;
	}
	else
	{
		pROM->ROMSize  = pVersion->small_ROMSize;
		pROM->CardSize = pVersion->small_cardSize;
	}

	/* Make sure the size is a multiple of 4k */
	pROM->ROMSize  = (pROM->ROMSize  + 0x0FFF) & 0xFFFFF000;

	if (pROM->CardSize > pROM->ROMSize)
	{
		free(pROM);
		return(NULL);
	}


	pROM->pROM    = (char*)malloc(pROM->ROMSize);
	if (! pROM->pROM)
	{
		FreeROM(pROM);
		return(NULL);
	}
	memset(pROM->pROM, 0xFF, pROM->ROMSize);

	pROM->ROM_base	= pVersion->ROM_base;
	pROM->Card_base	= pVersion->ROM_base;
	if (flags & RT_LARGE)
		pROM->Card_base += pVersion->card_bigROMOffset;

	pROM->File_base	= 0;

	pROM->pCard       = (CardHeaderPtr)(pROM->pROM);
	pROM->pStore      = (StorageHeaderPtr)(pROM->pCard + 1);
	pSpecial          = (BlockListPtr)(pROM->pStore + 1);
	pROM->pHeapList   = (HeapListPtr)(pSpecial + 1);

	/*
	 * Now that we have pointers in place, intialize the structures...
	 *
	 * 1) Card header
	 *		initStack		will point into 'System's 1st boot resource
	 *		resetVector		will point to   'System's 1st boot resource
	 *		halCodeOffset	will point to   'SmallHAL' for the Small ROM
	 *						                'BigHAL'   for the Large ROM
	 */
	{
		CardHeaderPtr	pCard			= pROM->pCard;
		memset(pCard, 0, sizeof(*pCard));

		pCard->signature				= sysCardSignature;
		pCard->hdrVersion				= pVersion->card_hdrVersion;
		pCard->flags					= pVersion->card_flags;
		strncpy(pCard->name,			  pVersion->card_name,
														sizeof(pCard->name));
		strncpy(pCard->manuf,			  pVersion->card_manuf,
										                 sizeof(pCard->manuf));
		pCard->version					= pVersion->card_version;
		pCard->creationDate				= unix_time_to_pilot_time(time(NULL));
		pCard->numRAMBlocks				= 0x0001;
		pCard->blockListOffset			= (UInt32)pSpecial;

		if (pCard->hdrVersion > 0x0001)
		{
			pCard->readWriteParmsOffset		= 0x00000000;
			pCard->readWriteParmsSize		= 0x00000000;

			// Trying to get -0x2000, since the RO params live at 0x6000 from
			// the start of the ROM and the big ROM lives at 0x8000.
			pCard->readOnlyParmsOffset		= pVersion->card_readOnlyParmsOffset -
				(pROM->Card_base - pROM->ROM_base) + (UInt32) pROM->pROM;
											//0x00006000 + ((UInt32)pROM->pROM);
			pCard->bigROMOffset				= (UInt32)pROM->pROM;
			if (flags & RT_SMALL)
				pCard->bigROMOffset += pVersion->card_bigROMOffset;

			pCard->checksumBytes			= pROM->CardSize;
			pCard->checksumValue			= 0x00000000;
		}

		if (pCard->hdrVersion > 0x0002)
		{
			pCard->readWriteWorkingOffset	= 0x00000000;
			pCard->readWriteWorkingSize		= 0x00000000;
		}

		if (pCard->hdrVersion > 0x0003)
		{
			pCard->halCodeOffset			= 0x00000000;
		}
	}

	/*
	 * 2) Storage header
	 *		initCodeOffset1		will point into 'System's 2nd boot resource
	 *		initCodeOffset2		will point into 'System's 3rd boot resource
	 *		databaseDirID		will point to the database list
	 */
	{
		StorageHeaderPtr	pStore	= pROM->pStore;
		memset(pStore, 0, sizeof(*pStore));

		pStore->signature				= sysStoreSignature;
		pStore->version					= pVersion->store_version;
		pStore->flags					= pVersion->store_flags;
		strncpy(pStore->name,			  pVersion->store_name,
												sizeof(pStore->name));
		pStore->creationDate			= 0x00000000;
		pStore->backupDate				= 0x00000000;
		pStore->heapListOffset			= (UInt32)pROM->pHeapList;
	}

	/*
	 * 3) Special - appears to be a block list (whatever that is)
	 *		It seems to always be '0x00000000 0xFFFFFFFF'
	 */
	memset(pSpecial,           0x00, 4);
	memset((UInt8*)pSpecial+4, 0xFF, 4);

	/*
	 * 4) Heap List
	 */
	{
		HeapListPtr				pList			= pROM->pHeapList;
		Int32					TotalHeapSize	= (UInt32)pROM->pROM +
												  pROM->CardSize -
												  (UInt32)pList -
												  sizeof(*pList);
		UInt32					SingleHeapSize;
		UInt16					HeapVer;
		UInt16					nHeaps			= 0;
		UInt16					idex;

		HeapVer					= memUHeapVerFromFlags(pVersion->heap_flags);

		if (HeapVer < 2)
		{
			/*
			 * Maximum Heap size is 64K
			 */
			SingleHeapSize = 0x10000;
		}
		else
		{
			/*
			 * Maximum Heap size is 4G and our ROM can only be 2M
			 * so we're safe in hardcoding this value
			 * (until memory becomes REEEALLY cheap)
			 */
			SingleHeapSize = TotalHeapSize;
		}

		/*
		 * Now, we need to figure out how many heaps (given SingleHeapSize)
		 * the total heap should be divided into...
		 */
		while (TotalHeapSize > 0)
		{
			TotalHeapSize -= sizeof(pList->heapOffset[0]) + SingleHeapSize;
			nHeaps++;
		}

		/*
		 * This may seem off by 1 because we need to include the
		 * NULL offset item at the end of the list.
		 */
		TotalHeapSize = (UInt32)pROM->pROM + pROM->CardSize -
		                (UInt32)pList - sizeof(*pList) -
		                sizeof(pList->heapOffset[0])*nHeaps;

		if (HeapVer > 1)
		{
			/* Grab the newly computed value */
			SingleHeapSize = TotalHeapSize;
		}

		/* For very early ROMS, the TotalHeapSize is less than 0x10000 */
		SingleHeapSize = SingleHeapSize > TotalHeapSize ? 
							TotalHeapSize : SingleHeapSize;


		memset(pList, 0, sizeof(*pList) + sizeof(pList->heapOffset[0])*nHeaps);

		/*
		 * Using our computed sizes, initialize the heap list and
		 * it's constituent heaps.
		 */
		//pList->numHeaps			= nHeaps;
		pList->numHeaps			= 1;
		pList->heapOffset[0]	= (UInt32)((UInt8*)(pList+1) +
								  nHeaps * sizeof(pList->heapOffset[0]));
		InitializeHeap((MemHeapHeaderUnionType*)pList->heapOffset[0], HeapVer,
						SingleHeapSize);

		for (idex = 1; idex < nHeaps; idex++)
		{
			UInt32	size	= SingleHeapSize;

			if (TotalHeapSize < SingleHeapSize)
				size = TotalHeapSize;

			pList->heapOffset[idex] = (UInt32)AddHeap(pROM, HeapVer, size);
			if (! pList->heapOffset[idex])
			{
				/*
				 * Assume this is the last heap and it's not big
				 * enough to contain even a heap header (which AddHeap checks).
				 */
				break;
			}

			TotalHeapSize -= size;
			pList->numHeaps++;
		}
	}

	pROM->pVersion = pVersion;

	return (pROM);
}

/*
 * Given a heap header, find the first free memory chunk in the heap
 */
static
MemChunkHeaderUnionType*
	LocateFirstFreeChunk	(MemHeapHeaderUnionType*	pHeapHdr)
{
	MemChunkHeaderUnionType*	pChunk	= NULL;

	if (! pHeapHdr)
		return(NULL);

	if (memUHeapVer(pHeapHdr) > 2)
	{
		if (! pHeapHdr->header.ver3.firstFreeChunkOffset)
			return(NULL);

		pChunk = (MemChunkHeaderUnionType*)((UInt8*)pHeapHdr +
	                         (pHeapHdr->header.ver3.firstFreeChunkOffset << 1));
	}
	else
	{
		/* We have to walk the chunk list to find the first free chunk */
		UInt16	ver	= memUHeapVer(pHeapHdr);
		pChunk = memUHeapFirstChunk (pHeapHdr, ver);

		ver = memUChunkVer(pHeapHdr);
		while (! memUChunkIsTerminator(pChunk, ver) &&
		       ! memUChunkFree(pChunk,ver))
		{
			pChunk = (MemChunkHeaderUnionType*)((UInt8*)pChunk +
							                     memUChunkSize(pChunk,ver));
		}

		if (memUChunkIsTerminator(pChunk, ver))
			/* We didn't find a free chunk */
			pChunk = NULL;
	}

	return(pChunk);
}

/*
 * Look for all the mem chunks that are owned by owner (usually dmPadOwnerID),
 * and set free.  The hOffset is left alone.  For padding purposes, this
 * function should only be called after everything else has been allocated in
 * ROM, since we don't want
 * anything to use our pad.
 */
static
void	SetChunksFree	(ROMPtr pROM,
						 UInt8	owner)
{
	UInt16	numHeaps;
	UInt16	idex;
	
	if (! pROM)
		return;
		
	numHeaps = pROM->pHeapList->numHeaps;
	
	for (idex = 0; idex < numHeaps; idex++)
	{
		MemHeapHeaderUnionType*	pHeapHdr = 
				(MemHeapHeaderUnionType*) pROM->pHeapList->heapOffset[idex];
		
		UInt16	heapVer	= memUHeapVer(pHeapHdr);
		UInt16	chunkVer = memUChunkVer (pHeapHdr);

		MemChunkHeaderUnionType* pChunk = memUHeapFirstChunk (pHeapHdr, heapVer);

		while (! memUChunkIsTerminator(pChunk, chunkVer))
		{
			if (memUChunkOwner (pChunk, chunkVer) == owner)
			{
				ROMfree(pROM, (void*)memUChunkData (pChunk,chunkVer));
				pChunk = memUHeapFirstChunk (pHeapHdr, heapVer);
				continue;
			}

			pChunk			= memUChunkNext (pChunk, chunkVer);
		}
	}
}


/*
 * Alloc size bytes, owned by owner in the given rom.  owner is dmRecOwnerID
 * for database records, and dmMgrOwnerID for everything else, i.e.  database
 * headers, database directories, ...
 */
static
void* HeapAlloc	(MemHeapHeaderUnionType*	pHeapHdr,
				 UInt32						size,
				 UInt8						owner)
{
	UInt32						roundedSize = (size + 1) & 0xFFFFFFFE;
	UInt32						newsize		= 0;
	UInt16						ver;
	MemChunkHeaderUnionType*	prevFree	= NULL;
	MemChunkHeaderUnionType*	oldFree;
	MemChunkHeaderUnionType*	newFree;
	
	if (! pHeapHdr)
		return(NULL);

	ver     = memUChunkVer(pHeapHdr);
	oldFree = LocateFirstFreeChunk(pHeapHdr);

	// Locate a free chunk large enough to accomodate the request
	while ((oldFree) && (! memUChunkIsTerminator(oldFree,ver)) &&
	       (roundedSize > memUChunkSize(oldFree, ver) - memUSizeOfChunkHeader(ver)))
	{
		prevFree = oldFree;
		oldFree  = memUChunkNextFree(oldFree, ver);
	}

	if ((! oldFree) || (memUChunkIsTerminator(oldFree,ver)))
		return(NULL);

	// Carve out the requested space
	newFree = (MemChunkHeaderUnionType*)((UInt8*)oldFree + roundedSize + 
	                              memUSizeOfChunkHeader(ver));
	newsize = memUChunkSize(oldFree,ver) - roundedSize -
						 memUSizeOfChunkHeader(ver);

	if (newsize < memUSizeOfChunkHeader(ver))
	{
		/*
		 * Handle the case where the last chunk is not large
		 * enough to hold a chunk header
		 */
		roundedSize += newsize;
		newsize      = 0;
	}

	if (newsize)
		InitializeFreeChunk (newFree, ver, newsize);

	if (memUChunkHOffset(oldFree, ver) && newsize)
	{
		// Only update the hOffset (pointer to next free chunk) if
		// the hOffset of the old free chunk was set.
      memUChunkHOffsetAssign(newFree, ver, memUChunkHOffset(oldFree, ver) - ((roundedSize + memUSizeOfChunkHeader(ver)) >>1));
	}


	if ((memUHeapVer(pHeapHdr) > 2) && (! prevFree))
	{
		// Modify the pointer to the first free chunk
		if (newsize)
			pHeapHdr->header.ver3.firstFreeChunkOffset = ((UInt32)newFree -
		                                                 (UInt32)pHeapHdr) >> 1;
		else
			pHeapHdr->header.ver3.firstFreeChunkOffset += memUChunkHOffset(oldFree,ver);
	}


	if (prevFree)
	{
		// The previous free chunk now needs to point to the new free chunk.
		if (newsize)
			memUChunkHOffsetAssign(prevFree, ver, ((UInt32)newFree - (UInt32)prevFree) >> 1);
		else
			memUChunkHOffsetAssign(prevFree, ver, memUChunkHOffset(prevFree, ver) + memUChunkHOffset(oldFree,ver));
	}



	if (ver > 1)
	{
		MemChunkHeaderPtr	pChunk	= &oldFree->header.ver2;

		pChunk->free		= 0x0;
		pChunk->size		= roundedSize + memUSizeOfChunkHeader(ver);
		pChunk->sizeAdj		= roundedSize - size;
		pChunk->lockCount	= memPtrLockCount;
		pChunk->owner		= owner;
		pChunk->hOffset		= 0x0;
	}
	else
	{
		Mem1ChunkHeaderPtr	pChunk	= &oldFree->header.ver1;

		pChunk->size		= roundedSize + memUSizeOfChunkHeader(ver);
		pChunk->flags		= roundedSize - size;
		pChunk->lockOwner	= (memPtrLockCount << 4) | owner;
		pChunk->hOffset		= 0x0;
	}

	return memUChunkData (oldFree, ver);
}

/*
 * Look in each heap for a free chunk of sufficient size.
 * If we find one, allocate the requested bytes there.
 */
void*	ROMalloc	(ROMPtr			pROM,
					 UInt32			size,
					 UInt8			owner)
{
	UInt32	idex;
	void*	pMem	= NULL;

	for (idex = 0; idex < pROM->pHeapList->numHeaps && ! pMem; idex++)
	{
		pMem = HeapAlloc((MemHeapHeaderUnionType*)pROM->pHeapList->heapOffset[idex],
		                 size,owner);
	}

	return(pMem);
}

/*
 * Given two adjacent free chunks, merge them iff they are contiguous
 */
static int
MergeIfAdjacent		(MemChunkHeaderUnionType*	pFirst,
					 MemChunkHeaderUnionType*	pSecond,
					 UInt32						ver)
{
	if ((pFirst && pSecond) &&
		(! memUChunkIsTerminator(pFirst,ver) && ! memUChunkIsTerminator(pSecond,ver)) &&
		(memUChunkFree(pFirst,ver) && memUChunkFree(pSecond,ver)) &&
		memUChunkNext(pFirst,ver) == pSecond)
	{
		/* Free, contiguous and neither are terminators */
		if (memUChunkHOffset(pSecond,ver) != 0)
			memUChunkHOffsetAssign(pFirst,ver, memUChunkHOffset(pFirst,ver) + memUChunkHOffset(pSecond,ver));
		else
			memUChunkHOffsetAssign(pFirst,ver, 0);

		memUChunkSizeAssign(pFirst,ver, memUChunkSize(pFirst,ver) + memUChunkSize(pSecond,ver));

		return (1);
	}

	return (0);
}

/*
 * Free the given memory from its heap
 */
void	ROMfree		(ROMPtr			pROM,
					 void*			ptr)
{
	UInt32						idex;
	UInt32						heapVer;
	UInt32						chunkVer;
	MemHeapHeaderUnionType*		pHeap		= NULL;
	MemChunkHeaderUnionType*	pChunk		= NULL;
	MemChunkHeaderUnionType*	pFreeChunk	= NULL;

	if (! pROM || ! ptr || ! IsValidPtr(pROM, ptr))
		return;

	for (idex = 0; idex < pROM->pHeapList->numHeaps; idex++)
	{
		pHeap   = (MemHeapHeaderUnionType*)pROM->pHeapList->heapOffset[idex];
		heapVer = memUHeapVer(pHeap);

		if (((UInt32)pHeap < (UInt32)ptr) &&
			((UInt32)ptr < ((UInt32)pHeap + memUHeapInterpSize(pHeap,heapVer))))
			break;
	
		pHeap = NULL;
	}

	if (! pHeap)
		/* ARRRGH!!  Cannot locate the memory in any heaps?!?!?!? */
		return;

	/*
	 * Grab a pointer to the chunk header that contains 'ptr'
	 */
	chunkVer = memUChunkVerFromHeapVer(heapVer);
	pChunk   = LocateAddrInHeap(pHeap, (UInt32)ptr);
	if (ptr != (void*)memUChunkData (pChunk, chunkVer) || memUChunkFree (pChunk,chunkVer))
	{
		// Attempt to free memory in the middle of a chunk
		return;
	}

	// Initialize this chunk as a free chunk.
	InitializeFreeChunk(pChunk, chunkVer, memUChunkSize(pChunk, chunkVer));


	pFreeChunk = LocateFirstFreeChunk(pHeap);
	if ((! pFreeChunk) || (pFreeChunk > pChunk))
	{
		// This new free chunk will be the first one on the free list.
		if (pFreeChunk)
			memUChunkHOffsetAssign(pChunk,chunkVer, ((UInt32)pFreeChunk - (UInt32)pChunk) >> 1);

		if (heapVer > 2)
		{
			if (! pFreeChunk)
				// In version 3 and above, the final free chunk points
				// to the heap terminator.
            memUChunkHOffsetAssign(pChunk,chunkVer, ((UInt32)pHeap +
                                                     memUHeapSize(pHeap, heapVer) -
                                                     memUSizeOfHeapTerminator(chunkVer) -
                                                     (UInt32)pChunk) >> 1);

			pHeap->header.ver3.firstFreeChunkOffset = ((UInt32)pChunk - (UInt32)pHeap)>>1;
		}

	}
	else
	{
		// This new free chunk is somewhere on the list (possibly at the end, but
		// NOT at the beginning)
		UInt32	hOffset;

		while (((hOffset = memUChunkHOffset(pFreeChunk,chunkVer)) != 0) &&
			   (pChunk > memUChunkNextFree(pFreeChunk,chunkVer)))
		{
			pFreeChunk = memUChunkNextFree(pFreeChunk,chunkVer);
		}

		// pFreeChunk should be pointing to the free chunk that should
		// immediately preceed us.
		if (hOffset)
			hOffset = ((UInt32)memUChunkNextFree(pFreeChunk,chunkVer) - (UInt32)pChunk)
											>> 1;

		memUChunkHOffsetAssign(pChunk, chunkVer, hOffset);
		memUChunkHOffsetAssign(pFreeChunk,chunkVer, ((UInt32)pChunk - (UInt32)pFreeChunk)>>1);
	}

	MergeIfAdjacent(pChunk, memUChunkNextFree(pChunk,chunkVer), chunkVer);
	if (pFreeChunk)
		MergeIfAdjacent(pFreeChunk, pChunk, chunkVer);
}

/*
 * Add some portion of the provided PRC to the ROM.  The flags indicate which
 * part of the PRC (either the header or the records) to add.  Returns 1 on
 * success and 0 on failure.  
 *
 * Flag values:
 * 		PR_HEADER
 * 		PR_RECORDS		(includes the app info block and sort info block, if
 * 					 	any)
 */
int	AddPRC	(ROMPtr						pROM, 
			 DatabaseListPtr			pDBList,
			 PRCPtr						pPRC,
			 UInt16						flags,
			 int						DBIdex)
{
	UInt32			DBHeaderSize;
	DatabaseHdrPtr	pDB;
	RecordListPtr	pList;
	RsrcEntryPtr	pRsrc;
	RecordEntryPtr	pRecord;
	RsrcEntryPtr	pRsrcNew;
	RecordEntryPtr	pRecordNew;
	
	int idex;
	
	if (! pROM || ! pDBList || ! pPRC || ! pPRC->pDB)
		return 0;
	
	pList	= &(pPRC->pDB->recordList);
	pRsrc	= (RsrcEntryPtr)(&pList->firstEntry);
	pRecord = (RecordEntryPtr)(&pList->firstEntry);
	
	DBHeaderSize = sizeof (*pDB) + pList->numRecords *
					(IsResource (pPRC->pDB) ? sizeof (RsrcEntryType) :
					sizeof (RecordEntryType));

	if (flags & PR_HEADER)
	{
		/* We haven't done the header, so do it now */
		pDB = (DatabaseHdrPtr) ROMalloc (pROM, DBHeaderSize, dmMgrOwnerID);
		if (! pDB)
			return 0;
		memcpy (pDB, pPRC->pDB, DBHeaderSize);
		pDBList->databaseOffset[DBIdex] = (UInt32)pDB;
	}
	
	if (pDBList->databaseOffset[DBIdex])
	{
		/* We've already done the header, so we need to grab it to correct the
		 * pointers in the record list in ROM */
		pDB = (DatabaseHdrPtr) pDBList->databaseOffset[DBIdex];
	}
	else
	{
		/* We haven't done the header, but we still have to keep track of the
		 * record list */
		pDB = pPRC->pDB;
	}

	/*
	 * By now, pDB points to the correct place to update the record list
	 * pointers, whether or not they are in the ROM.
	 */
	if (! (flags & PR_RECORDS))
		return 1;
	
	pRsrcNew = (RsrcEntryPtr)(&pDB->recordList.firstEntry);
	pRecordNew = (RecordEntryPtr)(&pDB->recordList.firstEntry);
	
	/* Copy and Relocate the appinfo block */
	if (pPRC->pDB->appInfoID)
	{
		UInt32	appInfoSize;
		LocalID	appInfoID;

		/* Figure out how big the appInfoBlock is */
		if (pPRC->pDB->sortInfoID)
		{
			appInfoSize = pPRC->pDB->sortInfoID - pPRC->pDB->appInfoID;
		}
		else if (pList->numRecords > 0)
		{
			if (IsResource(pPRC->pDB))
				appInfoSize = pRsrc->localChunkID - pPRC->pDB->appInfoID;
			else
				appInfoSize = pRecord->localChunkID - pPRC->pDB->appInfoID;
		}
		else
		{
			appInfoSize = pPRC->nBytes - 
							(pPRC->pDB->appInfoID - (UInt32)pPRC->pDB);
		}

		/* Allocate space for it in the rom and copy it over */
		appInfoID		= pPRC->pDB->appInfoID;
		pDB->appInfoID = (LocalID)ROMalloc (pROM, appInfoSize, dmRecOwnerID);
		if (! pDB->appInfoID)
			return 0;
		memcpy ((char*)pDB->appInfoID, (char*)appInfoID,appInfoSize);
	}

	/* Copy and Relocate the sortinfo block */
	if (pPRC->pDB->sortInfoID)
	{
		UInt32	sortInfoSize;
		LocalID	sortInfoID;
		
		/* Figure out how big the sortInfoBlock is */
		if (pList->numRecords > 0)
		{
			if (IsResource(pPRC->pDB))
				sortInfoSize = pRsrc->localChunkID - pPRC->pDB->sortInfoID;
			else
				sortInfoSize = pRecord->localChunkID - pPRC->pDB->sortInfoID;
		}
		else
		{
			sortInfoSize = pPRC->nBytes - 
							(pPRC->pDB->sortInfoID - (UInt32)pPRC->pDB);
		}

		/* Allocate space for it in the rom and copy it over */
		sortInfoID = pPRC->pDB->sortInfoID;
		pDB->sortInfoID = (LocalID)ROMalloc (pROM, sortInfoSize, dmRecOwnerID);
		if (! pDB->sortInfoID)
			return 0;
		memcpy ((char*)pDB->sortInfoID, (char*)sortInfoID, sortInfoSize);
	}

	
	/* Copy and Relocate the resources (or records) */
	for (idex = 0; idex < pList->numRecords; idex++)
	{
		int		recordSize;
		LocalID recordFrom;
		LocalID	recordTo;
		
		/* How big is this resource/record, and where did it come from? */
		if (IsResource (pDB))
		{
			if (idex < pList->numRecords - 1)
			{
				recordSize = (pRsrc + 1)->localChunkID - pRsrc->localChunkID;
			}
			else
			{
				recordSize = pPRC->nBytes - 
								(pRsrc->localChunkID - (UInt32)pPRC->pDB);
			}
			recordFrom = pRsrc->localChunkID;
		}
		else
		{
			if (idex < pList->numRecords - 1)
			{
				recordSize = (pRecord+1)->localChunkID - pRecord->localChunkID;
			}
			else
			{
				recordSize = pPRC->nBytes - 
							(pRecord->localChunkID - (UInt32)pPRC->pDB);
			}
			recordFrom = pRecord->localChunkID;
		}

		/* Copy the data */
		recordTo = (LocalID)ROMalloc (pROM, recordSize, dmRecOwnerID);
		if (! recordTo)
			return 0;
		memcpy ((char*)recordTo, (char*)recordFrom, recordSize);

		/* Remember where we put it */
		if (IsResource(pDB))
		{
			pRsrcNew->localChunkID = recordTo;
		}
		else
		{
			pRecordNew->localChunkID = recordTo;
		}
		
		/* Move on */
		pRsrc++;
		pRecord++;
		pRsrcNew++;
		pRecordNew++;
	}

	return pList->numRecords;
}

static
int OrderPRCs	(PRCPtr*	pPRCList,
				 char*		PRCNames[],
				 UInt32		numPRCs,
				 PRCIDPtr	pPRCOrder)
{
	UInt32			idex;
	UInt32			current	= 0;
	DatabaseListPtr	pDBList;
	int				listSize	= sizeof (DatabaseListType) +
					(numPRCs - 1) * sizeof (pDBList->databaseOffset[0]);


	if (! pPRCList || ! PRCNames || ! pPRCOrder)
		return(1);
	
	/* Create a dummy pDBList for use in LocateDB and friends */
	pDBList = (DatabaseListPtr)malloc(listSize);
	if (! pDBList)
	{
		free(pPRCList);
		return(0);
	}
	memset (pDBList, 0, listSize);
	pDBList->numDatabases = numPRCs;

	for (idex = 0; idex < numPRCs; idex++)
	{
		pDBList->databaseOffset[idex] = (UInt32)pPRCList[idex]->pDB;
	}

	for (idex = 0; pPRCOrder[idex].type != 0; idex++)
	{
		UInt32			pos		= current;
	
		DatabaseHdrPtr	pPRC	= NULL;
		UInt32			tmp		= 0;
		
		OmOverlaySpecType*	pOvly	= NULL;

		/* Must find the original PRC first to find overlays, so we have to
		 * start at the beginning of the list */
		if (pPRCOrder[idex].isOverlay)
			pos = 0;

		/* Search for the right PRC and put it in place */
		pPRC = LocateDB (pDBList, 
						 pPRCOrder[idex].type, 
						 pPRCOrder[idex].creator, 
						 &pos);
		if (! pPRC)
			continue;

		if (pPRCOrder[idex].isOverlay)
		{
			pOvly = LocateOverlayResource (pPRC);
			if (! pOvly)
				continue;

			pos = current;
			pPRC = LocateDBOverlay	(pDBList, pOvly, &pos);
			if (! pPRC)
				continue;

		}
		
		tmp = pDBList->databaseOffset[pos];
		pDBList->databaseOffset[pos] = pDBList->databaseOffset[current];
		pDBList->databaseOffset[current] = tmp;

		tmp = (UInt32) pPRCList[pos];
		pPRCList[pos] = pPRCList[current];
		pPRCList[current] = (PRCPtr) tmp;
		
		tmp = (UInt32) PRCNames[pos];
		PRCNames[pos] = PRCNames[current];
		PRCNames[current] = (char*) tmp;

		current++;
	}
	free(pDBList);
	return(1);
}

/*
 * Process the layout string given in the ROMVersion.  The syntax of the string
 * is comma-separated directives.  A directive that is followed by a '+'
 * applies to all the PRCs.  A directive that is followed by a '-'
 * applies to all the PRCs, but in reverse order.  Valid directives are:
 *
 * 	n	:	An n-byte pad
 * 	'd'	:	the database header of a PRC
 * 	'r'	:	the records of a PRC
 * 	'D'	:	the directory listing of the ROM
 *
 *
 *  The 'd' and 'r' directives must be followed by '+' or '-' for anything
 *  useful to happen.  'D' can never be followed by '+' or '-'.  Each of 'D',
 *  'd', and 'r' must occur exactly once.  A pad may occur at any point and may
 *  be repeated as necessary. 
 * 
 * 	For example, a layout string of "265,dr+,D" indicates that the ROM should
 * 	start with a pad of 265 bytes (after any necessary headers, like the card
 * 	and storage headers), followed by the headers and records of each PRC,
 * 	followed by the database directory listing.
 */
static
int	LayoutPRCs	(ROMPtr		pROM,
				 PRCPtr*	pPRCList,
				 char*		PRCNames[],
				 UInt32		numPRCs,
				 UInt16		flags)
{
	char*			layout		= NULL;
	char*			group		= NULL;
	DatabaseListPtr	pDBList		= NULL;
	UInt32			idex		= 0;
	int				listSize	= sizeof (DatabaseListType) +
			(numPRCs - 1) * sizeof (pDBList->databaseOffset[0]);
	int 			oldListSize = 0;
	int 			oldNumDBs	= 0;
	
	if (! pROM || ! pPRCList || ! PRCNames)
		return(1);

	if (pROM->pDatabaseList)
	{
		oldNumDBs = pROM->pDatabaseList->numDatabases;
		oldListSize = oldNumDBs * 
				    sizeof (pROM->pDatabaseList->databaseOffset[0]);
		listSize += oldListSize;
	}
	
	if (pROM->flags & RT_LARGE)
		layout = oldListSize ? pROM->pVersion->big_layout : 
							   pROM->pVersion->big_addLayout;
	else
		layout = oldListSize ? pROM->pVersion->small_layout : 
							   pROM->pVersion->small_addLayout;
	
	if (! layout)
		layout = "D,dr+";

	group	= layout;

	pDBList = (DatabaseListPtr) malloc (listSize);
	if (! pDBList)
		goto error;
	memset (pDBList, 0, listSize);
	if (oldListSize)
	{
		memcpy (pDBList, pROM->pDatabaseList, 
				sizeof (*pROM->pDatabaseList) + oldListSize);
		ROMfree (pROM, pROM->pDatabaseList);
		pROM->pDatabaseList         = NULL;
		pROM->pStore->databaseDirID = 0;
	}
	pDBList->numDatabases += numPRCs;
	
	while (*group)
	{
		char*	star			= NULL;
		char*	command			= NULL;
		char*	badchar			= NULL;
		UInt32	pad				= 0;
		UInt32	chunkHdrSize	= memUSizeOfChunkHeader 
					(memUChunkVer (pROM->pHeapList->heapOffset[0]));
		
		/* For bad layout strings, skip multiple commas */
		while (*group == ',')
			group++;

		if (! *group)
			break;

		star = strchr (group, ',');
		if (star)
			star--;
		/* For the last group of commands, point to the end instead */
		else
			star = group + strlen (group) - 1;

		if (*star == '+' || *star == '-')
		{
			Int32 start;
			Int32 end;
			Int32 delta;

			if (*star == '+')
			{
				start	= 0;
				end		= numPRCs;
				delta	= 1;
			}
			else
			{
				start	= numPRCs - 1;
				end		= -1;
				delta	= -1;
			}
				
			for (idex = start; idex != end; idex += delta)
			{
				command = group;
				while (command <= star)
				{
					switch (*command)
					{
					case 'D':
						goto error;
						break;
					case 'd':
						if (! AddPRC (pROM, pDBList, pPRCList[idex],
							  		  PR_HEADER,
									  idex + oldNumDBs))
							goto error;
						command++;
						break;
					case 'r':
						if (! AddPRC (pROM, pDBList, pPRCList[idex],
							  		  PR_RECORDS,
									  idex + oldNumDBs))
							goto error;
						command++;
						break;
					case '+':
					case '-':
						/* Premature '+' or '-'*/
						if (command != star)
							goto error;
						command++;
						break;
					default:
						pad = strtol (command, &badchar, 0);
						if (badchar == command || pad < chunkHdrSize)
							goto error;
	
						pad -= chunkHdrSize;
						(void)ROMalloc (pROM, pad, dmPadOwnerID);
						command = badchar;
						break;
					}
				}
			}
		}
		else
		{
			command = group;
			while (command <= star)
			{
				switch (*command)
				{
				case 'D':
					{
						DatabaseListPtr	pDBListTmp = 
								(DatabaseListPtr) ROMalloc (pROM, 
															listSize, 
															dmMgrOwnerID);
						if (! pDBListTmp)
							goto error;
							
						memcpy (pDBListTmp, pDBList, listSize);
						free (pDBList);
						pDBList = pDBListTmp;

						pROM->pDatabaseList         = pDBList;
						pROM->pStore->databaseDirID = (LocalID)pDBList;
					}
					command++;
					break;
				case 'd':
				case 'r':
					goto error;
					break;
				case '+':
				case '-':
					/* Premature '+' or '-'*/
					if (command != star)
						goto error;
					command++;
					break;
				default:
					pad = strtol (command, &badchar, 0);
					if (badchar == command || pad < chunkHdrSize)
						goto error;

					pad -= chunkHdrSize;
					(void)ROMalloc (pROM, pad, dmPadOwnerID);
					command = badchar;
					break;
				}
			}
		}
		group = strchr (group, ',');
		if (! group)
			break;
		group++;
	}

	/* In case of padding, set all the pad chunks to be free */
	SetChunksFree (pROM, dmPadOwnerID);
	
	return(0);

error:
	if (pDBList && ! IsValidPtr (pROM, pDBList))
		free (pDBList);

	return(1);
}

/*
 * Add the given list of PRCs to the ROM
 *	numPRCs		is similar to argc
 *	PRCNames	is similar to argv[]
 */
int	AddPRCs	(ROMPtr						pROM, 
			 int						numPRCs,
			 char*						PRCNames[],
			 UInt16						flags)
{
	int				idex;
	PRCPtr*			pPRCList;
	PRCIDPtr		pPRCOrder	= NULL;
	
	if (! pROM || numPRCs < 1 || ! PRCNames)
		return 0;

	/* Read in all the PRCs */
	if (! (pPRCList = (PRCPtr*)malloc(numPRCs * sizeof(PRCPtr))))
		return 0;
	memset(pPRCList, 0, numPRCs * sizeof(PRCPtr));

	for (idex = 0; idex < numPRCs; idex++)
	{
		pPRCList[idex] = ReadPRC(PRCNames[idex]);

		if (! pPRCList[idex])
		{
			fprintf (stderr, "*** Error reading PRC '%s'\n", PRCNames[idex]);
			FreePRCList(pPRCList, idex);
			return 0;
		}

	}

	/************************************************************
	 * Now, if we have PRC ordering information, order the PRCs
	 */
	pPRCOrder = pROM->flags & RT_LARGE ? 
				pROM->pVersion->big_PRCList : pROM->pVersion->small_PRCList;
						
	if (pPRCOrder && ! OrderPRCs (pPRCList, PRCNames, numPRCs, pPRCOrder))
	{
		fprintf (stderr, "*** Error re-ordering PRCs\n");
		FreePRCList (pPRCList, numPRCs);
		return 0;
	}

	if (LayoutPRCs(pROM, pPRCList, PRCNames, numPRCs, flags))
	{
		fprintf (stderr, "*** Error laying out PRCs\n");
		FreePRCList (pPRCList, numPRCs);
		return 0;
	}	

	FreePRCList(pPRCList, numPRCs);

	return numPRCs;
}

/*
 * Generate a CRC for the given ROM
 *
 * This assumes that the ROM is currently in Palm Order and none of the
 * pointers are valid for our address space.
 */
static int	SetCRC	(ROMPtr	pROM,
					 UInt32	nBytes)
{
	// From Bank_ROM.cpp
	// The checksum is the cumulative checksum of the ROM image before
	// the stored checksum value and the ROM image following the checksum
	// value.  First, calculate the first part.

	//UInt32	chunkSize = offsetof (CardHeaderType, checksumValue);
	UInt32	chunkSize;
	UInt16	checksumValue;

	if (! pROM || ! pROM->pCard || ! pROM->pROM || ! nBytes)
		return(0);
	
	chunkSize = (UInt32)
						&(pROM->pCard->checksumValue) - (UInt32)pROM->pCard;
	checksumValue = Crc16CalcBigBlock (pROM->pROM, chunkSize, 0);

	// Now calculate the second part.
	checksumValue = Crc16CalcBigBlock (
		pROM->pROM + chunkSize + sizeof (pROM->pCard->checksumValue),
		nBytes - chunkSize - sizeof (pROM->pCard->checksumValue),
		checksumValue);

	pROM->pCard->checksumValue			= BYTE_SWAP_16(checksumValue);

	return (1);	//checksumValue);
}

/*
 * Output the given ROM to a file with the given name
 */
int	WriteROM	(ROMPtr		pROM,
				 int		hROM)
{
	int		RC		= 0;
	UInt32	nBytes;

	if ((! pROM) || (hROM < 0))
		return(0);

	nBytes = pROM->pCard->checksumBytes;

	/* Relocate and byte-swap */
	H2P_Translate_ROM(pROM);

	if (nBytes && ! SetCRC(pROM, nBytes))
	{
		fprintf (stderr, "*** Error setting up the ROM CRC\n");
		goto leave;
	}

	/* Write out the ROM image */
	fprintf (stderr, "Writing %ld ROM bytes (%ld used)...",
					 pROM->ROMSize, pROM->CardSize);
	fflush  (stderr);

	if (hROM != fileno(stdout))
	{
		if (lseek(hROM, pROM->File_base, SEEK_SET) != pROM->File_base)
		{
			fprintf (stderr, " ERROR: Cannot seek to %ld\n", pROM->File_base);
			goto leave;
		}
	}

	RC = write(hROM, pROM->pROM, pROM->ROMSize);
	if (RC == pROM->ROMSize)
	{
		fprintf (stderr, " success\n");
	}
	else
	{
		fprintf (stderr, " ERROR %d - %s\n",
		         errno, sys_errlist[errno]);
	}

leave:
	/* Now, put the ROM back to it's incoming state */
	P2H_Translate_ROM(pROM);

	return (RC);
}

/*
 * qsort comparison routine - sort by type.ctor
 */
int	CompareTypeCtor	(const void*	ppDB1,
					 const void*	ppDB2)
{
	DatabaseHdrPtr	pDB1	= *((DatabaseHdrPtr*)ppDB1);
	DatabaseHdrPtr	pDB2	= *((DatabaseHdrPtr*)ppDB2);

	if ((pDB1->type    == pDB2->type) &&
		(pDB1->creator == pDB2->creator))
	{
		// If type and creator are equal, sort by DB name
		return(strncmp(pDB1->name, pDB2->name, sizeof(pDB1->name)));
	}

	return (pDB1->type == pDB2->type ?
					(pDB1->creator - pDB2->creator) :
					(pDB1->type - pDB2->type));
}

/*
 *
 */
static
int	LocateSplashes	(ROMPtr			pROM,
					 RsrcEntryPtr*	ppBootSplash,
					 RsrcEntryPtr*	ppResetSplash)
{
	DatabaseHdrPtr	pDataDB			= NULL;

	if (! pROM || ! ppBootSplash || ! ppResetSplash)
		return 0;

	/*
	 * With PalmOS3.5 (header version 4) we now need to
	 * locate pointers to the splash screens and set them
	 * in the 'NV System Params' area of the storage header.
	 *
	 *
	 * Locate the database which contains the splash screens.
	 * This is a bit complicated because for localized versions of
	 * PalmOS the bitmaps are actually in an overlay...
	 *
	 * First look in the default location:
	 * (type.ctor)	Default location spls.psys
	 *           	Overlay location ovly.psys
	 */
	pDataDB = LocateDB(pROM->pDatabaseList,sysFileTSplash,sysFileCSystem, NULL);
	if (!pDataDB)
		return 0;

	/*
	 * Does this database have an overlay?  If so, locate the
	 * overlay database.
	 */
	{
		OmOverlaySpecType*	pOvly	= LocateOverlayResource(pDataDB);

		if (pOvly)
		{
			pDataDB = LocateDBOverlay(pROM->pDatabaseList, pOvly, NULL);

			if (! pDataDB)
				return 0;
		}
	
	}

	*ppBootSplash	= LocateResource(pDataDB,
						             bsBitmapRsc,sysResIDBitmapSplash);
	*ppResetSplash= LocateResource(pDataDB,
						             bsBitmapRsc,sysResIDBitmapConfirm);

	if (! *ppBootSplash || ! *ppResetSplash)
		return 0;

	return 1;
}

/*
 * Given a ROM, use the incoming PRC name to setup the
 * initial stack, init 1&2 codes, reset vector, and HAL offset
 */
int		SetSystem		(ROMPtr		pROM)
{
	DatabaseHdrPtr	pSysDB			= NULL;
	DatabaseHdrPtr	pHALDB			= NULL;
	
	RsrcEntryPtr	pReset			= NULL;
	RsrcEntryPtr	pInit1			= NULL;
	RsrcEntryPtr	pInit2			= NULL;
	RsrcEntryPtr	pHAL			= NULL;
	RsrcEntryPtr	pBootSplash		= NULL;
	RsrcEntryPtr	pResetSplash	= NULL;
	UInt16			Locale			= 0;

	UInt8*			pEntry			= "ENTRYPOI";
	UInt32			size			= 0;
	UInt32			initStack		= 0;

	if (! pROM)
		return(0);

	/*
	 * (type.ctor)	Small ROM is boot.psys	(sysFileTBoot,   sysFileCSystem)
	 * 				Large ROM is rsrc.psys	(sysFileTSystem, sysFileCSystem)
	 */
	if (pROM->flags & RT_LARGE)
		pSysDB = LocateDB (pROM->pDatabaseList, sysFileTSystem,
						                        sysFileCSystem, NULL);
	else
		pSysDB = LocateDB (pROM->pDatabaseList, sysFileTBoot,
						                        sysFileCSystem, NULL);
	if (! pSysDB)
		return 0;
	
	if (pROM->pCard->hdrVersion >= 0x04)
	{
		/*
		 * (type.ctor)	Small ROM is shal.psys (sysFileTSmallHal,sysFileCSystem)
		 * 				Large ROM is bhal.psys (sysFileTBigHal,  sysFileCSystem)
		 */
		if (pROM->flags & RT_LARGE)
			pHALDB = LocateDB (pROM->pDatabaseList, sysFileTBigHal,
						                            sysFileCSystem, NULL);
		else
			pHALDB =LocateDB (pROM->pDatabaseList, sysFileTSmallHal,
						                           sysFileCSystem, NULL);
		if (! pHALDB)
			return 0;


		if (pROM->flags & RT_LARGE)
		{
			if (! LocateSplashes(pROM, &pBootSplash, &pResetSplash))
				return 0;
		}
	}


	/*
	 * Locate the 3 boot resources in the System database
	 *	1)	'boot'	sysResIDBootReset		(10000)	- resetVector
	 *	2)	'boot'	sysResIDBootInitCode	(10001)	- initCodeOffset1
	 *	3)	'boot'	sysResIDBootInitCode+1	(10002)	- initCodeOffset2
	 *
	 * and the boot resource in the HAL
	 *	4)	'boot'	sysResIDBootHAL			(19000) - halCodeOffset
	 */
	pReset = LocateResource(pSysDB, sysResTBootCode, sysResIDBootReset);
	if (! pReset)
		return(0);

	if (pROM->flags & RT_LARGE)
	{
		pInit1 = LocateResource(pSysDB, sysResTBootCode,sysResIDBootInitCode);
		pInit2 = LocateResource(pSysDB, sysResTBootCode,sysResIDBootInitCode+1);

		if (! pInit1 || ! pInit2)
			return(0);
	}

	if (pROM->pCard->hdrVersion > 3)
	{
		UInt8*	pStack	= (UInt8*)pReset->localChunkID;
		UInt32	addr;

		pHAL = LocateResource(pHALDB, sysResTBootCode, sysResIDBootHAL);

		if (! pHAL)
			return(0);
	
		/*
		 * Now, figure out where the initStack should point in
		 * the 1st System Boot resource...
		 *
		 * This is really a KLUDGE based upon observation:
		 *	Starting at the beginning of the 'sysResIDBootHAL'
		 *	resource in the HAL Database
		 *	(type 'sysFileTBigHal'   ['bhal'] for the Large ROM,
		 *	      'sysFileTSmallHal' ['shal'] for the Small ROM)
		 *	search for 'ENTRYPOI' followed by a 4-byte offset that is
		 *	equivalent to it's own offset.  The initStack is the
		 *	offset of the next byte.
		 */
		size = SizeOfChunk (pROM->pHeapList, pReset->localChunkID);
	
		while (pStack - (UInt8*)pReset->localChunkID < size)
		{
			if (! memcmp (pStack, pEntry, strlen (pEntry)))
				break;

			pStack++;
		}

		if (pStack - (UInt8*)pReset->localChunkID == size)
			return(0);

		pStack += strlen (pEntry);

		/* These 4 bytes need to be the offset of these 4 bytes */
		addr = BYTE_SWAP_32((UInt32)pStack - (UInt32)pROM->pCard);
		memcpy(pStack, &addr, sizeof(addr));


		pStack += 4;

		initStack = (UInt32) pStack - (UInt32) pROM->pCard;

		/* How do we figure out what the Locale should be??? */
		Locale    = 0x0017;
	}
	else
	{
		initStack = pROM->pVersion->card_initStack;
	}

	
	/*
	 * Now, set the properly values in our ROM
	 */
	pROM->pCard->initStack        = initStack;
	pROM->pCard->resetVector      = pReset->localChunkID;

	/* 
	 * This is a hack -- for some reason the reset vector for PalmOS 3.0 and
	 * before has a 1 in the high nibble -- none of the other addresses (heap
	 * offset, big ROM offset, etc) are like this.
	 */
	if (pROM->pVersion && pROM->pVersion->palmOSVer <= 0x300)
		pROM->pCard->resetVector  += 0x10000000;

	if (pInit1)
		pROM->pStore->initCodeOffset1 = pInit1->localChunkID;

	if (pInit2)
		pROM->pStore->initCodeOffset2 = pInit2->localChunkID;

	if (pHAL)
		pROM->pCard->halCodeOffset = pHAL->localChunkID - (UInt32) pROM->pCard;

	if (pBootSplash)
		pROM->pStore->nvParams.splashScreenPtr		=
										(void*)pBootSplash->localChunkID;
			
	if (pResetSplash)
		pROM->pStore->nvParams.hardResetScreenPtr	=
										(void*)pResetSplash->localChunkID;

	if (Locale)
		pROM->pStore->nvParams.localeCountry		= Locale;
	

	/*
	 * Finally, sort the database list by type.ctor.name
	 */
	if (pROM->pVersion->palmOSVer > 0x300)
	{
		qsort((void*)pROM->pDatabaseList->databaseOffset,
	   	    pROM->pDatabaseList->numDatabases,
	   	    sizeof(DatabaseHdrPtr), CompareTypeCtor);
	}
	
	return(1);
}
