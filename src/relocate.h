#ifndef	__RELOCATE_H__
#define	__RELOCATE_H__

#include "rom.h"
#include "os_structs.h"

extern void	Relocate_MemChunk		(MemChunkHeaderUnionType*	pChunk,
									 UInt16						version,
									 UInt32						oldBase,
									 UInt32						newBase);

extern void	Relocate_CardHeader		(CardHeaderPtr				pCard,
									 UInt32						oldBase,
									 UInt32						newBase);

extern void	Relocate_StorageHeader	(StorageHeaderPtr			pStore,
									 UInt32						oldBase,
									 UInt32						newBase);

extern void	Relocate_HeapHeader		(MemHeapHeaderUnionType*	pHeader,
									 UInt32						oldBase,
									 UInt32						newBase);

extern void	Relocate_HeapList		(HeapListPtr				pHeapList,
									 UInt32						oldBase,
									 UInt32						newBase);

extern void	Relocate_DatabaseList	(DatabaseListPtr			pList,
									 UInt32						oldBase,
									 UInt32						newBase);

extern void	Relocate_DatabaseHdr	(DatabaseHdrPtr				pDatabase,
									 UInt32						oldBase,
									 UInt32						newBase);

#endif	// __RELOCATE_H__
