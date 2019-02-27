#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rom.h"
#include "os_structs.h"
#include "byte_swap.h"
#include "utils.h"
#include "SystemResources.h"

/*
 * Free a ROM
 */
void	FreeROM	(ROMPtr	pROM)
{
	if (! pROM)
		return;

	if (pROM->pROM)
		free(pROM->pROM);

	if (pROM->pSortedDBList)
		free(pROM->pSortedDBList);

	if (pROM->pContext)
		free(pROM->pContext);

	memset(pROM, 0, sizeof(*pROM));
	free(pROM);
}

void	FreePRC	(PRCPtr	pPRC)
{
	if (! pPRC)
		return;
	
	if (pPRC->pDB)
		free (pPRC->pDB);
	free (pPRC);
}

void	FreePRCList	(PRCPtr*	pPRCList,
					 int		nItems)
{
	int	jdex;

	if (! pPRCList)
		return;

	/* Free all the PRCs in the list */
	for (jdex = 0; jdex < nItems; jdex++)
	{
		if (! pPRCList[jdex])
			continue;

		FreePRC(pPRCList[jdex]);
	}

	free(pPRCList);
}

/*
 * qsort comparison routine
 */
int	CompareAddrs	(const void*	pAddr1,
					 const void*	pAddr2)
{
	UInt32	Addr1	= *(UInt32*)pAddr1;
	UInt32	Addr2	= *(UInt32*)pAddr2;

	if (Addr1 == Addr2)
		return(0);

	return (Addr1 > Addr2 ? 1 : -1);
}

/*
 * Given the first chunk in a heap and an address, locate the
 * chunk which contains the address.
 */
MemChunkHeaderUnionType*
	LocateAddrInChunk	(MemChunkHeaderUnionType*	pChunk,
						 UInt16						version,
						 UInt32						addr)
{
	if (addr <= (UInt32)pChunk)
		return (NULL);
	
	while (pChunk && ! memUChunkIsTerminator(pChunk,version))
	{
		UInt32	nextAddr	= (UInt32)memUChunkNext (pChunk, version);

		if (addr < nextAddr)
		{
			return(pChunk);
		}

		pChunk = (MemChunkHeaderUnionType*)nextAddr;
	}

	return(NULL);
}

/*
 * Given a heap and an address, locate the memory chunk which contains
 * the address.
 */
MemChunkHeaderUnionType*
	LocateAddrInHeap	(MemHeapHeaderUnionType*	pHeap,
						 UInt32						addr)
{
	UInt16						ver		= memUHeapVer(pHeap);
	MemChunkHeaderUnionType*	pChunk;

	/*
	 * Locate the first chunk in this heap
	 * (should immediately follow the heap header)
	 */
	pChunk = memUHeapFirstChunk (pHeap,ver);

	return LocateAddrInChunk(pChunk, memUChunkVer(pHeap), addr);
}

/*
 * Given a heap list and an address, locate the memory chunk which
 * contains the address.
 */
MemChunkHeaderUnionType*	LocateChunk (HeapListPtr		pHeapList,
										 UInt32				addr)
{
	UInt32						idex;
	MemChunkHeaderUnionType*	pChunk		= NULL;
	MemChunkHeaderUnionType*	pChunkHOLD	= NULL;

	if (! pHeapList)
		return(NULL);

	for (idex = 0; idex < pHeapList->numHeaps; idex++)
	{
		MemHeapHeaderUnionType*	pHeap	= (MemHeapHeaderUnionType*)
											(pHeapList->heapOffset[idex]);

		pChunk = LocateAddrInHeap(pHeap, addr);

		if (pChunk)
		{
			if (memUChunkFree(pChunk,memUChunkVer(pHeap)))
			{
				pChunkHOLD = pChunk;
				continue;
			}

			break;
		}
	}

	if (! pChunk && pChunkHOLD)
		pChunk = pChunkHOLD;

	return (pChunk);
}

DatabaseHdrPtr	LocateDBbyName (DatabaseListPtr	pDBList,
								char*			name)
{
	DatabaseHdrPtr pDB;
	UInt16 idex;

	if (! pDBList)
		return NULL;
	
	for (idex = 0; idex < pDBList->numDatabases; idex++)
	{
		pDB = (DatabaseHdrPtr) pDBList->databaseOffset[idex];
		if (! strncmp (pDB->name, name, sizeof (pDB->name)))
			return pDB;
	}
	
	return NULL;
}

/*
 * Returns a database with the given type and creator.  Assumes that the
 * databases are uniquely keyed on (type, creator) passed in.
 * If a starting index is provided (*pStart), start our search at that
 * index.
 *
 * Return the index of the match (if pStart != NULL)
 */
DatabaseHdrPtr	LocateDB	(DatabaseListPtr	pDBList,
							 UInt32				type,
							 UInt32				creator,
							 UInt32*			pStart)
{
	UInt32			idex;
	DatabaseHdrPtr	pDB;

	if (! pDBList)
		return NULL;

	for (idex = (pStart ? *pStart : 0);
	     idex < pDBList->numDatabases; idex++)
	{
		// The type and creator are really strings, but typed as if it were a
		// 32-byte integer, so always remain in Palm order
		pDB = (DatabaseHdrPtr) pDBList->databaseOffset[idex];
		if (pDB->type == type && pDB->creator == creator)
		{
			if (pStart)
				*pStart = idex;
			return pDB;
		}
	}

	if (pStart)
		// Let the caller know that we searched the entire list
		*pStart = pDBList->numDatabases;

	return NULL;
}


static inline int	DoesOverlayMatch	(OmOverlaySpecType*	pOvly,
										 OmOverlaySpecType*	pOvlyCmp)
{
	/*
	 * To match:
	 *	1) Only 1 overlay should have 'omSpecAttrForBase' set
	 *	2) baseChecksum  should match
	 *	3) baseDBType    should match
	 *	4) baseDBCreator should match
	 *	5) numOverlays   should match
	 */
	return (((pOvly->flags    & omSpecAttrForBase) !=
	         (pOvlyCmp->flags & omSpecAttrForBase))          &&
			(pOvly->baseChecksum  == pOvlyCmp->baseChecksum) &&
	        (pOvly->baseDBType    == pOvlyCmp->baseDBType)   &&
	        (pOvly->baseDBCreator == pOvlyCmp->baseDBCreator)&&
	        (pOvly->numOverlays   == pOvlyCmp->numOverlays));
}

/*
 * Given a DB list and an overlay spec, locate the overlay
 * database which will be used as an overlay...
 */
DatabaseHdrPtr	LocateDBOverlay	(DatabaseListPtr	pDBList,
								 OmOverlaySpecType*	pOvly,
								 UInt32*			pStart)
{
	DatabaseHdrPtr	pDataDB	= NULL;
	UInt32			idex;

	idex = pStart ? *pStart : 0;

	if (! pDBList || ! pOvly)
		return NULL;

	for (idex = pStart ? *pStart : 0; idex < pDBList->numDatabases; idex++)
	{
		OmOverlaySpecType*	pCurOvly;

		pDataDB  = (DatabaseHdrPtr)pDBList->databaseOffset[idex];
		pCurOvly = LocateOverlayResource(pDataDB);

		if (pCurOvly && DoesOverlayMatch(pOvly, pCurOvly))
		{
			/*
			 * Yes -- this database seems to contain the
			 *        desired database.
			 */
			break;
		}
		pDataDB = NULL;
	}

	if (pStart)
		*pStart = idex;
	return(pDataDB);
}

/*
 * Locate a resource with given type and id
 */
RsrcEntryPtr	LocateResource	(DatabaseHdrPtr	pDB,
								 UInt32			Type,
								 UInt16			ID)
{
	int				idex;
	RsrcEntryPtr	pRes;

	if (! IsResource(pDB))
		return(NULL);

	pRes = (RsrcEntryPtr)&(pDB->recordList.firstEntry);
	for (idex = 0; idex < pDB->recordList.numRecords; idex++)
	{
		// The type is really a string, but typed as if it were a 32-byte
		// integer, so it always remains in Palm order
		if ((pRes[idex].type == Type) && (pRes[idex].id == ID))
			return(pRes + idex);
	}

	return (NULL);
}

/* If pROM is not NULL, use the heap information within to
 * figure out the size of the recordList's idexth record's contents.
 * If pROM is NULL,  assume the records are contiguously layed out 
 * and stored in order to figure out their size.  Note that this does
 * not work for the last record, so we return 0.
 */
int SizeOfRecordContents	(ROMPtr			pROM,
							 UInt32			highAddr,
						 	 RecordListPtr	pRecordList,
							 UInt16			lType,
							 UInt16			idex)
{
		UInt32 addr;

		if (! pRecordList || idex >= pRecordList->numRecords)
			return 0;
		
		switch (lType)
		{
		case RL_RESOURCES:
			addr = ((RsrcEntryPtr)&pRecordList->firstEntry)[idex].localChunkID;
			break;
		case RL_RECORDS:
			addr = ((RecordEntryPtr)&pRecordList->firstEntry)[idex].localChunkID;
			break;
		default:
			fprintf (stderr, "SizeOfRecordContents called on unknown list type.\n");
			return 0;
			break;
		}
		
		if (pROM && pROM->pHeapList)
		{
			return SizeOfChunk(pROM->pHeapList, addr);
		}
		else if (idex < pRecordList->numRecords - 1)
		{
			UInt32 nextaddr;
			switch (lType)
			{
			case RL_RESOURCES:
				nextaddr = 
					((RsrcEntryPtr)&pRecordList->firstEntry)[idex+1].localChunkID;
				break;
			case RL_RECORDS:
				nextaddr = 
					((RecordEntryPtr)&pRecordList->firstEntry)[idex+1].localChunkID;
				break;
			default:
				fprintf (stderr, "SizeOfRecordContents called on unknown list type.\n");
				return 0;
				break;
			}
			return nextaddr - addr;
		}
		else
		{
			return highAddr - addr;
		}
}

/* Compute the size of the DBheader and all it's records */
int DBTotalSize (ROMPtr			pROM,
				 DatabaseHdrPtr	pDB)
{
	int Total = 0;
	UInt16 idex;
	
	if (! pROM || !pROM->pHeapList || ! pDB)
		return 0;

	Total += SizeOfChunk (pROM->pHeapList, (UInt32)pDB);

	if (pDB->appInfoID)
		Total += SizeOfChunk (pROM->pHeapList, pDB->appInfoID);
	if (pDB->sortInfoID)
		Total += SizeOfChunk (pROM->pHeapList, pDB->sortInfoID);
			
	for (idex = 0; idex < pDB->recordList.numRecords; idex++)
	{
		Total += SizeOfRecordContents (pROM, 0, &pDB->recordList, 
									   IsResource (pDB) ? RL_RESOURCES : RL_RECORDS,
									   idex);
	}
	return Total;
}

/*
 * Return the first overlay resource in the given database
 * (actually it only checks the first resource period)
 */
OmOverlaySpecType*	LocateOverlayResource	(DatabaseHdrPtr	pDatabase)
{
	OmOverlaySpecType*	pOvly	= NULL;

	if (pDatabase && IsResource(pDatabase))
	{
		RsrcEntryType*	pRsrc	= (RsrcEntryType*)&(pDatabase->recordList.firstEntry);

		if (pRsrc->type == sysFileTOverlay)
		{
			pOvly = (OmOverlaySpecType*)pRsrc->localChunkID;
		}
	}

	return (pOvly);
}

/* Exact value of "Jan 1, 1970 0:00:00 GMT" - "Jan 1, 1904 0:00:00 GMT" */
#define PILOT_TIME_DELTA (unsigned)(2082844800)

time_t	pilot_time_to_unix_time	(UInt32	raw_time)
{
    return (time_t) (raw_time - PILOT_TIME_DELTA);
}

UInt32	unix_time_to_pilot_time	(time_t	time)
{
    return (UInt32) ((UInt32) time + PILOT_TIME_DELTA);
}

char*	pilot_time_str	(UInt32	raw_time)
{
	static char*	pTimeStr	= NULL;
	time_t			time		= pilot_time_to_unix_time(raw_time);

	pTimeStr = ctime(&time);
	pTimeStr[strlen(pTimeStr)-1] = '\0';

	return pTimeStr;
}

/*
 * Give a 'numEntries' list of strings in the form:
 * 	'type.ctor' where both 'type' and 'ctor' are 4-bytes
 *
 * convert them to a 'numEntries' list of TypeCtorType.
 *
 * This routine allocate the new TypeCtorType array.
 * The caller is responsible for freeing it.
 */
TypeCtorPtr StrList2TypeCtorList		(char*			StrList[],
										 UInt32			numEntries)
{
	TypeCtorPtr	pTCList	= NULL;
	UInt32		size	= numEntries * sizeof(TypeCtorType);
	UInt32		idex;

	if (! StrList)
		return NULL;

	pTCList = (TypeCtorPtr)malloc(size);
	if (! pTCList)
		return NULL;
	memset(pTCList, 0, size);

	for (idex = 0; idex < numEntries; idex++)
	{
		char*	pDot	= NULL;
		char*	pType	= NULL;

		// Must have a non-null string with at LEAST 9 characters
		// (4 for type, one for '.', and 4 for ctor)
		if ((! StrList[idex]) || (strlen(StrList[idex]) < 9))
			goto error;

		// First, interpret the type
		pType = StrList[idex];
		pDot  = strchr(pType, '.');
		if (! pDot)
			goto error;

		memcpy(&(pTCList[idex].type), pType,  sizeof(pTCList[idex].type));
		memcpy(&(pTCList[idex].ctor), pDot+1, sizeof(pTCList[idex].ctor));

		pTCList[idex].type = BYTE_SWAP_32(pTCList[idex].type);
		pTCList[idex].ctor = BYTE_SWAP_32(pTCList[idex].ctor);

		//fprintf (stdout, "%2ld: '%s' ==> '%04lX'.'%04lX'\n",
		//		 idex, StrList[idex], pTCList[idex].type, pTCList[idex].ctor);
	}

	return pTCList;

error:
	if (pTCList)
		free(pTCList);

	return NULL;
}

/*
 * Is the given 'type' and 'ctor' in the TypeCtorType array?
 */
UInt8	IsInTypeCtorList	(UInt32			type,
							 UInt32			ctor,
							 TypeCtorPtr	TypeCtorList,
							 UInt32			numEntries)
{
	UInt32	idex;
	UInt8	bFound	= 0;

	if ((! TypeCtorList) || (numEntries < 1))
		// Default to TRUE
		return 1;

	// Is this database listed in our TypeCtorList?
	for (idex = 0; idex < numEntries; idex++)
	{
		if ((type == TypeCtorList[idex].type) &&
		    (ctor == TypeCtorList[idex].ctor))
		{
			bFound = 1;
			break;
		}
	}

	return bFound;
}
