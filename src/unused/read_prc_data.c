#include <stdio.h>
#include <stdlib.h>

#include "rom.h"
#include "DataPrv.h"
#include "SystemResources.h"
#include "extract.h"
#include "utils.h"
#include "byte_swap.h"

void*	DecompressDATA	(void*	data,
						 int*	pLen)
{
	char*	pData		= (char*)data;
	void*	pRetData	= NULL;
	char*	pCopyData	= NULL;


	if (! pData || ! pLen)
		return NULL;

	/* Skip over 4 bytes of offset to xrefs */
	pData += 4;
	memcpy(pLen, pData, sizeof(*pLen));

	/* REMOVE FOR PALM */ *pLen = BYTE_SWAP_32(*pLen);
	/* The length in the header is the negative of the actual length */
	*pLen = 0 - *pLen;
	pData += 4;

	pRetData = (void*)malloc(*pLen);
	if (! pRetData)
		return NULL;

	memset(pRetData, 0, *pLen);

	/*
	 * The data comes in blocks of at most 128-bytes.
	 * These blocks consist of 1-byte of length followed
	 * by the bytes of data.
	 */
	pCopyData = (char*)pRetData;
	while (*pData & 0x80)
	{
		unsigned char	BlkLen	= (*pData - 0x7F);

		memcpy(pCopyData, pData+1, BlkLen);

		pData     += BlkLen + 1;
		pCopyData += BlkLen;
	}

	return pRetData;
}

void	OutputBytes	(void*	data,
					 int	nBytes)
{
	unsigned char*	pData	= (unsigned char*)data;
	int		idex;

	fprintf (stdout, "%d bytes\n", nBytes);
	for (idex = 0; idex < nBytes; idex++)
	{
		if (idex && ((idex % 16) == 0))
			fprintf (stdout, "\n");

		fprintf (stdout, "%02X ", pData[idex]);
	}

	fprintf (stdout, "\n");
}
			 

int	main	(int	argc,
			 char*	argv[])
{
	PRCPtr			pPRC;
	RsrcEntryPtr	pRsrc;

	if ((pPRC = ReadPRC(argv[1])) == NULL)
	{
		fprintf (stderr, "Error\n");
		return -1;
	}

	if ((pRsrc = LocateResource(pPRC->pDB, sysResTAppGData, 0)) != NULL)
	{
		char*	pData	= (char*)pRsrc->localChunkID;
		int		nBytes;
		void*	pBlock	= DecompressDATA(pData, &nBytes);

		if (! pBlock)
		{
			fprintf (stderr, "Error 2\n");
		}
		else
		{
			OutputBytes(pBlock, nBytes);
		}
	}

	FreePRC(pPRC);

	return 0;
}
