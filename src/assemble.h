#ifndef	__ASSEMBLE_H__
#define	__ASSEMBLE_H__

#include "os_structs.h"
#include "rom.h"

#define PR_HEADER		0x01
#define PR_RECORDS		0x02
#define PR_INDIRECT_ADD	0x04

extern	ROMPtr	InitializeROM	(ROMVersion*				pVersion,
								 UInt16						flags);
extern	void*	ROMalloc		(ROMPtr						pROM,
								 UInt32						size,
								 UInt8						owner);
extern	void	ROMfree			(ROMPtr						pROM,
								 void*						ptr);

extern int		ShrinkROM		(ROMPtr						pROM,
								 UInt32						newSize);

extern	int		AddPRC			(ROMPtr						pROM,
								 DatabaseListPtr			pDBList,
								 PRCPtr						pPRC,
								 UInt16						flags,
								 int						DBIdex);
extern	int		AddPRCs			(ROMPtr						pROM, 
								 int						numPRCs,
								 char*						PRCNames[],
								 UInt16						flags);
extern	int		WriteROM		(ROMPtr						pROM,
								 int						hROM);

extern	int		SetSystem		(ROMPtr						pROM);

extern	int		CompareTypeCtor	(const void*				ppDB1,
								 const void*				ppDB2);
#endif	// __ASSEMBLE_H__
