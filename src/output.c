#include <stdio.h>
#include <string.h>

#include "os_structs.h"
#include "rom.h"
#include "output.h"
#include "utils.h"
#include "byte_swap.h"
#include "SystemResources.h"
#include "output_resource.h"

void	Output_ROM				(ROMPtr						pROM,
								 UInt32						offsetAdj,
								 UInt16						bShowNV,
								 UInt16						bShowHeap,
								 UInt16						bShowChunks,
								 UInt16						bShowDB)
{
	if (! pROM)
		return;

	//fprintf (stdout, "ROM Offset: 0x%08lX\n", (UInt32)(pROM->pROM));
	if (pROM->pCard)
	{
		Output_CardHeader    (pROM->pCard, offsetAdj);
		fflush(stdout);
	}

	if (pROM->pStore)
	{
		Output_StorageHeader (pROM->pStore, bShowNV, offsetAdj);
		fflush(stdout);
	}

	if ((bShowHeap || bShowChunks) && pROM->pHeapList)
	{
		Output_HeapList      (pROM->pHeapList,bShowHeap,bShowChunks,offsetAdj);
		fflush(stdout);
	}

	if (bShowDB && pROM->pDatabaseList)
	{
		Output_DatabaseList(pROM, pROM->pDatabaseList, offsetAdj, 0, 0,
						    bShowDB);
		fflush(stdout);
	}
}

/*
 * Output Card Info
 */
void	Output_CardHeader	(CardHeaderPtr	pCard,
							 UInt32			offsetAdj)
{
	fprintf (stdout, "- 0x%08lX ----- Card Header ------------------------------\n"
					 "  Initial Stack           0x%08lX\n"
					 "  Reset Vector            0x%08lX\n"
					 "  Signature               0x%08lX\n"
					 "  Header Version          0x%04X\n"
					 "  Flags                   0x%04X %s\n"
					 "  Card Name               '%s'\n"
					 "  Card Manufacturer       '%s'\n"
					 "  Version                 0x%04X\n"
					 "  Creation Date           %s [0x%08lX]\n"
					 "  RAM Block count         %-2d [0x%04X]\n"
					 "  Block List Offset       0x%08lX\n"
					 "  RW Params Offset        0x%08lX\n"
					 "  RW Params Size          0x%08lX [%5.2fK]\n"
					 "  RO Params Offset        0x%08lX\n"
					 "  Big ROM Offset          0x%08lX\n"
					 "  Checksum Bytes          0x%08lX [%5.2fK]\n"
					 "  Checksum (crc16)        0x%04X\n"
					 "  RW Working Offset       0x%08lX\n"
					 "  RW Working Size         0x%08lX [%5.2fK]\n"
					 "  HAL Code Offset         0x%08lX\n",

					 Adjust((UInt32)pCard, offsetAdj),
					 (pCard->initStack),
					 Adjust(pCard->resetVector, offsetAdj),
					 (pCard->signature),
					 (pCard->hdrVersion),
					 (pCard->flags),
					 (pCard->flags & memCardHeaderFlagRAMOnly ?
							"[RAM only card]"                   :
					 (pCard->flags & memCardHeaderFlag328     ?
							 "[ROM Supports 68328 processor]"   :
					 (pCard->flags & memCardHeaderFlagEZ      ?
						     "[ROM Supports 68EZ328 processor]" :
					 (pCard->flags & memCardHeaderFlag230K    ?
						     "[SmallROM supports 230Kbps]"      :
							 "")))),
					 pCard->name,
					 pCard->manuf,
					 (pCard->version),
					 pilot_time_str(pCard->creationDate),
					 (pCard->creationDate),
					 (pCard->numRAMBlocks), (pCard->numRAMBlocks),
					 Adjust(pCard->blockListOffset, offsetAdj),
					 Adjust(pCard->readWriteParmsOffset, offsetAdj),
					 (pCard->readWriteParmsSize),
					 (pCard->readWriteParmsSize / 1024.0),
					 Adjust(pCard->readOnlyParmsOffset, offsetAdj),
					 Adjust(pCard->bigROMOffset, offsetAdj),
					 (pCard->checksumBytes),
					 (pCard->checksumBytes / 1024.0),
					 (pCard->checksumValue),
					 Adjust(pCard->readWriteWorkingOffset, offsetAdj),
					 (pCard->readWriteWorkingSize),
					 (pCard->readWriteWorkingSize / 1024.0),
					 pCard->halCodeOffset);
}

void	Output_SysNVParams	(SysNVParamsPtr	pParams,
							 UInt32			offsetAdj)
{
	fprintf (stdout, "  - 0x%08lX ----- Sys NV Params --------------------------\n"
					 "    RTC Hours               0x%08lX\n"
					 "    RTC HoursMinSec         0x%08lX\n"
					 "    LCD Contrast            0x%02X\n"
					 "    LCD Brightness          0x%02X\n"
					 "    Splash Screen Offset    0x%08lX\n"
					 "    Hard Rst Screen Offset  0x%08lX\n"
					 "    Locale Language         0x%04X\n"
					 "    Locale Country          0x%04X\n"
					 "    OEM Storage 1           0x%08lX\n"
					 "    OEM Storage 2           0x%08lX\n",
					 Adjust((UInt32)pParams, offsetAdj),
					 pParams->rtcHours,
					 pParams->rtcHourMinSecCopy,
					 pParams->swrLCDContrastValue,
					 pParams->swrLCDBrightnessValue,
					 Adjust((UInt32)(pParams->splashScreenPtr), offsetAdj),
					 Adjust((UInt32)(pParams->hardResetScreenPtr), offsetAdj),
					 pParams->localeLanguage,
					 pParams->localeCountry,
					 Adjust(pParams->sysNVOEMStorage1, offsetAdj),
					 Adjust(pParams->sysNVOEMStorage2, offsetAdj));

	
}

void	Output_StorageHeader	(StorageHeaderPtr	pStore,
								 UInt16				bShowNV,
								 UInt32				offsetAdj)
{
	fprintf (stdout, "- 0x%08lX ----- Storage Header ---------------------------\n"
					 "  Signature               0x%08lX\n"
					 "  Version                 0x%04X\n"
					 "  Flags                   0x%04X [%s]\n"
					 "  Storage Name            '%s'\n"
					 "  Creation Date           %s [0x%08lX]\n"
					 "  Backup   Date           %s [0x%08lX]\n"
					 "  Heap List Offset        0x%08lX\n"
					 "  Init Code Offset 1      0x%08lX\n"
					 "  Init Code Offset 2      0x%08lX\n"
					 "  Database Dir ID         0x%08lX\n"
					 "  First Heap              0x%08lX\n"
					 "  Dynamic Heap Size       0x%08lX\n"
					 "  First RAM block         0x%08lX\n"
					 "  CRC                     0x%08lX\n",
					 Adjust((UInt32)pStore, offsetAdj),
					 (pStore->signature),
					 (pStore->version),
					 (pStore->flags),
					 (pStore->flags & memStoreHeaderFlagRAMOnly ?
						"RAM store" : "ROM store"),
					 pStore->name,
					 pilot_time_str(pStore->creationDate),
					 (pStore->creationDate),
					 pilot_time_str(pStore->backupDate),
					 (pStore->backupDate),
					 Adjust(pStore->heapListOffset, offsetAdj),
					 Adjust(pStore->initCodeOffset1, offsetAdj),
					 Adjust(pStore->initCodeOffset2, offsetAdj),
					 Adjust(pStore->databaseDirID, offsetAdj),
					 Adjust(pStore->rsvSpace, offsetAdj),
					 Adjust(pStore->dynHeapSpace, offsetAdj),
					 (pStore->firstRAMBlockSize),
					 (pStore->crc));

	if (bShowNV)
	{
		Output_SysNVParams(&(pStore->nvParams), offsetAdj);
	}
}

void	Output_MemMstrPtrTable	(MemMstrPtrTableUnionType*	pTable,
								 UInt16						ver,
								 UInt32						offsetAdj)
{
	UInt32	idex;
	UInt32*	pPtr;

	fprintf (stdout, "    - 0x%08lX ----- Master Pointer Table %d ---------------\n"
					 "      num Entries             0x%04X\n"
					 "      Next Table Offset       0x%08X\n",
					 Adjust((UInt32)pTable, offsetAdj),
					 ver,
					 memUMstrPtrTableNumEntries (pTable, ver),
					 (UInt16)Adjust(memUMstrPtrTableNextTblOffset(pTable, ver), 
									offsetAdj));

	pPtr = (UInt32*)((UInt8*)pTable + memUSizeOfMstrPtrTable (ver));
	for (idex = 0; idex < memUMstrPtrTableNumEntries(pTable, ver); idex++)
	{
		fprintf (stdout, "        %3ld: 0x%08lX\n", idex, *pPtr);
		pPtr++;
	}
}

void	Output_MemChunkHeader	(MemChunkHeaderUnionType*	pChunk,
								 UInt16						version,
								 UInt32						offsetAdj)
{
	//fprintf (stdout, "- 0x%08lX ----- Memory Chunk Header -------------\n"
	fprintf (stdout, "  0x%08lX: %c%c%c%c  size=%7d [%7.2fK]-%d: %02d%-7s %d%-8s - 0x%08X\n",
					 Adjust((UInt32)pChunk, offsetAdj),
					 (memUChunkFree(pChunk,version)    ? 'f' : ' '),
					 (memUChunkMoved(pChunk,version)   ? 'm' : ' '),
					 (memUChunkUnused2(pChunk,version) ? '2' : ' '),
					 (memUChunkUnused3(pChunk,version) ? '3' : ' '),
					 memUChunkSize(pChunk,version), 
					 (memUChunkSize(pChunk,version) / 1024.0),
					 memUChunkSizeAdj(pChunk,version),
					 memUChunkLockCount(pChunk,version),
					 (memUChunkLockCount(pChunk,version) == memPtrLockCount
					  	? "[!move]" : ""),
					 memUChunkOwner(pChunk,version),
					 (memUChunkOwner(pChunk,version) == dmDynOwnerID
					  	? "[Dyn]" :
					 (memUChunkOwner(pChunk,version) == dmMgrOwnerID
					  	? "[Mgr]" :
					 (memUChunkOwner(pChunk,version) == dmRecOwnerID
					  	? "[Rec]" :
					 (memUChunkOwner(pChunk,version) == dmOrphanOwnerID
					  	? "[Orphan]" :
					 (memUChunkOwner(pChunk,version) == memOwnerMasterPtrTbl
					  	? "[MPTbl]" : ""))))),
					 memUChunkHOffset(pChunk,version));
}

void	Output_MemChunks	(MemChunkHeaderUnionType*	pChunk,
							 UInt16						version,
							 UInt16						bIndent,
							 UInt32						offsetAdj)
{
	UInt32	Total		= 0;
	UInt32	TotalAdj	= 0;

	fprintf (stdout, "%s---------------- Memory Chunks ----------------------------\n",
			 (bIndent ? "    " : ""));
	while (pChunk && ! memUChunkIsTerminator(pChunk,version))
	{
		Total    += memUChunkSize   (pChunk,version);
		TotalAdj += memUChunkSizeAdj(pChunk,version);
		Output_MemChunkHeader(pChunk, version, offsetAdj);
		
		if (memUChunkIsTerminator(pChunk,version))
			break;

		pChunk = memUChunkNext (pChunk, version);
	}
	fprintf (stdout, "%s-----------------------------------------------------------\n"
	                 "            : Total Size=%7ld [%7.2fK]-%ld\n",
	                 (bIndent ? "    " : ""),
					 Total, (Total / 1024.0),
					 TotalAdj);
}

void	Output_MemHeapHeader		(MemHeapHeaderUnionType*	pHeader,
									 UInt16						nHeap,
									 UInt16						bShowHeap,
									 UInt16						bShowChunks,
									 UInt32						offsetAdj)
{
	UInt16	ver	= memUHeapVer(pHeader);

	if (bShowHeap)
	{
		fprintf (stdout, "  - 0x%08lX ----- Heap Header #%03d -----------------------\n"
						 "    Flags                   0x%04X [version %d%s]\n",
						 Adjust((UInt32)pHeader, offsetAdj),
						 nHeap,
						 memUHeapFlags(pHeader), ver,
						 (memUHeapFlags(pHeader) & memHeapFlagReadOnly ?
						  		" RO" : ""));

		switch (ver)
		{
		case 1:
			fprintf (stdout, "    Size                    0x%04lX [%5.2fK]\n",
						 memUHeapSize(pHeader,ver), 
						 memUHeapInterpSize(pHeader,ver)/1024.0);
			break;

		case 2:
			fprintf (stdout, "    Size                    0x%08lX [%5.2fK]\n",
						 memUHeapSize(pHeader,ver), 
						 memUHeapInterpSize(pHeader,ver)/1024.0);
			break;

		case 3:
		case 4:
			fprintf (stdout, "    Size                    0x%08lX [%5.2fK]\n"
							 "    First Free Chunk        0x%08lX [0x%08lX]\n",
						 memUHeapSize(pHeader,ver), 
						 memUHeapInterpSize(pHeader,ver)/1024.0,
						 pHeader->header.ver3.firstFreeChunkOffset,
						 pHeader->header.ver3.firstFreeChunkOffset * 2);
			break;
		}
		Output_MemMstrPtrTable(memUHeapMstrPtrAddr(pHeader, ver), 
							   memUHeapMstrPtrVer (ver), offsetAdj);

	}

	/*
	 * Output all chunks in this heap (if requested)
	 */
	if (bShowChunks)
	{
		MemChunkHeaderUnionType*	pFirstChunk	= memUHeapFirstChunk (pHeader,ver);
		Output_MemChunks(pFirstChunk,memUChunkVer(pHeader),bShowHeap,offsetAdj);
	}
}

void	Output_HeapList			(HeapListPtr		pHeapList,
								 UInt16				bShowHeap,
								 UInt16				bShowChunks,
								 UInt32				offsetAdj)
{
	UInt16	idex;

	if (bShowHeap)
	{
		fprintf (stdout, "- 0x%08lX ----- Heap List --------------------------------\n"
						 "  num Heaps               0x%04X\n",
						 Adjust((UInt32)pHeapList, offsetAdj),
						 pHeapList->numHeaps);
	}

	for (idex = 0; idex < pHeapList->numHeaps; idex++)
	{
		Output_MemHeapHeader((MemHeapHeaderUnionType*)
		                           (pHeapList->heapOffset[idex]),
								   idex,
								   bShowHeap,
		                           bShowChunks,
								   offsetAdj);
	}
}

/*
 * Output the given buffer as raw bytes...
 */
static
void	Output_ASCII		(UInt8*			pData,
							 UInt32			nStart,
							 UInt32			nEnd)
{
	UInt32	idex;

	fprintf (stdout, " : ");
	for (idex = nStart; idex < nEnd; idex++)
	{
		if ((pData[idex] >= ' ') && (pData[idex] <= '~'))
			fprintf (stdout, "%c", pData[idex]);
		else
			fprintf (stdout, ".");
	}
	fprintf (stdout, "\n");
}

/*
 * Output the given buffer as raw bytes...
 */
void	Output_RawBytes		(UInt8*			pData,
							 UInt32			nBytes,
							 UInt32			offsetAdj,
							 int			nIndent)
{
	UInt32	idex;
	//					ncols           addr    percol   col_groups
	UInt32	nCols	= (((78 - nIndent - 11 - 3) / 4)     / 4) * 4;
	UInt32	nCol	= 0;
	UInt32	nStart	= -1;

	for (idex = 0; idex < nBytes; idex++)
	{
		if (! (idex % nCols))
		{
			if (nStart != -1)
			{
				Output_ASCII(pData, nStart, idex);
			}

			fprintf (stdout, "%*s0x%08lX:",
					 nIndent, " ",
					 Adjust((UInt32)&(pData[idex]), offsetAdj));
			nStart = idex;
			nCol   = 0;
		}

		fprintf (stdout, " %02X", pData[idex]);

		nCol++;
	}

	while (nCol < nCols)
	{
		fprintf (stdout, "   ");
		nCol++;
	}

	if (nStart != -1)
	{
		Output_ASCII(pData, nStart, idex);
	}
	fprintf (stdout, "\n");
}


void	Output_RecordEntry	(ROMPtr			pROM,
							 RecordEntryPtr	pRecord,
							 DatabaseHdrPtr	pDatabase,
							 UInt32			size,
							 UInt32			offsetAdj,
							 int			nIndent,
							 UInt32			bDetail)
{
	fprintf (stdout, "0x%02X  0x%02X.%02X.%02X  0x%08lX [%5ld bytes]\n",
			 pRecord->attributes,
			 pRecord->uniqueID[0],
			 pRecord->uniqueID[1],
			 pRecord->uniqueID[2],
	         Adjust(pRecord->localChunkID, offsetAdj),
			 size);

	if (bDetail > DETAIL_RECORD)
	{
		Output_RawBytes((UInt8*)pRecord->localChunkID, size,
		                offsetAdj, nIndent);
	}
}

void	Output_RsrcEntry	(ROMPtr			pROM,
							 RsrcEntryPtr	pResource,
							 DatabaseHdrPtr	pDatabase,
							 UInt32			size,
							 UInt32			offsetAdj,
							 int			nIndent,
							 UInt32			bDetail)
{
	UInt32	Type	= BYTE_SWAP_32(pResource->type);
	fprintf (stdout, "%c%c%c%c  %6d  0x%08lX [%5ld bytes]\n",
	         ((char*)&(Type))[0],
	         ((char*)&(Type))[1],
	         ((char*)&(Type))[2],
	         ((char*)&(Type))[3],
			 pResource->id,
	         Adjust(pResource->localChunkID, offsetAdj),
			 size);

	if (bDetail == DETAIL_RECORD_RAW)
	{
		Output_RawBytes((UInt8*)pResource->localChunkID, size,
		                offsetAdj, nIndent+7);
	}
	else if (bDetail > DETAIL_RECORD)
	{
		/* Output a bit of information about this resouce */
		Interpret_Resource(pROM, pResource, pDatabase,
		                   size, offsetAdj, nIndent, bDetail);
	}
}

/*
 * Output a record list.  This may be a list of resources, records,
 * or database headers depending upon 'lType':
 * 	RL_RESOURCES (0)	- Resources
 * 	RL_RECORDS   (1)	- Records
 */
void	Output_RecordList	(ROMPtr			pROM,
							 DatabaseHdrPtr	pDatabase,
							 UInt32			highAddr,
							 RecordListPtr	pRecordList,
							 UInt32			offsetAdj,
							 UInt16			lType,
							 UInt16			bDetail)
{
	UInt32	idex;
	UInt8*	pItem		= (UInt8*)(&(pRecordList->firstEntry));
	UInt32	ItemBytes;
	char*	pLabel;
	char	Header[256];
	//char*	pIndent		= "         ";
	int		nIndent		= 9;

	switch (lType)
	{
	case RL_RESOURCES:
		pLabel    = "Resource";
		ItemBytes = sizeof(RsrcEntryType);

		sprintf (Header, "%-3s: %-10s %-4s: %-4s  %-6s  %-10s",
		         "idx", "offset", "size", "type", "id", "Chunk ID");
		break;

	case RL_RECORDS:
		pLabel    = "Record";
		ItemBytes = sizeof(RecordEntryType);

		sprintf (Header, "%-3s: %-10s %-4s: %-4s  %-10s  %-10s",
		         "idx", "offset", "size", "attr", "Unique ID", "Chunk ID");
		break;
	}

	fprintf (stdout, "%*s%d %s%s [next 0x%08lX]\n",
			 nIndent, " ",
			 pRecordList->numRecords, pLabel,
			 (pRecordList->numRecords == 1 ? "" : "s"),
			 Adjust(pRecordList->nextRecordListID, offsetAdj));
	fprintf (stdout, "%*s  %s\n", nIndent, " ", Header);

	for (idex = 0; idex < pRecordList->numRecords; idex++)
	{
		UInt32	nBytes;

		fprintf (stdout, "%*s  %3ld: 0x%08lX %4ld: ",
		         nIndent, " ",
				 idex,
				 Adjust((UInt32)pItem, offsetAdj), ItemBytes);


		nBytes = SizeOfRecordContents (pROM, highAddr, pRecordList, lType, idex);
		switch (lType)
		{
		case RL_RESOURCES:		// Resource
			Output_RsrcEntry(pROM, (RsrcEntryPtr)pItem, pDatabase,
			                 nBytes, offsetAdj, nIndent, bDetail);
			break;

		case RL_RECORDS:		// Record
			Output_RecordEntry(pROM, (RecordEntryPtr)pItem, pDatabase,
			                   nBytes, offsetAdj, nIndent, bDetail);
			break;
		}

		pItem += ItemBytes;
	}
}

/*
 * Relocate a database list.
 */
void	Output_DatabaseList	(ROMPtr				pROM,
							 DatabaseListPtr	pList,
							 UInt32				offsetAdj,
							 UInt16				bBrief,
							 UInt16				bNoLabel,
							 UInt16				bDetail)
{
	UInt32	idex;

	if (bBrief)
	{
		fprintf (stdout, "      %d Database%s [next 0x%08lX]\n",
						 pList->numDatabases,
						 (pList->numDatabases == 1 ? "" : "s"),
						 Adjust(pList->nextRecordListID, offsetAdj));
	}
	else
	{
		if (! bNoLabel)
			fprintf (stdout, "- 0x%08lX ----- Database List ---------------------------\n",
						 Adjust((UInt32)pList, offsetAdj));

		fprintf (stdout, "  Next Database ID       0x%08lX\n"
						 "  num Databases          %d [0x%04X]\n",
						 Adjust(pList->nextRecordListID, offsetAdj),
						 pList->numDatabases,
						 pList->numDatabases);
	}

	fprintf (stdout, "%s    %-3s: %-10s %-6s: %-4s.%-4s %-3s  %-11s  %s\n",
	         (bBrief ? "    " : ""),
	         "idx", "offset", "size", "type", "ctor", "ver", "attributes",
			 "name");

	for (idex = 0; idex < pList->numDatabases; idex++)
	{
		DatabaseHdrPtr	pDB			= pROM->pSortedDBList[idex];
		UInt32			ItemIdex	= LocateItemIdex((void**)pList->databaseOffset,
									                 (void*)pDB,
													 pList->numDatabases,
													 sizeof(pDB));

		if (ItemIdex >= pList->numDatabases)
		{
			pDB      = (DatabaseHdrPtr)(pList->databaseOffset[idex]);
			ItemIdex = idex;
		}

		fprintf (stdout, "%s    %3ld: 0x%08lX %6d: ",
				 (bBrief ? "    " : ""),
		         ItemIdex,
				 Adjust((UInt32)pDB, offsetAdj), 
				 DBTotalSize (pROM, pDB));
		Output_DatabaseHdr(pROM, 0, pDB, offsetAdj, 1, bDetail);
	}
}

void	Output_DatabaseHdr	(ROMPtr			pROM,
							 UInt32			highAddr,
							 DatabaseHdrPtr	pDatabase,
							 UInt32			offsetAdj,
							 UInt16			bBrief,
							 UInt16			bDetail)
{
	if (bBrief)
	{
		UInt32	Type	= BYTE_SWAP_32(pDatabase->type);
		UInt32	Ctor	= BYTE_SWAP_32(pDatabase->creator);

		fprintf (stdout, "%c%c%c%c.%c%c%c%c %3d [%c%c%c%c%c%c%c%c%c%c%c] '%s'\n",
						 ((char*)&(Type))[0],
						 ((char*)&(Type))[1],
						 ((char*)&(Type))[2],
						 ((char*)&(Type))[3],
						 ((char*)&(Ctor))[0],
						 ((char*)&(Ctor))[1],
						 ((char*)&(Ctor))[2],
						 ((char*)&(Ctor))[3],
						 pDatabase->version,
						 (pDatabase->attributes & dmHdrAttrResDB
						  	? 'R' : 'r'),
						 (pDatabase->attributes & dmHdrAttrReadOnly
						  	? 'R' : 'W'),
						 (pDatabase->attributes & dmHdrAttrAppInfoDirty
						  	? 'D' : '.'),
						 (pDatabase->attributes & dmHdrAttrBackup
						  	? 'b' : '.'),
						 (pDatabase->attributes & dmHdrAttrOKToInstallNewer
						  	? 'i' : '.'),
						 (pDatabase->attributes & dmHdrAttrResetAfterInstall
						  	? 'r' : '.'),
						 (pDatabase->attributes & dmHdrAttrCopyPrevention
						  	? '@' : '.'),
						 (pDatabase->attributes & dmHdrAttrStream
						  	? 's' : '.'),
						 (pDatabase->attributes & dmHdrAttrHidden
						  	? 'h' : '.'),
						 (pDatabase->attributes & dmHdrAttrLaunchableData
						  	? 'x' : '.'),
						 (pDatabase->attributes & dmHdrAttrOpen
						  	? 'o' : '.'),
						 pDatabase->name);

		if (bDetail > DETAIL_HEADER)
			Output_RecordList(pROM, pDatabase,
			                  highAddr, &(pDatabase->recordList),offsetAdj,
			                  IsResource(pDatabase) ? RL_RESOURCES : RL_RECORDS,
							  bDetail);
	}
	else
	{
		UInt32	Type	= BYTE_SWAP_32(pDatabase->type);
		UInt32	Ctor	= BYTE_SWAP_32(pDatabase->creator);

		fprintf (stdout, "- 0x%08lX ----- Database Header --------------------------\n"
						 "  Name                    '%s'\n"
						 "  Attributes              0x%04X [%c%c%c%c%c%c%c%c%c%c%c]\n"
						 "  Version                 0x%04X\n"
						 "  Date Created            %s [0x%08lX]\n"
						 "  Date Modified           %s [0x%08lX]\n"
						 "  Date Last Backup        %s [0x%08lX]\n"
						 "  Modification Number     0x%08lX\n"
						 "  Application Info ID     0x%08lX\n"
						 "  Sort Info ID            0x%08lX\n"
						 "  Database Type           %c%c%c%c\n"
						 "  Database Creator        %c%c%c%c\n"
						 "  Unique ID Seed          0x%08lX\n",
						 Adjust((UInt32)pDatabase, offsetAdj),
						 pDatabase->name,
						 pDatabase->attributes,
						 (pDatabase->attributes & dmHdrAttrResDB
						  	? 'R' : 'r'),
						 (pDatabase->attributes & dmHdrAttrReadOnly
						  	? 'R' : 'W'),
						 (pDatabase->attributes & dmHdrAttrAppInfoDirty
						  	? 'D' : '.'),
						 (pDatabase->attributes & dmHdrAttrBackup
						  	? 'b' : '.'),
						 (pDatabase->attributes & dmHdrAttrOKToInstallNewer
						  	? 'i' : '.'),
						 (pDatabase->attributes & dmHdrAttrResetAfterInstall
						  	? 'r' : '.'),
						 (pDatabase->attributes & dmHdrAttrCopyPrevention
						  	? '@' : '.'),
						 (pDatabase->attributes & dmHdrAttrStream
						  	? 's' : '.'),
						 (pDatabase->attributes & dmHdrAttrHidden
						  	? 'h' : '.'),
						 (pDatabase->attributes & dmHdrAttrLaunchableData
						  	? 'x' : '.'),
						 (pDatabase->attributes & dmHdrAttrOpen
						  	? 'o' : '.'),
						 pDatabase->version,
						 pilot_time_str(pDatabase->creationDate),
						 pDatabase->creationDate,
						 pilot_time_str(pDatabase->modificationDate),
						 pDatabase->modificationDate,
						 pilot_time_str(pDatabase->lastBackupDate),
						 pDatabase->lastBackupDate,
						 pDatabase->modificationNumber,
						 pDatabase->appInfoID,
						 pDatabase->sortInfoID,
						 ((char*)&(Type))[0],
						 ((char*)&(Type))[1],
						 ((char*)&(Type))[2],
						 ((char*)&(Type))[3],
						 ((char*)&(Ctor))[0],
						 ((char*)&(Ctor))[1],
						 ((char*)&(Ctor))[2],
						 ((char*)&(Ctor))[3],
						 pDatabase->uniqueIDSeed);

		if (bDetail > DETAIL_HEADER)
			Output_RecordList(pROM, pDatabase,
			                  highAddr, &(pDatabase->recordList),offsetAdj,
			                  IsResource(pDatabase) ? RL_RESOURCES : RL_RECORDS,
							  bDetail);
	}
}
