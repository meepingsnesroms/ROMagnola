/******************************************************************************
 ******************************************************************************
 *** From <emulator>/SrcShared/Palm/Platform/Incs/Core/System/DataMgr.h
 ******************************************************************************
 ******************************************************************************/
/******************************************************************************
 *
 * Copyright (c) 1994-1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DataMgr.h
 *
 * Description:
 *		Header for the Data Manager
 *
 * History:
 *   	11/14/94  RM - Created by Ron Marianetti
 *
 *****************************************************************************/

#ifndef __DATAMGR_H__
#define __DATAMGR_H__

typedef UInt32	DmResType;
typedef UInt16	DmResID;

/************************************************************
 * Category equates
 *************************************************************/
#define	dmRecAttrCategoryMask	0x0F	// mask for category #
#define	dmRecNumCategories		16		// number of categories
#define	dmCategoryLength			16		// 15 chars + 1 null terminator

#define  dmAllCategories			0xff
#define  dmUnfiledCategory  		0

#define	dmMaxRecordIndex			0xffff



// Record Attributes
//
// *** IMPORTANT:
// ***
// *** Any changes to record attributes must be reflected in dmAllRecAttrs and dmSysOnlyRecAttrs ***
// ***
// *** Only one nibble is available for record attributes
//
// *** ANY CHANGES MADE TO THESE ATTRIBUTES MUST BE REFLECTED IN DESKTOP LINK
// *** SERVER CODE (DLCommon.h, DLServer.c)
#define	dmRecAttrDelete			0x80	// delete this record next sync
#define	dmRecAttrDirty				0x40	// archive this record next sync
#define	dmRecAttrBusy				0x20	// record currently in use
#define	dmRecAttrSecret			0x10	// "secret" record - password protected


// All record atributes (for error-checking)
#define	dmAllRecAttrs				( dmRecAttrDelete |			\
												dmRecAttrDirty |			\
												dmRecAttrBusy |			\
												dmRecAttrSecret )

// Record attributes which only the system is allowed to change (for error-checking)
#define	dmSysOnlyRecAttrs			( dmRecAttrBusy )


/************************************************************
 * Database Header equates
 *************************************************************/
#define	dmDBNameLength				32			// 31 chars + 1 null terminator

// Attributes of a Database
//
// *** IMPORTANT:
// ***
// *** Any changes to database attributes must be reflected in dmAllHdrAttrs and dmSysOnlyHdrAttrs ***
// ***
#define	dmHdrAttrResDB					0x0001	// Resource database
#define 	dmHdrAttrReadOnly				0x0002	// Read Only database
#define	dmHdrAttrAppInfoDirty		0x0004	// Set if Application Info block is dirty
															// Optionally supported by an App's conduit
#define	dmHdrAttrBackup				0x0008	//	Set if database should be backed up to PC if
															//	no app-specific synchronization conduit has
															//	been supplied.
#define	dmHdrAttrOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
															//  for it to install a newer version of this database
															//  with a different name if the current database is
															//  open. This mechanism is used to update the 
															//  Graffiti Shortcuts database, for example. 
#define	dmHdrAttrResetAfterInstall	0x0020 	// Device requires a reset after this database is 
															// installed.
#define	dmHdrAttrCopyPrevention		0x0040	// This database should not be copied to 

#define	dmHdrAttrStream				0x0080	// This database is used for file stream implementation.
#define	dmHdrAttrHidden				0x0100	// This database should generally be hidden from view
															//  used to hide some apps from the main view of the
															//  launcher for example.
															// For data (non-resource) databases, this hides the record
															//	 count within the launcher info screen.
#define	dmHdrAttrLaunchableData		0x0200	// This data database (not applicable for executables)
															//  can be "launched" by passing it's name to it's owner
															//  app ('appl' database with same creator) using
															//  the sysAppLaunchCmdOpenNamedDB action code. 

#define	dmHdrAttrOpen					0x8000	// Database not closed properly


// All database atributes (for error-checking)
#define	dmAllHdrAttrs					(	dmHdrAttrResDB |						\
													dmHdrAttrReadOnly |					\
													dmHdrAttrAppInfoDirty |				\
													dmHdrAttrBackup |						\
													dmHdrAttrOKToInstallNewer |		\
													dmHdrAttrResetAfterInstall |		\
													dmHdrAttrCopyPrevention |			\
													dmHdrAttrStream |						\
													dmHdrAttrOpen	)
													
// Database attributes which only the system is allowed to change (for error-checking)
#define	dmSysOnlyHdrAttrs				(	dmHdrAttrResDB |		\
													dmHdrAttrOpen	)


/************************************************************
 * Unique ID equates
 *************************************************************/
#define	dmRecordIDReservedRange		1			// The range of upper bits in the database's
															// uniqueIDSeed from 0 to this number are
															// reserved and not randomly picked when a
															// database is created.
#define	dmDefaultRecordsID			0			// Records in a default database are copied
															// with their uniqueIDSeeds set in this range.
#define	dmUnusedRecordID				0			// Record ID not allowed on the device


/************************************************************
 * Mode flags passed to DmOpenDatabase
 *************************************************************/
#define	dmModeReadOnly				0x0001		// read  access
#define	dmModeWrite					0x0002		// write access
#define	dmModeReadWrite			0x0003		// read & write access
#define	dmModeLeaveOpen			0x0004		// leave open when app quits
#define	dmModeExclusive			0x0008		// don't let anyone else open it
#define	dmModeShowSecret			0x0010		// force show of secret records

// Generic type used to represent an open Database
typedef	void *						DmOpenRef;


/************************************************************
 * Structure passed to DmGetNextDatabaseByTypeCreator and used
 *  to cache search information between multiple searches.
 *************************************************************/
typedef struct {
	UInt32		info[8];
	} DmSearchStateType;
typedef DmSearchStateType*	DmSearchStatePtr;	



/************************************************************
 * Structures used by the sorting routines
 *************************************************************/
typedef struct {
	UInt8			attributes;							// record attributes;
	UInt8			uniqueID[3];						// unique ID of record
	} SortRecordInfoType;

typedef SortRecordInfoType *SortRecordInfoPtr;

typedef Int16 DmComparF (void *, void *, Int16 other, SortRecordInfoPtr, 
								SortRecordInfoPtr, MemHandle appInfoH);



/************************************************************
 * Database manager error codes
 * the constant dmErrorClass is defined in ErrorBase.h
 *************************************************************/
#define	dmErrMemError					(dmErrorClass | 1)
#define	dmErrIndexOutOfRange			(dmErrorClass | 2)
#define	dmErrInvalidParam				(dmErrorClass | 3)
#define	dmErrReadOnly					(dmErrorClass | 4)
#define	dmErrDatabaseOpen				(dmErrorClass | 5)
#define	dmErrCantOpen					(dmErrorClass | 6)
#define	dmErrCantFind					(dmErrorClass | 7)
#define	dmErrRecordInWrongCard		(dmErrorClass | 8)
#define	dmErrCorruptDatabase			(dmErrorClass | 9)
#define	dmErrRecordDeleted			(dmErrorClass | 10)
#define	dmErrRecordArchived			(dmErrorClass | 11)
#define	dmErrNotRecordDB				(dmErrorClass | 12)
#define	dmErrNotResourceDB			(dmErrorClass | 13)
#define	dmErrROMBased					(dmErrorClass | 14)
#define	dmErrRecordBusy				(dmErrorClass | 15)
#define	dmErrResourceNotFound		(dmErrorClass | 16)
#define	dmErrNoOpenDatabase			(dmErrorClass | 17)
#define	dmErrInvalidCategory			(dmErrorClass | 18)
#define	dmErrNotValidRecord			(dmErrorClass | 19)
#define	dmErrWriteOutOfBounds		(dmErrorClass | 20)
#define	dmErrSeekFailed				(dmErrorClass | 21)
#define	dmErrAlreadyOpenForWrites	(dmErrorClass | 22)
#define	dmErrOpenedByAnotherTask	(dmErrorClass | 23)
#define  dmErrUniqueIDNotFound		(dmErrorClass | 24)
#define  dmErrAlreadyExists			(dmErrorClass | 25)
#define	dmErrInvalidDatabaseName	(dmErrorClass | 26)
#define	dmErrDatabaseProtected		(dmErrorClass | 27)
#define	dmErrDatabaseNotProtected	(dmErrorClass | 28)

/************************************************************
 * Values for the direction parameter of DmSeekRecordInCategory
 *************************************************************/
#define dmSeekForward				 1
#define dmSeekBackward				-1

#endif // __DATAMGR_H__
