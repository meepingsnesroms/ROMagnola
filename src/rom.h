#ifndef __ROM_H__
#define __ROM_H__

#include "os_structs.h"

#define	DEF_CARD_NAME	"RomeoCard"
#define	DEF_CARD_MANU	"Romeo Computing"
#define	DEF_STORE_NAME	"Romeo Store"

/******************************************************************************
 ******************************************************************************
 *** Custom structures
 ******************************************************************************
 ******************************************************************************/

/******************************************
 * Structure to identify PRC/database
 */
typedef struct	{
	UInt32						type;
	UInt32						creator;
	UInt16						isOverlay;
	} PRCIDType;
typedef PRCIDType*	PRCIDPtr;

/******************************************
 * Version-specific ROM defaults
 */
typedef struct {
	UInt16		palmOSVer;
	UInt32		ROM_base;

	UInt32		small_ROMSize;	// Size of hard-memory devoted to small ROM.
	UInt32		small_cardSize;	// Size of structures and heaps in small ROM.
	PRCIDPtr	small_PRCList;
	char*		small_layout;	// layout of PRCs in the small ROM image
	char*		small_addLayout;// layout of PRCs added to existing ROM
	
	UInt32		big_ROMSize;	// Size of hard-memory devoted to large ROM.
	UInt32		big_cardSize;	// Size of structures and heaps in large ROM.
	PRCIDPtr	big_PRCList;
	char*		big_layout;		// layout of PRCs in the large ROM image
	char*		big_addLayout;	// layout of PRCs added to existing ROM
	
	UInt8		card_name[memMaxNameLen];
	UInt8		card_manuf[memMaxNameLen];
	UInt32		card_initStack;
	UInt16		card_hdrVersion;
	UInt16		card_flags;
	UInt16		card_version;
	UInt32		card_readOnlyParmsOffset;
	UInt32		card_bigROMOffset;
	
	UInt8		store_name[memMaxNameLen];
	UInt16		store_version;
	UInt16		store_flags;

	UInt16		heap_flags;
} ROMVersion;

#define romNumROMVers 7
extern ROMVersion ROMVers[romNumROMVers];

// A Database list gives a list of database offsets
typedef struct {
	LocalID				nextRecordListID;	// local chunkID of next list
	UInt16				numDatabases;		// Number of databases
	UInt32				databaseOffset[1];	// offset to database
	} DatabaseListType;
typedef DatabaseListType*		DatabaseListPtr;

typedef	struct	{
	UInt32	type;
	UInt32	ctor;
	} TypeCtorType;
typedef TypeCtorType*		TypeCtorPtr;

// This is just a guess -- we have an idea that a block list is 64-bits
// but no idea what those 64-bits are used for...
typedef	UInt8			BlockListType[8];
typedef	BlockListType*	BlockListPtr;

/******************************************
 * Meta-data for a PRC/database
 */
typedef struct {
	UInt32						nBytes;
	DatabaseHdrPtr				pDB;
	} PRCType;
typedef PRCType*	PRCPtr;

/******************************************
 * Meta-data for a Palm ROM
 */
#define	RT_SMALL		0x01
#define	RT_LARGE		0x02
#define	RT_HEADERS_ONLY	0x10
#define	RT_TYPE_MASK	0x0F


typedef	struct	{
	UInt8*						pROM;			// Points to the ROM bytes.

	UInt32						ROMSize;		// How large is the ROM?
	UInt32						CardSize;		// How much of the ROM are we
												// using?

	UInt32						ROM_base;		// Address of the ROM (small
												// ROM)
	UInt32						Card_base;		// Address of the card (either
												// ROM)
	UInt32						File_base;
	
	CardHeaderPtr				pCard;
	StorageHeaderPtr			pStore;
	HeapListPtr					pHeapList;
	
	DatabaseListPtr				pDatabaseList;
	DatabaseHdrPtr*				pSortedDBList;	// Array of DatabaseHdrPtrs

	void*						pContext;

	/*
	 * Version-specific defaults
	 */
	ROMVersion*					pVersion;
	UInt16						flags;

}	ROMType;
typedef ROMType*	ROMPtr;

extern ROMVersion*	LocateROMVerByOSVer	(UInt16	osver);
extern ROMVersion*	GuessVersion		(ROMPtr	pROM);

#endif /* __ROM_H__ */
