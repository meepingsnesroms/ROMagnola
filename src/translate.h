#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__

#include "rom.h"

extern int	Setup_ROM					(ROMPtr						pROM);
extern void Translate_ROM				(ROMPtr						pROM);

extern void H2P_Translate_ROM			(ROMPtr						pROM);
extern void P2H_Translate_ROM			(ROMPtr						pROM);

extern void	H2P_Translate_PRC			(PRCPtr						pPRC);
extern void	P2H_Translate_PRC			(PRCPtr						pPRC);

extern void H2P_Translate_DatabaseHdr	(ROMPtr						pROM,
										 DatabaseHdrPtr				pDatabase);
extern void P2H_Translate_DatabaseHdr	(ROMPtr						pROM,
										 DatabaseHdrPtr				pDatabase);
extern void H2P_Translate_CardHeader	(ROMPtr						pROM,
										 CardHeaderPtr				pCard);
extern void P2H_Translate_CardHeader	(ROMPtr						pROM,
										 CardHeaderPtr				pCard);

extern void H2P_Translate_DatabaseList	(ROMPtr						pROM,
										 DatabaseListPtr			pDatabaseList);
extern void P2H_Translate_DatabaseList	(ROMPtr						pROM,
										 DatabaseListPtr			pDatabaseList);


typedef void(*TransFunc)	(ROMPtr pROM, const void* pList);

extern void	Translate_GenericList		(ROMPtr						pROM,
										 void**						ppList,
										 UInt16						numEntries,
										 UInt16						stepSize,
										 TransFunc					translate);

#endif //__TRANSLATE_H__
