#ifndef	__BYTE_SWAP_H__
#define	__BYTE_SWAP_H__

#include "rom.h"
#include "os_structs.h"

extern void	H2P_MemChunk			(MemChunkHeaderUnionType*	pChunk, 
									 UInt16						version);
extern void	P2H_MemChunk			(MemChunkHeaderUnionType*	pChunk, 
									 UInt16						version);

extern void	H2P_HeapHeader			(MemHeapHeaderUnionType*	pHeader);
extern void	P2H_HeapHeader			(MemHeapHeaderUnionType*	pHeader);
extern void	H2P_CardHeader			(CardHeaderPtr				pCard);
extern void	P2H_CardHeader			(CardHeaderPtr				pCard);
extern void	H2P_StorageHeader		(StorageHeaderPtr			pStore);
extern void	P2H_StorageHeader		(StorageHeaderPtr			pStore);
extern void	H2P_HeapList			(HeapListPtr				pHeapList);
extern void	P2H_HeapList			(HeapListPtr				pHeapList);
extern void	H2P_DatabaseList		(DatabaseListPtr			pDatabaseList);
extern void	P2H_DatabaseList		(DatabaseListPtr			pDatabaseList);
extern void	H2P_DatabaseHdr			(DatabaseHdrPtr				pDatabase);
extern void	P2H_DatabaseHdr			(DatabaseHdrPtr				pDatabase);
extern void	H2P_OverlaySpec			(OmOverlaySpecType*			pOvly);
extern void	P2H_OverlaySpec			(OmOverlaySpecType*			pOvly);

/******************************************************************************
 ******************************************************************************
 *** From <emulator>/SrcShared/ByteSwapping.h
 ******************************************************************************
 ******************************************************************************/
#if CPU_ENDIAN == CPU_ENDIAN_LITTLE
#  define BYTE_SWAP_16(n)	((((unsigned short) (n) & 0x0000FF00) >> 8) |	\
							 (((unsigned short) (n) & 0x000000FF) << 8))

#  define BYTE_SWAP_32(n)	( (((unsigned long) (n)) << 24) |				\
							 ((((unsigned long) (n)) << 8) & 0x00FF0000) |	\
							 ((((unsigned long) (n)) >> 8) & 0x0000FF00) |	\
							  (((unsigned long) (n)) >> 24))

#endif	// CPU_ENDIAN == CPU_ENDIAN_LITTLE

#endif	// __BYTE_SWAP_H__
