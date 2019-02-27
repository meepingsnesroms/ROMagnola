#ifndef	__OUTPUT_H__
#define	__OUTPUT_H__

#include "rom.h"
#include "os_structs.h"

#define	DETAIL_NONE				0
#define	DETAIL_HEADER			1
#define	DETAIL_RECORD			2
#define	DETAIL_RECORD_INTERP	3
#define	DETAIL_RECORD_DETAIL	4
#define	DETAIL_RECORD_RAW		5


extern	void	Output_ROM				(ROMPtr						pROM,
										 UInt32						offsetAdj,
										 UInt16						bShowNV,
										 UInt16						bShowHeap,
										 UInt16						bShowChunks,
										 UInt16						bShowDB);
extern	void	Output_CardHeader		(CardHeaderPtr				pCard,
										 UInt32						offsetAdj);
extern	void	Output_StorageHeader	(StorageHeaderPtr			pStore,
										 UInt16						bShowNV,
										 UInt32						offsetAdj);
extern	void	Output_MemHeapHeader	(MemHeapHeaderUnionType*	pHeader,
										 UInt16						nHeap,
										 UInt16						bShowHeap,
										 UInt16						bShowChunks,
										 UInt32						offsetAdj);

extern	void	Output_MemMstrPtrTable	(MemMstrPtrTableUnionType*	pTable,
										 UInt16						ver,
										 UInt32						offsetAdj);
extern	void	Output_HeapList			(HeapListPtr				pHeapList,
										 UInt16						bShowHeap,
										 UInt16						bShowChunks,
										 UInt32						offsetAdj);

extern	void	Output_MemChunks		(MemChunkHeaderUnionType*	pChunk,
										 UInt16						version,
										 UInt16						bIndent,
										 UInt32						offsetAdj);
extern	void	Output_MemChunkHeader	(MemChunkHeaderUnionType*	pChunk,
										 UInt16						version,
										 UInt32						offsetAdj);


extern	void	Output_RawBytes			(UInt8*						pData,
										 UInt32						nBytes,
										 UInt32						offsetAdj,
										 int						nIndent);
extern	void	Output_RecordEntry		(ROMPtr						pROM,
										 RecordEntryPtr				pRecord,
										 DatabaseHdrPtr				pDatabase,
										 UInt32						size,
										 UInt32						offsetAdj,
										 int						nIndent,
										 UInt32						bDetail);
extern	void	Output_RsrcEntry		(ROMPtr						pROM,
										 RsrcEntryPtr				pResource,
										 DatabaseHdrPtr				pDatabase,
										 UInt32						size,
										 UInt32						offsetAdj,
										 int						nIndent,
										 UInt32						bDetail);

extern	void	Output_RecordList		(ROMPtr						pROM,
										 DatabaseHdrPtr				pDatabase,
										 UInt32						highAddr,
										 RecordListPtr				pRecordList,
										 UInt32						offsetAdj,
										 UInt16						lType,
										 UInt16						bDetail);

extern	void	Output_DatabaseList		(ROMPtr						pROM,
										 DatabaseListPtr			pList,
										 UInt32						offsetAdj,
										 UInt16						bBrief,
										 UInt16						bNoLabel,
										 UInt16						bDetail);
extern	void	Output_DatabaseHdr		(ROMPtr						pROM,
										 UInt32						highAddr,
										 DatabaseHdrPtr				pDatabase,
										 UInt32						offsetAdj,
										 UInt16						bBrief,
										 UInt16						bDetail);

#endif	// __OUTPUT_H__
