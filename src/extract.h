#ifndef	__EXTRACT_H__
#define	__EXTRACT_H__

#include "rom.h"
#include "os_structs.h"

extern	ROMPtr			ReadRAM		(int			hROM,
									 UInt16			flags);
extern	ROMPtr			ReadROM		(int			hROM,
									 UInt16			flags);
extern	PRCPtr			ReadPRC		(char*			pFileName);

extern	int				WritePRC	(ROMPtr			pROM,
									 DatabaseHdrPtr	pDatabase);
extern	int				WritePRCs	(ROMPtr			pROM,
									 TypeCtorType	TypeCtorList[],
									 UInt32			numEntries);


#endif	// __EXTRACT_H__
