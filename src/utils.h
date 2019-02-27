#ifndef	__UTILS_H__
#define	__UTILS_H__

#include <time.h>
#include "types.h"
#include "rom.h"
#include "os_structs.h"

extern	void			FreeROM			(ROMPtr				pROM);
extern	void			FreePRC			(PRCPtr				pPRC);
extern	void			FreePRCList		(PRCPtr*			pPRCList,
										 int				nItems);

extern	int				CompareAddrs	(const void*		pAddr1,
										 const void*		pAddr2);


extern MemChunkHeaderUnionType*
						LocateAddrInChunk	(MemChunkHeaderUnionType*	pChunk,
											 UInt16						version,
											 UInt32						addr);
extern MemChunkHeaderUnionType*
						LocateAddrInHeap	(MemHeapHeaderUnionType*	pHeap,
											 UInt32						addr);

extern	MemChunkHeaderUnionType*
						LocateChunk		(HeapListPtr		pHeapList,
										 UInt32				addr);

extern	DatabaseHdrPtr	LocateDBbyName (DatabaseListPtr		pDBList,
										char*				name);
extern	DatabaseHdrPtr	LocateDB		(DatabaseListPtr	pDBList,
										 UInt32				type,
										 UInt32				creator,
										 UInt32*			pStart);
extern	DatabaseHdrPtr	LocateDBOverlay	(DatabaseListPtr	pDBList,
										 OmOverlaySpecType*	pOvly,
										 UInt32*			pStart);

extern	RsrcEntryPtr	LocateResource	(DatabaseHdrPtr		pDB,
										 UInt32				Type,
										 UInt16				ID);
extern	int SizeOfRecordContents		(ROMPtr				pROM,
							 			 UInt32				highAddr,
						 	 			 RecordListPtr		pRecordList,
							 			 UInt16				lType,
							 			 UInt16				idex);
extern	int DBTotalSize 				(ROMPtr				pROM,
				 						 DatabaseHdrPtr		pDB);
extern	OmOverlaySpecType*
			LocateOverlayResource		(DatabaseHdrPtr		pDatabase);

extern	time_t			pilot_time_to_unix_time	(UInt32	raw_time);
extern	UInt32			unix_time_to_pilot_time	(time_t	time);
extern	char*			pilot_time_str			(UInt32	raw_time);

extern	TypeCtorPtr		StrList2TypeCtorList	(char*			StrList[],
												 UInt32			numEntries);
extern	UInt8			IsInTypeCtorList		(UInt32			type,
												 UInt32			ctor,
												 TypeCtorPtr	TypeCtorList,
												 UInt32			numEntries);
/*
 * Adjust the given 'offset' by an 'adjust'ment value
 */
static inline	UInt32	Adjust	(UInt32	offset,
								 UInt32	adjust)
{
	return (offset ? (offset - adjust) : offset);
	//return (offset < adjust ? offset : (offset - adjust));
	//return (offset - adjust);
}

/* Does the pointer point inside the rom */
static inline int IsValidPtr (ROMPtr pROM, void* pItem)
{
	return ((UInt8*)pItem >= pROM->pROM && 
			(UInt8*)pItem <  pROM->pROM + pROM->ROMSize);
}

/*
 * Returns the size of the chunk containing the given address
 */
static inline	UInt32	SizeOfChunk	(HeapListPtr	pHeapList,
									 UInt32			addr)
{
	MemChunkHeaderUnionType* pChunk;
	UInt16					 version;
	
	if (! pHeapList)
		return 0;
	
	if (! (pChunk = LocateChunk (pHeapList, addr)))
		return 0;

	version = memUChunkVer (pHeapList->heapOffset[0]);

	return (memUChunkPayloadSize (pChunk, version));
}

/*
 * Given the head of a list (pList) and an item on the list (pItem)
 * locate the items index in the list.
 */
static inline	UInt32	LocateItemIdex	(void**		pList,
										 void*		pItem,
										 UInt32		nItems,
										 UInt32		stepSize)
{
	UInt32	idex	= 0;

	for (idex = 0; idex < nItems; idex++)
	{
		if (*pList == pItem)
			return idex;

		pList = (void**)((UInt8*)pList + stepSize);
	}

	return(nItems);
}


#endif	// __UTILS_H__
