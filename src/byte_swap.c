#include <stdio.h>
#include "rom.h"
#include "os_structs.h"
#include "byte_swap.h"

/****************************************************************************
 ****************************************************************************
 *** Low-level byte-swap routines
 ****************************************************************************
 ****************************************************************************/

void	BS_GenericList32 (void* pArray, UInt16 numEntries, UInt16 stepSize)
{
	UInt16 idex;

	if (! pArray)
		return;

	for (idex = 0; idex < numEntries; idex++)
	{
		*(UInt32*)pArray = (UInt32) BYTE_SWAP_32 (*(UInt32*)pArray);
		pArray = (UInt8*)pArray + stepSize;
	}
}

void	BS_GenericList16 (void* pArray, UInt16 numEntries, UInt16 stepSize)
{
	UInt16 idex;

	if (! pArray)
		return;

	for (idex = 0; idex < numEntries; idex++)
	{
		*(UInt16*)pArray = (UInt16) BYTE_SWAP_16 (*(UInt16*)pArray);
		pArray = (UInt8*)pArray + stepSize;
	}
}

void	BS_SysNVParams	(SysNVParamsPtr	pParams)
{
	if (! pParams)
		return;

	pParams->rtcHours = BYTE_SWAP_32(pParams->rtcHours);
	pParams->rtcHourMinSecCopy = BYTE_SWAP_32(pParams->rtcHourMinSecCopy);
	pParams->splashScreenPtr = (void*)BYTE_SWAP_32(pParams->splashScreenPtr);
	pParams->hardResetScreenPtr = (void*)BYTE_SWAP_32(pParams->hardResetScreenPtr);
	pParams->localeLanguage = BYTE_SWAP_16(pParams->localeLanguage);
	pParams->localeCountry = BYTE_SWAP_16(pParams->localeCountry);
	pParams->sysNVOEMStorage1 = BYTE_SWAP_32(pParams->sysNVOEMStorage1);
	pParams->sysNVOEMStorage2 = BYTE_SWAP_32(pParams->sysNVOEMStorage2);
}

void	BS_MemMstrPtrTableHeader	(MemMstrPtrTablePtr	pTable)
{
	if (! pTable)
		return;

	pTable->numEntries = BYTE_SWAP_16(pTable->numEntries);
	pTable->nextTblOffset = BYTE_SWAP_32(pTable->nextTblOffset);

}

void	BS_MemMstrPtrTableList	(MemMstrPtrTablePtr pTable, UInt16 numEntries)
{
	BS_GenericList32 ((void*)(pTable+1), numEntries, sizeof (UInt32*));
}

void	BS_Mem1MstrPtrTableHeader	(Mem1MstrPtrTablePtr	pTable)
{
	if (! pTable)
		return;

	pTable->numEntries = BYTE_SWAP_16(pTable->numEntries);
	pTable->nextTblOffset = BYTE_SWAP_16(pTable->nextTblOffset);

}

void	BS_Mem1MstrPtrTableList	(Mem1MstrPtrTablePtr pTable, UInt16 numEntries)
{
	BS_GenericList32 ((void*)(pTable+1), numEntries, sizeof (UInt32));
}

void	BS_MemChunkHeader	(MemChunkHeaderPtr	pChunk)
{
	typedef struct	{	UInt32	one;
						UInt32	two;
					}	swap;

	if (! pChunk)
		return;

	((swap*)pChunk)->one =
			BYTE_SWAP_32(((swap*)pChunk)->one);
	((swap*)pChunk)->two =
			BYTE_SWAP_32(((swap*)pChunk)->two);
}

void	BS_Mem1ChunkHeader	(Mem1ChunkHeaderPtr	pChunk)
{
	if (! pChunk)
		return;

	pChunk->size = BYTE_SWAP_16(pChunk->size);
	pChunk->hOffset = BYTE_SWAP_16(pChunk->hOffset);
}

void	BS_MemChunk	(MemChunkHeaderUnionType* pChunk, UInt16 version)
{
	if (version == 0x01)
		BS_Mem1ChunkHeader (&pChunk->header.ver1);
	else
		BS_MemChunkHeader (&pChunk->header.ver2);
}

/* Swaps only the MemHeapHeader, not the MemMstrPtrTable or its header */
void	BS_HeapHeader		(MemHeapHeaderUnionType*	pHeader,
							 UInt16						version)
{
	if (! pHeader)
		return;

	pHeader->header.ver3.flags = BYTE_SWAP_16(pHeader->header.ver3.flags);
	switch (version)
	{
	case 1:
		pHeader->header.ver1.size = BYTE_SWAP_16(pHeader->header.ver1.size);
		break;
	case 2:
		pHeader->header.ver2.size = BYTE_SWAP_32(pHeader->header.ver2.size);
		break;
	case 3:
	case 4:
		pHeader->header.ver3.size = BYTE_SWAP_32(pHeader->header.ver3.size);
		pHeader->header.ver3.firstFreeChunkOffset = 
				BYTE_SWAP_32(pHeader->header.ver3.firstFreeChunkOffset);
	}
}

void	BS_CardHeader	(CardHeaderPtr	pCard)
{
	if (! pCard)
		return;

	pCard->initStack			= BYTE_SWAP_32(pCard->initStack);
	pCard->resetVector			= BYTE_SWAP_32(pCard->resetVector);
	pCard->signature			= BYTE_SWAP_32(pCard->signature);
	pCard->hdrVersion			= BYTE_SWAP_16(pCard->hdrVersion);
	pCard->flags				= BYTE_SWAP_16(pCard->flags);
	pCard->version				= BYTE_SWAP_16(pCard->version);
	pCard->creationDate			= BYTE_SWAP_32(pCard->creationDate);
	pCard->numRAMBlocks			= BYTE_SWAP_16(pCard->numRAMBlocks);
	pCard->blockListOffset		= BYTE_SWAP_32(pCard->blockListOffset);
	pCard->readWriteParmsOffset	= BYTE_SWAP_32(pCard->readWriteParmsOffset);
	pCard->readWriteParmsSize	= BYTE_SWAP_32(pCard->readWriteParmsSize);
	pCard->readOnlyParmsOffset	= BYTE_SWAP_32(pCard->readOnlyParmsOffset);
	pCard->bigROMOffset			= BYTE_SWAP_32(pCard->bigROMOffset);
	pCard->checksumBytes		= BYTE_SWAP_32(pCard->checksumBytes);
	pCard->checksumValue		= BYTE_SWAP_16(pCard->checksumValue);
	pCard->readWriteWorkingOffset= BYTE_SWAP_32(pCard->readWriteWorkingOffset);
	pCard->readWriteWorkingSize	= BYTE_SWAP_32(pCard->readWriteWorkingSize);
	pCard->halCodeOffset		= BYTE_SWAP_32(pCard->halCodeOffset);
}

void	BS_StorageHeader	(StorageHeaderPtr	pStore)
{
	if (! pStore)
		return;

	pStore->signature			= BYTE_SWAP_32(pStore->signature);
	pStore->version				= BYTE_SWAP_16(pStore->version);
	pStore->flags				= BYTE_SWAP_16(pStore->flags);
	pStore->creationDate		= BYTE_SWAP_32(pStore->creationDate);
	pStore->backupDate			= BYTE_SWAP_32(pStore->backupDate);
	pStore->heapListOffset		= BYTE_SWAP_32(pStore->heapListOffset);
	pStore->initCodeOffset1		= BYTE_SWAP_32(pStore->initCodeOffset1);
	pStore->initCodeOffset2		= BYTE_SWAP_32(pStore->initCodeOffset2);
	pStore->databaseDirID		= BYTE_SWAP_32(pStore->databaseDirID);
	pStore->rsvSpace			= BYTE_SWAP_32(pStore->rsvSpace);
	pStore->dynHeapSpace		= BYTE_SWAP_32(pStore->dynHeapSpace);
	pStore->firstRAMBlockSize	= BYTE_SWAP_32(pStore->firstRAMBlockSize);
	BS_SysNVParams(&(pStore->nvParams));
	pStore->crc					= BYTE_SWAP_32(pStore->crc);
}

void	BS_HeapListHeader	(HeapListPtr	pHeapList)
{
	if (! pHeapList)
		return;

	pHeapList->numHeaps = BYTE_SWAP_16(pHeapList->numHeaps);
}

void	BS_HeapListList		(HeapListPtr	pHeapList, UInt16 nHeaps)
{
	BS_GenericList32 ((void*)pHeapList->heapOffset, nHeaps, sizeof (UInt32));
}

void	BS_RecordListHeader	(RecordListPtr	pRecordList)
{
	UInt8*	pItem;

	if (! pRecordList)
		return;

	pItem	= (UInt8*)(&(pRecordList->firstEntry));

	pRecordList->nextRecordListID = BYTE_SWAP_32(pRecordList->nextRecordListID);
	pRecordList->numRecords		  = BYTE_SWAP_16(pRecordList->numRecords);

}

/*
 * Byte swap a record list.  This may be a list of resources or records
 * depending upon 'lType':
 * 	RL_RESOURCES (0)	- Resources
 * 	RL_RECORDS   (1)	- Records
 */
void	BS_RecordListList	(RecordListPtr pRecordList, UInt16 nRecords, UInt16 lType)
{
	RsrcEntryPtr	pRsrc	= (RsrcEntryPtr) &pRecordList->firstEntry;
	RecordEntryPtr	pRec	= (RecordEntryPtr) &pRecordList->firstEntry;

	if (! pRecordList)
		return;

	switch (lType)
	{
	case RL_RESOURCES:		// Resource
		BS_GenericList32 ((void*) &pRsrc->type, nRecords, sizeof (*pRsrc));
		BS_GenericList16 ((void*) &pRsrc->id, nRecords, sizeof (*pRsrc));
		BS_GenericList32 ((void*) &pRsrc->localChunkID, nRecords,
		                  sizeof (*pRsrc));
		break;
	case RL_RECORDS:		// Record
		BS_GenericList32 ((void*) &pRec->localChunkID, nRecords,
		                  sizeof (*pRec));
		break;
	default:
		fprintf (stderr, "*** BS_RecordListList: record list of invalid type %d\n",
		                  lType);
		break;
	}
}

/*
 * Byte swap a database list.
 */
void	BS_DatabaseListHeader	(DatabaseListPtr	pList)
{
	if (! pList)
		return;

	pList->nextRecordListID = BYTE_SWAP_32(pList->nextRecordListID);
	pList->numDatabases     = BYTE_SWAP_16(pList->numDatabases);

}

void	BS_DatabaseListList		(DatabaseListPtr	pList,
								 UInt16				numDatabases)
{
	BS_GenericList32 ((void*)pList->databaseOffset, numDatabases, 
					sizeof (*pList->databaseOffset));
}

/*
 * Does not swap the record list
 */
void	BS_DatabaseHdrHeader	(DatabaseHdrPtr	pDatabase)
{
	if (! pDatabase)
		return;

	pDatabase->attributes			= BYTE_SWAP_16(pDatabase->attributes);
	pDatabase->version				= BYTE_SWAP_16(pDatabase->version);
	pDatabase->creationDate			= BYTE_SWAP_32(pDatabase->creationDate);
	pDatabase->modificationDate		= BYTE_SWAP_32(pDatabase->modificationDate);
	pDatabase->lastBackupDate		= BYTE_SWAP_32(pDatabase->lastBackupDate);
	pDatabase->modificationNumber	= BYTE_SWAP_32(pDatabase->modificationNumber);
	pDatabase->appInfoID			= BYTE_SWAP_32(pDatabase->appInfoID);
	pDatabase->sortInfoID			= BYTE_SWAP_32(pDatabase->sortInfoID);
	pDatabase->type					= BYTE_SWAP_32(pDatabase->type);
	pDatabase->creator				= BYTE_SWAP_32(pDatabase->creator);
	pDatabase->uniqueIDSeed			= BYTE_SWAP_32(pDatabase->uniqueIDSeed);
}

void	BS_OverlaySpecList		(OmOverlaySpecType*	pOvly,
								 UInt16				nOvlys)
{
	BS_GenericList16 ((void*) &pOvly->overlays[0].overlayType, nOvlys,
	                          sizeof (pOvly->overlays[0]));
	BS_GenericList16 ((void*) &pOvly->overlays[0].rscID, nOvlys,
	                          sizeof (pOvly->overlays[0]));
	BS_GenericList32 ((void*) &pOvly->overlays[0].rscType, nOvlys,
	                          sizeof (pOvly->overlays[0]));
	BS_GenericList32 ((void*) &pOvly->overlays[0].rscLength, nOvlys,
	                          sizeof (pOvly->overlays[0]));
	BS_GenericList32 ((void*) &pOvly->overlays[0].rscChecksum, nOvlys,
	                          sizeof (pOvly->overlays[0]));
}

void	BS_Locale	(OmLocaleType*	pLocale)
{
	pLocale->language	= BYTE_SWAP_16(pLocale->language);
	pLocale->country	= BYTE_SWAP_16(pLocale->country);
}

void	BS_OverlaySpecHeader	(OmOverlaySpecType*	pOvly)
{
	if (! pOvly)
		return;

	pOvly->version			= BYTE_SWAP_16(pOvly->version);
	pOvly->flags			= BYTE_SWAP_32(pOvly->flags);
	pOvly->baseChecksum		= BYTE_SWAP_32(pOvly->baseChecksum);

	BS_Locale(&pOvly->targetLocale);

	pOvly->baseDBType		= BYTE_SWAP_32(pOvly->baseDBType);
	pOvly->baseDBCreator	= BYTE_SWAP_32(pOvly->baseDBCreator);

	pOvly->baseDBCreateDate	= BYTE_SWAP_32(pOvly->baseDBCreateDate);
	pOvly->baseDBModDate	= BYTE_SWAP_32(pOvly->baseDBModDate);
	pOvly->numOverlays		= BYTE_SWAP_16(pOvly->numOverlays);
}

/****************************************************************************
 ****************************************************************************
 *** High-level byte-swap routines
 ****************************************************************************
 ****************************************************************************/
void	H2P_MemMstrPtrTable	(MemMstrPtrTablePtr	pTable)
{
	if (! pTable)
		return;

	BS_MemMstrPtrTableList		(pTable, pTable->numEntries);
	BS_MemMstrPtrTableHeader	(pTable);

}

void 	P2H_MemMstrPtrTable	(MemMstrPtrTablePtr	pTable)
{
	if (! pTable)
		return;

	BS_MemMstrPtrTableHeader	(pTable);
	BS_MemMstrPtrTableList		(pTable, pTable->numEntries);
}

void 	H2P_Mem1MstrPtrTable	(Mem1MstrPtrTablePtr	pTable)
{
	if (! pTable)
		return;

	BS_Mem1MstrPtrTableList		(pTable, pTable->numEntries);
	BS_Mem1MstrPtrTableHeader	(pTable);

}

void 	P2H_Mem1MstrPtrTable	(Mem1MstrPtrTablePtr	pTable)
{
	if (! pTable)
		return;

	BS_Mem1MstrPtrTableHeader	(pTable);
	BS_Mem1MstrPtrTableList		(pTable, pTable->numEntries);
}

void	H2P_MemChunk		(MemChunkHeaderUnionType*	pChunk,
							 UInt16						version)
{
	BS_MemChunk (pChunk, version);
}

void	P2H_MemChunk		(MemChunkHeaderUnionType*	pChunk,
							 UInt16						version)
{
	BS_MemChunk (pChunk, version);
}

void	H2P_HeapHeader		(MemHeapHeaderUnionType*	pHeader)
{
	UInt16 version;

	if (! pHeader)
		return;

	version = memUHeapVer (pHeader);
	BS_HeapHeader (pHeader, version);
	switch (version)
	{
	case 4:
	case 3:
		H2P_MemMstrPtrTable (&(pHeader->header.ver3.mstrPtrTbl));
		break;
	case 2:
		H2P_MemMstrPtrTable (&(pHeader->header.ver2.mstrPtrTbl));
		break;
	case 1:
		H2P_Mem1MstrPtrTable (&(pHeader->header.ver1.mstrPtrTbl));
		break;
	default:
		fprintf (stderr, "*** H2P_HeapHeader: invalid version %d\n", version);
		break;
	}
}

void	P2H_HeapHeader		(MemHeapHeaderUnionType*	pHeader)
{
	UInt16 version;
	if (! pHeader)
		return;

	// Too lazy to extract the version when the flags are in the wrong order ...
	pHeader->header.ver3.flags = BYTE_SWAP_16 (pHeader->header.ver3.flags);
	version = memUHeapVer (pHeader);
	pHeader->header.ver3.flags = BYTE_SWAP_16 (pHeader->header.ver3.flags);

	BS_HeapHeader (pHeader, version);
	switch (version)
	{
	case 4:
	case 3:
		P2H_MemMstrPtrTable (&(pHeader->header.ver3.mstrPtrTbl));
		break;
	case 2:
		P2H_MemMstrPtrTable (&(pHeader->header.ver2.mstrPtrTbl));
		break;
	case 1:
		P2H_Mem1MstrPtrTable (&(pHeader->header.ver1.mstrPtrTbl));
		break;
	default:
		fprintf (stderr, "*** P2H_HeapHeader: invalid version %d\n", version);
		break;
	}
}

void	H2P_CardHeader		(CardHeaderPtr	pCard)
{
	BS_CardHeader (pCard);
}

void	P2H_CardHeader		(CardHeaderPtr	pCard)
{
	BS_CardHeader (pCard);
}

void	H2P_StorageHeader	(StorageHeaderPtr	pStore)
{
	BS_StorageHeader (pStore);
}

void	P2H_StorageHeader	(StorageHeaderPtr	pStore)
{
	BS_StorageHeader (pStore);
}

void	H2P_HeapList		(HeapListPtr pHeapList)
{
	BS_HeapListList		(pHeapList, pHeapList->numHeaps);
	BS_HeapListHeader	(pHeapList);
}

void	P2H_HeapList		(HeapListPtr pHeapList)
{
	BS_HeapListHeader	(pHeapList);
	BS_HeapListList		(pHeapList, pHeapList->numHeaps);
}

void	H2P_RecordList		(RecordListPtr pRecord, UInt16 lType)
{
	BS_RecordListList	(pRecord, pRecord->numRecords, lType);
	BS_RecordListHeader	(pRecord);
}

void	P2H_RecordList		(RecordListPtr pRecord, UInt16 lType)
{
	BS_RecordListHeader	(pRecord);
	BS_RecordListList	(pRecord, pRecord->numRecords, lType);
}

void	H2P_DatabaseList		(DatabaseListPtr pDatabaseList)
{
	BS_DatabaseListList		(pDatabaseList, pDatabaseList->numDatabases);
	BS_DatabaseListHeader	(pDatabaseList);
}

void	P2H_DatabaseList		(DatabaseListPtr pDatabaseList)
{
	BS_DatabaseListHeader	(pDatabaseList);
	BS_DatabaseListList		(pDatabaseList, pDatabaseList->numDatabases);
}

void	H2P_DatabaseHdr		(DatabaseHdrPtr	pDatabase)
{
	UInt16	lType;

	if (! pDatabase)
		return;

	lType = IsResource (pDatabase) ? RL_RESOURCES : RL_RECORDS;
	H2P_RecordList	(&pDatabase->recordList, lType);
	BS_DatabaseHdrHeader	(pDatabase);
}

void	P2H_DatabaseHdr		(DatabaseHdrPtr	pDatabase)
{
	UInt16	lType;
	BS_DatabaseHdrHeader	(pDatabase);
	lType = IsResource (pDatabase) ? RL_RESOURCES : RL_RECORDS;
	P2H_RecordList	(&pDatabase->recordList, lType);
}

void	H2P_OverlaySpec	(OmOverlaySpecType*	pOvly)
{
	if (! pOvly)
		return;

	BS_OverlaySpecList(pOvly, pOvly->numOverlays);
	BS_OverlaySpecHeader(pOvly);
}

void	P2H_OverlaySpec	(OmOverlaySpecType*	pOvly)
{
	if (! pOvly)
		return;

	BS_OverlaySpecHeader(pOvly);
	BS_OverlaySpecList(pOvly, pOvly->numOverlays);
}
