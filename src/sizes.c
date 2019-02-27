#include <stdio.h>
#include "rom.h"

int	main	(int	argc,
			 char*	argv[])
{
	fprintf (stdout, "DatabaseListType:     %3d [%04X]\n",
	         sizeof(DatabaseListType),
	         sizeof(DatabaseListType));
	fprintf (stdout, "DatabaseHdrType:      %3d [%04X]\n",
	         sizeof(DatabaseHdrType),
	         sizeof(DatabaseHdrType));
	fprintf (stdout, "DatabaseDirType:      %3d [%04X]\n",
	         sizeof(DatabaseDirType),
	         sizeof(DatabaseDirType));
	fprintf (stdout, "DatabaseDirEntryType: %3d [%04X]\n",
	         sizeof(DatabaseDirEntryType),
	         sizeof(DatabaseDirEntryType));
	fprintf (stdout, "-----------------------------------------------\n");
	fprintf (stdout, "RecordListType:       %3d [%04X]\n",
	         sizeof(RecordListType),
	         sizeof(RecordListType));
	fprintf (stdout, "RecordEntryType:      %3d [%04X]\n",
	         sizeof(RecordEntryType),
	         sizeof(RecordEntryType));
	fprintf (stdout, "RsrcEntryType:        %3d [%04X]\n",
	         sizeof(RsrcEntryType),
	         sizeof(RsrcEntryType));
	fprintf (stdout, "-----------------------------------------------\n");
	fprintf (stdout, "MemChunkHeaderType:   %3d [%04X]\n",
	         sizeof(MemChunkHeaderType),
	         sizeof(MemChunkHeaderType));

	return 0;
}
