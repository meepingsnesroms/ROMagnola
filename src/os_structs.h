#ifndef	__OS_STRUCTS_H__
#define	__OS_STRUCTS_H__

// Ensure that structure elements are 16-bit aligned
// Other [host] development platforms may need this as well...
#pragma pack(2)

#include "types.h"

#define	RL_RESOURCES	0
#define	RL_RECORDS		1

#include "DataMgr.h"
#include "SystemPrv.h"
#include "MemoryPrv.h"
#include "DataPrv.h"

/*******************************************************************
 * Convenience macros, not part of the Palm emulator
 */

/*******************************************************************
 * Data stuff
 */
#define dmPadOwnerID				0x04		// Chunks used for padding
#define	IsResource(db)	((db)->attributes & dmHdrAttrResDB)

/*******************************************************************
 * Memory stuff
 */
typedef HeapListType*		HeapListPtr;
#define memHeapFlagVers1	0x0000		// version 1 heap (<= 64K)

#define memUHeapInterpSize(p,ver)	\
	(ver>2 ? memUHeapSize(p,ver) : 	\
	 memUHeapSize(p,ver) != 0 ? memUHeapSize(p,ver) : 0x010000)
		
#define memUHeapMstrPtrVer(ver)		\
		(ver>2 ? 2 : 1)
	
#define	memUHeapVerFromFlags(v)   \
	((v) & memHeapFlagVers4 ? 4 : \
	((v) & memHeapFlagVers3 ? 3 : \
	((v) & memHeapFlagVers2 ? 2 : 1)))

#define memUHeapFirstChunk(p,ver) \
			(MemChunkHeaderUnionType*)((UInt8*)(p) + (memUSizeOfHeapHeader(ver)))

#define	memUChunkVerFromHeapVer(v)	\
		((v) < 2 ? 1 : 2)

#define	memUChunkIsTerminator(p,ver)	\
	(memUChunkSize(p,ver) == 0)
		
// p is a HeapHdrPtr
#define memUChunkVer(p) \
	(memUChunkVerFromHeapVer(memUHeapVer(p)))

#define memUChunkPayloadSize(p,ver)	\
		(memUChunkSize(p,ver) - memUSizeOfChunkHeader(ver) - memUChunkSizeAdj(p,ver))

#define memUChunkFree(p,ver)		\
	(ver>1 ? ((MemChunkHeaderPtr)p)->free : memUChunkFlags(p,ver) & memChunkFlagFree)

#define memUChunkMoved(p,ver)		\
	(ver>1 ? ((MemChunkHeaderPtr)p)->moved : \
	 memUChunkFlags(p,ver) & memChunkFlagUnused1) 

#define memUChunkUnused2(p,ver)		\
	(ver>1 ? ((MemChunkHeaderPtr)p)->unused2 : \
	 memUChunkFlags(p,ver) & memChunkFlagUnused2) 

#define memUChunkUnused3(p,ver)		\
	(ver>1 ? ((MemChunkHeaderPtr)p)->unused3 : \
	 memUChunkFlags(p,ver) & memChunkFlagUnused3) 


#define memUChunkData(p,v) (void *)((UInt32)(p) + memUSizeOfChunkHeader(v))

#define	memUChunkNext(p,v)		(MemChunkHeaderUnionType*)((UInt32)(p) + \
									                       memUChunkSize(p,v))
#define	memUChunkNextFree(p,v)	(MemChunkHeaderUnionType*)((UInt32)(p) + \
									                       (memUChunkHOffset(p,v) << 1))

#endif /* __OS_STRUCTS_H__ */
