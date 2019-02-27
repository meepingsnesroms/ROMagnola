#ifndef	__OUTPUT_RESOURCE_H__
#define	__OUTPUT_RESOURCE_H__

#include "rom.h"
#include "os_structs.h"

extern void	Interpret_Resource	(ROMPtr			pROM,
								 RsrcEntryPtr	pResource,
								 DatabaseHdrPtr	pDatabase,
								 UInt32			size,
								 UInt32			offsetAdj,
								 int			nIndent,
								 UInt32			bDetail);

#endif	// __OUTPUT_RESOURCE_H__
