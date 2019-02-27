#include "rom.h"
#include "os_structs.h"
#include "relocate.h"
#include "byte_swap.h"
/**************************************************************************
 **************************************************************************
 ** All Relocate routines assume that the objects are in host order -- all
 ** they do is make adjustments to the offsets of the pointers.  No Relocate
 ** routines follow pointers.
 **************************************************************************
 **************************************************************************/

void	Relocate_GenericList	(void**		pList,
								 UInt16		numEntries,
								 UInt16		stepSize,
								 UInt32		oldBase,
								 UInt32		newBase)
{
	UInt16	idex;
	UInt32*	pVal;

	for (idex = 0; idex < numEntries; idex++)
	{
		pVal   = (UInt32*)pList;

		if (*pVal)
			*pVal  = *pVal - oldBase + newBase;

		pList  = (void**)((UInt8*) pList + stepSize);
	}
}

void	Relocate_SysNVParams	(SysNVParamsPtr	pParams,
								 UInt32			oldBase,
								 UInt32			newBase)
{
	if (! pParams)
		return;

	if (pParams->splashScreenPtr != 0)
		pParams->splashScreenPtr = pParams->splashScreenPtr + newBase - oldBase;

	if (pParams->hardResetScreenPtr != 0)
		pParams->hardResetScreenPtr = pParams->hardResetScreenPtr + newBase - oldBase;

}

void	Relocate_MemMstrPtrTable	(MemMstrPtrTablePtr	pTable,
									 UInt32				oldBase,
									 UInt32				newBase)
{
	if (! pTable)
		return;

	if (pTable->nextTblOffset != 0)
		pTable->nextTblOffset = pTable->nextTblOffset + newBase - oldBase;

	Relocate_GenericList ((void**)(pTable+1), pTable->numEntries,
	                       sizeof (UInt32), oldBase, newBase);
}

void	Relocate_Mem1MstrPtrTable	(Mem1MstrPtrTablePtr	pTable,
									 UInt32					oldBase,
									 UInt32					newBase)
{
	if (! pTable)
		return;

	if (pTable->nextTblOffset != 0)
		pTable->nextTblOffset = pTable->nextTblOffset + newBase - oldBase;

	Relocate_GenericList ((void**)(pTable+1), pTable->numEntries,
	                      sizeof (UInt32), oldBase, newBase);
}

void	Relocate_MemChunkHeader	(MemChunkHeaderPtr	pChunk,
								 UInt32				oldBase,
								 UInt32				newBase)
{
	if (! pChunk)
		return;
}

void	Relocate_Mem1ChunkHeader	(Mem1ChunkHeaderPtr	pChunk,
									 UInt32				oldBase,
									 UInt32				newBase)
{
	if (! pChunk)
		return;
}

void	Relocate_MemChunk		(MemChunkHeaderUnionType*	pChunk,
								 UInt16						version,
								 UInt32						oldBase,
								 UInt32						newBase)
{
	if (version == 0x01)
		Relocate_Mem1ChunkHeader (&pChunk->header.ver1, oldBase, newBase);
	else
		Relocate_MemChunkHeader (&pChunk->header.ver2, oldBase, newBase);
}

void	Relocate_CardHeader	(CardHeaderPtr	pCard,
							 UInt32			oldBase,
							 UInt32			newBase)
{
	if (! pCard)
		return;

	if (pCard->resetVector != 0)
		pCard->resetVector = pCard->resetVector + newBase - oldBase;
	if (pCard->blockListOffset != 0)
		pCard->blockListOffset = pCard->blockListOffset + newBase - oldBase;
	if (pCard->readWriteParmsOffset != 0)
		pCard->readWriteParmsOffset = pCard->readWriteParmsOffset
				+ newBase - oldBase;
	if (pCard->readOnlyParmsOffset != 0)
		pCard->readOnlyParmsOffset = pCard->readOnlyParmsOffset
				+ newBase - oldBase;
	if (pCard->bigROMOffset != 0)
		pCard->bigROMOffset = pCard->bigROMOffset + newBase - oldBase;
	if (pCard->readWriteWorkingOffset != 0)
		pCard->readWriteWorkingOffset = pCard->readWriteWorkingOffset
				+ newBase - oldBase;
}

void	Relocate_StorageHeader	(StorageHeaderPtr	pStore,
								 UInt32				oldBase,
								 UInt32				newBase)
{
	if (! pStore)
		return;

	if (pStore->heapListOffset != 0)
		pStore->heapListOffset = pStore->heapListOffset + newBase - oldBase;
	if (pStore->initCodeOffset1 != 0)
		pStore->initCodeOffset1 = pStore->initCodeOffset1 + newBase - oldBase;
	if (pStore->initCodeOffset2 != 0)
		pStore->initCodeOffset2 = pStore->initCodeOffset2 + newBase - oldBase;
	if (pStore->databaseDirID != 0)
		pStore->databaseDirID = pStore->databaseDirID + newBase - oldBase;
	if (pStore->rsvSpace != 0)
		pStore->rsvSpace = pStore->rsvSpace + newBase - oldBase;
	if (pStore->dynHeapSpace != 0)
		pStore->dynHeapSpace = pStore->dynHeapSpace + newBase - oldBase;
	if (pStore->firstRAMBlockSize != 0)
		pStore->firstRAMBlockSize = pStore->firstRAMBlockSize
				+ newBase - oldBase;
	Relocate_SysNVParams(&(pStore->nvParams), oldBase, newBase);
}

void	Relocate_HeapHeader		(MemHeapHeaderUnionType*	pHeader,
								 UInt32						oldBase,
								 UInt32						newBase)
{
	UInt16	ver;

	if (! pHeader)
		return;

	ver = memUHeapVer (pHeader);
	switch (ver)
	{
	case 1:
		Relocate_Mem1MstrPtrTable(&(pHeader->header.ver1.mstrPtrTbl),
		                          oldBase, newBase);
		break;

	case 2:
		Relocate_MemMstrPtrTable(&(pHeader->header.ver2.mstrPtrTbl),
		                         oldBase, newBase);
		break;

	case 3:
	case 4:
		Relocate_MemMstrPtrTable(&(pHeader->header.ver3.mstrPtrTbl),
		                         oldBase, newBase);
	}
}

void	Relocate_HeapList	(HeapListPtr	pHeapList,
							 UInt32			oldBase,
							 UInt32			newBase)
{
	if (! pHeapList)
		return;

	Relocate_GenericList ((void**)pHeapList->heapOffset, pHeapList->numHeaps,
							sizeof(pHeapList->heapOffset[0]), oldBase, newBase);
}

/*
 * Relocate a record list.  This may be a list of resources, records,
 * or database headers depending upon 'lType':
 * 	RL_RESOURCES (0)	- Resources
 * 	RL_RECORDS   (1)	- Records
 */
void	Relocate_RecordList	(RecordListPtr	pRecordList,
							 UInt32			oldBase,
							 UInt32			newBase,
							 UInt16			lType)
{
	RsrcEntryPtr	pRsrc;
	RecordEntryPtr	pRec;

	if (! pRecordList)
		return;

	pRsrc	= (RsrcEntryPtr)	&pRecordList->firstEntry;
	pRec	= (RecordEntryPtr)	&pRecordList->firstEntry;

	if (pRecordList->nextRecordListID != 0)
		pRecordList->nextRecordListID = pRecordList->nextRecordListID
				+ newBase - oldBase;

	if (lType == RL_RESOURCES)
		Relocate_GenericList ((void**)&(pRsrc->localChunkID),
		                        pRecordList->numRecords, 
								sizeof(*pRsrc), oldBase, newBase);
	else
		Relocate_GenericList ((void**)&pRec->localChunkID,
		                        pRecordList->numRecords, 
								sizeof(*pRec), oldBase, newBase);
}

/*
 * Relocate a database list.  Doesn't relocate the database headers pointed
 * to by the list.
 */
void	Relocate_DatabaseList	(DatabaseListPtr	pList,
								 UInt32				oldBase,
								 UInt32				newBase)
{
	if (! pList)
		return;

	if (pList->nextRecordListID != 0)
		pList->nextRecordListID = pList->nextRecordListID
				+ newBase - oldBase;

	Relocate_GenericList ((void**)pList->databaseOffset,pList->numDatabases,
							sizeof(pList->databaseOffset[0]),oldBase, newBase);
}

void	Relocate_DatabaseHdr	(DatabaseHdrPtr	pDatabase,
								 UInt32			oldBase,
								 UInt32			newBase)
{
	if (! pDatabase)
		return;

	if (pDatabase->appInfoID != 0)
		pDatabase->appInfoID = pDatabase->appInfoID + newBase - oldBase;
	if (pDatabase->sortInfoID != 0)
		pDatabase->sortInfoID = pDatabase->sortInfoID + newBase - oldBase;
	//pDatabase->creator = pDatabase->creator + newBase - oldBase;
	//pDatabase->uniqueIDSeed = pDatabase->uniqueIDSeed + newBase - oldBase;
	Relocate_RecordList(&(pDatabase->recordList), oldBase, newBase,
	                    IsResource(pDatabase) ? RL_RESOURCES : RL_RECORDS);
}
