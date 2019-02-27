#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "os_structs.h"
#include "rom.h"
#include "utils.h"
#include "byte_swap.h"
#include "SystemResources.h"

#include "output.h"
#include "output_resource.h"

typedef struct	rloc_entry
{
	Int16	next;
	Int16	target;
} rloc_entry;

typedef struct	mac_jump_table
{
	UInt32	sizeAboveA5;
	UInt32	dataSize;
	UInt32	jumpTableSize;
	UInt32	jumpTableOffsetA5;
	UInt16	offset;
}	mac_jump_table;



static
void*	DecompressDATA	(void*	data,
						 UInt32	sizeAboveA5,
						 UInt32	dataSize)
{
	UInt8*	pData		= (UInt8*)data;
	void*	pRetData	= NULL;
	char*	pCopyData	= NULL;
	UInt16	nRound		= 0;
	Int32	Offset;


	if (! pData || (sizeAboveA5 < 1) || (dataSize < 1))
		return NULL;

	pRetData = (void*)malloc(sizeAboveA5 + dataSize);
	if (! pRetData)
		return NULL;

	memset(pRetData, 0, sizeAboveA5 + dataSize);

	/* Skip over the the XRef offset */
	pData += sizeof(Int32);

	for (nRound = 0; nRound < 3; nRound++)
	{
		/*
		 * The data comes in blocks of at most 128-bytes.
		 * These blocks consist of 1-byte of length followed
		 * by the bytes of data.
		 */
		Offset = BYTE_SWAP_32(*((Int32*)pData));
	
		pCopyData = (char*)pRetData + dataSize + Offset;
		pData    += sizeof(Int32);

		while (1)
		{
			unsigned char	BlkLen;
			if (*pData >= 128)
			{
				/* Literal */
				BlkLen = (*pData - 0x7F);

				memcpy(pCopyData, pData+1, BlkLen);

				pData     += BlkLen + 1;
				pCopyData += BlkLen;
			}
			else if (*pData >= 64)
			{
				/* Run of 00 */
				BlkLen = (*pData - 0x3f);

				memset(pCopyData, 0, BlkLen);

				pData     += 1;
				pCopyData += BlkLen;
			}
			else if (*pData >= 32)
			{
				/* Run of literal */
				BlkLen = (*pData - 0x1e);

				pData++;
				memset(pCopyData, *pData, BlkLen);

				pData     += 1;
				pCopyData += BlkLen;
			}
			else if (*pData >= 16)
			{
				/* Run of FF */
				BlkLen = (*pData - 0x0f);

				memset(pCopyData, 0xFF, BlkLen);

				pData     += 1;
				pCopyData += BlkLen;
			}
			else if (*pData >= 5)
			{
				/* Unknown... */
				fprintf (stdout, "*** Unknown compression type: 0x%02X\n",
						 *pData);
				break;
			}
			else if (*pData >= 4)
			{
				pData++;

				*pCopyData++ = 0xa9;
				*pCopyData++ = 0xf0;
				*pCopyData++ = 0x00;

				*pCopyData++ = *pData++;
				*pCopyData++ = *pData++;
				*pCopyData++ = *pData++;

				*pCopyData++ = 0x00;
				*pCopyData++ = *pData++;
			}
			else if (*pData >= 3)
			{
				pData++;

				*pCopyData++ = 0xa9;
				*pCopyData++ = 0xf0;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;

				*pCopyData++ = *pData++;
				*pCopyData++ = *pData++;

				*pCopyData++ = 0x00;
				*pCopyData++ = *pData++;
			}
			else if (*pData >= 2)
			{
				pData++;

				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0xff;

				*pCopyData++ = *pData++;
				*pCopyData++ = *pData++;
				*pCopyData++ = *pData++;
			}
			else if (*pData >= 1)
			{
				pData++;

				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0x00;
				*pCopyData++ = 0xff;
				*pCopyData++ = 0xff;

				*pCopyData++ = *pData++;
				*pCopyData++ = *pData++;
			}
			else
			{
				pData++;
				break;
			}
		}
	}

	return pRetData;
}

static
void	Output_OverlayInfo	(OmOverlaySpecType*	pOvly,
							 UInt32				offsetAdj,
							 int				nIndent,
							 UInt32				bDetail)
{
	UInt32				Type;
	UInt32				Ctor;
	char				Flags[32];

	if (! pOvly)
		return;

	nIndent += 2+3 + 4+8 + 1+4;

	Flags[0] = '\0';
	if (pOvly->flags & omSpecAttrForBase)
	{
		strcat(Flags, "base");

		if (pOvly->flags & omSpecAttrStripped)
		{
			strcat(Flags, ", stripped");
		}
	}
	else
	{
		strcat(Flags, "overlay");
	}


	Type = BYTE_SWAP_32(pOvly->baseDBType);
	Ctor = BYTE_SWAP_32(pOvly->baseDBCreator);

	fprintf (stdout, "%*s:   v%-2d 0x%04lX [%-14s] "
	                 "%c%c%c%c.%c%c%c%c (#%d)\n",
	         nIndent, " ",
			 pOvly->version,
			 pOvly->flags, Flags,
			 ((char*)&Type)[0],
			 ((char*)&Type)[1],
			 ((char*)&Type)[2],
			 ((char*)&Type)[3],
			 ((char*)&Ctor)[0],
			 ((char*)&Ctor)[1],
			 ((char*)&Ctor)[2],
			 ((char*)&Ctor)[3],
			 pOvly->numOverlays);

	if (bDetail == DETAIL_RECORD_DETAIL)
	{
		UInt32				idex;
		for (idex = 0; idex < pOvly->numOverlays; idex++)
		{
			OmOverlayRscType*	pRsc	= &(pOvly->overlays[idex]);
			char*				pKind	= NULL;

			switch (pRsc->overlayType)
			{
			case omOverlayKindHide:		pKind = "Hide"; break;
			case omOverlayKindAdd:		pKind = "Add";  break;
			case omOverlayKindReplace:	pKind = "Repl"; break;
			case omOverlayKindBase:		pKind = "Base"; break;
			default:					pKind = "????"; break;
			}

			Type = BYTE_SWAP_32(pRsc->rscType);

			fprintf (stdout, "%*s:   %03ld: %-4s %c%c%c%c.%d\n",
			         nIndent, " ",
					 idex,
					 pKind,
					 ((char*)&Type)[0],
					 ((char*)&Type)[1],
					 ((char*)&Type)[2],
					 ((char*)&Type)[3],
					 pRsc->rscID);
		}
	}
}

static
void	Output_DATA		(ROMPtr				pROM,
						 DatabaseHdrPtr		pDatabase,
						 void*				pData,
						 UInt32				size,
						 UInt32				offsetAdj,
						 int				nIndent,
						 UInt32				bDetail)
{
	void*			pDATA	= NULL;
	RsrcEntryPtr	pCode	= NULL;
	
	/*
	 * Locate the code.0 resource
	 * (which contains the size of the data)
	 */
	if (! (pCode = LocateResource(pDatabase, sysResTAppCode, 0)))
	{
		fprintf (stdout, "%*s          : *** Cannot locate the 'code.0' resource!\n",
		         nIndent, " ");
		Output_RawBytes((UInt8*)pData, size, offsetAdj, nIndent);
	}
	else
	{
		mac_jump_table*	pTable	= (mac_jump_table*)pCode->localChunkID;
		UInt32			sizeAboveA5	= BYTE_SWAP_32(pTable->sizeAboveA5);
		UInt32			dataSize	= BYTE_SWAP_32(pTable->dataSize);

		fprintf (stdout, "%*s          : %ld bytes of initialized data (%ld above A5)\n",
		         nIndent, " ", sizeAboveA5 + dataSize, sizeAboveA5);

		if (bDetail == DETAIL_RECORD_DETAIL)
		{
			pDATA = DecompressDATA(pData, sizeAboveA5, dataSize);

			if (! pDATA)
			{
				fprintf (stdout, "%*s          : *** Cannot decompress!!\n",
				         nIndent, " ");
			}
			else
			{
				fprintf (stdout, "%*s          : Decompressed Data\n",
				         nIndent, " ");
				Output_RawBytes((UInt8*)pDATA, sizeAboveA5+dataSize,
				                (UInt32)pDATA + dataSize, nIndent);
				free(pDATA);
			}
		}
		else if (bDetail == DETAIL_RECORD_RAW)
		{
			fprintf (stdout, "%*s          : Compressed Data\n",
			         nIndent, " ");
			Output_RawBytes((UInt8*)pData, size, offsetAdj, nIndent);
		}
	}
}

static
void	Output_RLOC		(rloc_entry*		head,
						 void*				data,
						 UInt32				offsetAdj,
						 int				nIndent,
						 UInt32				bDetail)
{
	Int16		next;
	Int16		target;
	UInt32		count	= 0;

	//nIndent += 3;

	if (! head || ! data)
		return;

	do
	{
		/* Save off next since relocating destroys the structure */
		next   = BYTE_SWAP_16(head->next);
		target = BYTE_SWAP_16(head->target);

		/* If this really wants relocating */
		if (target >= 0)
		{
			/* Output this relocation information */
			fprintf (stdout, "%*s:   %3ld: 0x%08lX offset by 0x%04X\n",
			         nIndent, " ", count++,
					 Adjust((UInt32)head, offsetAdj),
					 target);
		}

		head = (rloc_entry*)((UInt8*)data + next);
	/* rloc chains are terminated with a next value of 0xffff */
	} while (next >= 0);
}

static
void	Output_RLOCS	(ROMPtr				pROM,
						 DatabaseHdrPtr		pDatabase,
						 void*				pData,
						 UInt32				size,
						 UInt32				offsetAdj,
						 int				nIndent,
						 UInt32				bDetail)
{
	Int16*			pRLocs		= (Int16*)pData;
	UInt32			nItem		= 0;
	RsrcEntryPtr	pResource;
	RsrcEntryPtr	pCode;
	void*			data		= NULL;
	UInt32			sizeAboveA5	= 0;
	UInt32			dataSize	= 0;

	nIndent += 10;

	/* First, count them */
	while (*pRLocs > 0)
	{
		nItem++;
		pRLocs++;
	}

	fprintf (stdout, "%*s: %ld Relocation Tables\n",
	         nIndent, " ", nItem);

	if (! (pResource = LocateResource(pDatabase, sysResTAppGData, 0)))
		fprintf (stdout, "%*s: *** Cannot locate data resource\n",
		         nIndent, " ");
	else
	{
		if (! (pCode = LocateResource(pDatabase, sysResTAppCode, 0)))
			fprintf (stdout, "%*s: *** Cannot locate code.0 resource\n",
			         nIndent, " ");
		else
		{
			mac_jump_table*	pTable	= (mac_jump_table*)pCode->localChunkID;
			sizeAboveA5	= BYTE_SWAP_32(pTable->sizeAboveA5);
			dataSize	= BYTE_SWAP_32(pTable->dataSize);

			data = DecompressDATA((void*)pResource->localChunkID,
			                      sizeAboveA5, dataSize);
		}
	}

	pRLocs = (Int16*)pData;
	nItem = 0;
	while (*pRLocs > 0)
	{
		UInt32			Type	= sysResTAppCode;
		UInt16			ID		= nItem;
		UInt32			OutType;
		UInt16			RLoc	= BYTE_SWAP_16(*pRLocs);

		if (nItem == 0)
		{
			Type = sysResTAppGData;
		}

		OutType = BYTE_SWAP_32(Type);
		fprintf (stdout, "%*s:   %c%c%c%c.%-2d: 0x%04X\n",
				 nIndent, " ",
				 ((char*)&OutType)[0],
				 ((char*)&OutType)[1],
				 ((char*)&OutType)[2],
				 ((char*)&OutType)[3],
				 ID, RLoc);

		if (pResource && (bDetail == DETAIL_RECORD_DETAIL))
		{
			rloc_entry*		head	= (rloc_entry*)((UInt8*)data + RLoc);

			Output_RLOC(head, data, (UInt32)data + dataSize, nIndent, bDetail);
		}


		nItem++;
		pRLocs++;
	}


	if (data)
		free(data);

}

/*****************************************************************************
 *
 */
void	Interpret_Resource	(ROMPtr			pROM,
							 RsrcEntryPtr	pResource,
							 DatabaseHdrPtr	pDatabase,
							 UInt32			size,
							 UInt32			offsetAdj,
							 int			nIndent,
							 UInt32			bDetail)
{
	switch (pResource->type)
	{
	case sysFileTOverlay:
		Output_OverlayInfo((OmOverlaySpecType*)pResource->localChunkID,
		                   offsetAdj, nIndent, bDetail);
		break;
	case sysResTAppGData:
		Output_DATA(pROM, pDatabase, (void*)pResource->localChunkID,
		            size, offsetAdj, nIndent+7, bDetail);
		break;
	case sysResTAppRLOC:
		Output_RLOCS(pROM, pDatabase, (void*)pResource->localChunkID,
		             size, offsetAdj, nIndent+7, bDetail);
		break;
	default:
		//Output_RawBytes((UInt8*)pResource->localChunkID, size,
		//                offsetAdj, nIndent+7);
		break;
	}
}
