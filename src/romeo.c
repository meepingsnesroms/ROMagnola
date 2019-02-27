#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "opts_parse.h"
#include "rom.h"
#include "os_structs.h"
#include "extract.h"
#include "assemble.h"
#include "output.h"
#include "utils.h"

#define	SET_D(v)												\
	(v) = (opt_d.optidex != 0 || opt_V.optidex !=0) ?			\
					DETAIL_HEADER : DETAIL_NONE;				\
	if (opt_D.optidex)											\
	{															\
		/* Do we have an option modifier? */					\
		if (opt_D.optmod)										\
		{														\
			UInt32	Level = strtoul(opt_D.optmod, NULL, 0);		\
			if (Level)											\
				(v) = Level;									\
			else												\
				(v) = DETAIL_RECORD;							\
		}														\
		else													\
		{														\
			(v) = DETAIL_RECORD;								\
		}														\
																\
		if (! (v))												\
			(v) = DETAIL_HEADER;								\
	}

/******************************************************************************
 ******************************************************************************
 *** MAIN
 ******************************************************************************
 ******************************************************************************/
static char*	progName		= NULL;

static opt_info	opt_a			= EMPTY_OPT_INFO;
static opt_info	  opt_aR		= EMPTY_OPT_INFO;
static opt_info	  opt_al		= EMPTY_OPT_INFO;
static opt_info	  opt_as		= EMPTY_OPT_INFO;
static opt_info	  opt_ao		= EMPTY_OPT_INFO;
static opt_info	  opt_af		= EMPTY_OPT_INFO;
static opt_info	  opt_aF		= EMPTY_OPT_INFO;
static opt_info	opt_r			= EMPTY_OPT_INFO;
static opt_info	opt_p			= EMPTY_OPT_INFO;
static opt_info	opt_x			= EMPTY_OPT_INFO;
static opt_info	  opt_xl		= EMPTY_OPT_INFO;
static opt_info	  opt_xs		= EMPTY_OPT_INFO;
static opt_info	opt_v			= EMPTY_OPT_INFO;
static opt_info	  opt_vl		= EMPTY_OPT_INFO;
static opt_info	  opt_vs		= EMPTY_OPT_INFO;
static opt_info	opt_N			= EMPTY_OPT_INFO;
static opt_info	opt_h			= EMPTY_OPT_INFO;
static opt_info	opt_c			= EMPTY_OPT_INFO;
static opt_info	opt_d			= EMPTY_OPT_INFO;
static opt_info	opt_D			= EMPTY_OPT_INFO;
static opt_info	opt_H			= EMPTY_OPT_INFO;
static opt_info	opt_V			= EMPTY_OPT_INFO;
static opt_info	opt_files		= EMPTY_OPT_INFO;


static parsed_opts		a_subopts[]	= { {
								OPT_NOT_MODAL,
								"R", "rom",

								"Read in a rom and modify it instead of creating a new one",
								{ "<file>", 1, 1, 1 },

								NULL,
								&opt_aR,
							  },
							  {
								OPT_NOT_MODAL,
								"s", "small",

								"Include a small ROM composed of the given PRC files.  The size "
								"of the small ROM may be specified using the option modifier.  "
								"e.g. '-l:<size>' or '--small:<size>'.",
								{ "<file>", 1, 1, ARGS_INFINITE },

								NULL,
								&opt_as
						      },
							  {
								OPT_NOT_MODAL,
								"l", "large",

								"Include a large ROM composed of the given PRC files.  The size "
								"of the large ROM may be specified using the option modifier.  "
								"e.g. '-l:<size>' or '--large:<size>'.",
								{ "<file>", 1, 1, ARGS_INFINITE },

								NULL,
								&opt_al,
							  },
							  {
								OPT_NOT_MODAL,
								"f", "format_small",

								"Generate the small ROM with the given format string",
								{ "<format>", 1, 1, 1 },

								NULL,
								&opt_af
						      },
							  {
								OPT_NOT_MODAL,
								"F", "format_big",

								"Generate the big ROM with the given format string",
								{ "<format>", 1, 1, 1 },

								NULL,
								&opt_aF
						      },
							  {
								OPT_NOT_MODAL,
								"o", "output",

								"Output the generated ROM image to the given <file>.",
								{ "<file>", 1, 1, 1 },

								NULL,
								&opt_ao
						      },
						      NULL_OPTS  };

static parsed_opts		x_subopts[]	= { {
								OPT_NOT_MODAL,
								"l", "large",

								"Extract PRC/databases from the large ROM.\nIf 1 or more "
								"<type.ctor> is provided, only extract PRC/databases which have "
								"a <type.ctor> in the provided list",
								{ "<type.ctor>", 1, 0, ARGS_INFINITE},

								NULL,
								&opt_xl,
							  },
							  {
								OPT_NOT_MODAL,
								"s", "small",

								"Extract PRC/databases from the small ROM.\nIf 1 or more "
								"<type.ctor> is provided, only extract PRC/databases which have "
								"a <type.ctor> in the provided list",
								{ "<type.ctor>", 1, 0, ARGS_INFINITE},

								NULL,
								&opt_xs,
						      },
						      NULL_OPTS  };

static parsed_opts		v_subopts[]	= { {
								OPT_NOT_MODAL,
								"l", "large",

								"View information about the large ROM.",
								NULL_ARG_INFO,

								NULL,
								&opt_vl,
							  },
							  {
								OPT_NOT_MODAL,
								"s", "small",

								"View information about the small ROM.",
								NULL_ARG_INFO,

								NULL,
								&opt_vs,
						      },
						      NULL_OPTS  };


static parsed_opts		opts[]		= {

							  { OPT_DEFAULT_ARGS,
								"i", "input",
								
								"Image(s) to process",
								{ "<image>", 1, 0, ARGS_INFINITE },

								NULL,
								&opt_files
							  },

							  /************************************************
							   * Modal options
							   */
							  {
								OPT_MODAL,
								"a", "assemble:create",

								"Assemble a new ROM from the given set of PRC database files "
								"(specified using the -l and/or -s suboption).  The ROM <version> "
								"may optionally be specified.",
								{ "<version>", 1, 0, 1 },

								a_subopts,
								&opt_a
						      },
							  {
								OPT_MODAL,
								"p", "PRC",

								"Read and interpret the given PRC/database image.",
								NULL_ARG_INFO,

								NULL,
								&opt_p
						      },
							  {
								OPT_MODAL,
								"r", "RAM",

								"Read and interpret the given RAM image (not fully implemented)",
								NULL_ARG_INFO,

								NULL,
								&opt_r
						      },
							  {
								OPT_MODAL,
								"v", "view",

								"View the given ROM image.",
								NULL_ARG_INFO,

								v_subopts,
								&opt_v
						      },
							  {
								OPT_MODAL,
								"x", "extractPRCs",

								"Extract all PRC/databases from the provided image <file>.",
								NULL_ARG_INFO,

								x_subopts,
								&opt_x
						      },

							  /************************************************
							   * Global/Non-modal options
							   */
							  {
								OPT_NOT_MODAL,
								"c", "showChunks",

								"Show all memory chunks",
								NULL_ARG_INFO,

								NULL,
								&opt_c
						      },
							  {
								OPT_NOT_MODAL,
								"d", "showPRC",

								"List all PRC/databases within the image",
								NULL_ARG_INFO,

								NULL,
								&opt_d
						      },
							  {
								OPT_NOT_MODAL,
								"D", "showPRCDetail",

								"Show the details of all PRC/databases within the image.  "
								"This option also accepts an option modifier which specifies the "
								"level of detail desired:"
								"\n.2:8.-D:1  is the same as '-d',"
								"\n.2:8.-D:2  will show the records/resources within a database,"
								"\n.2:8.-D:3  will display records/resources interpret the "
								"records/resources which are currently understood, dumping all "
								"others as RAW bytes,"
								"\n.2:8.-D:4  is the same as '-D:3' with a little more "
								"record/resource detail."
								"\n.2:8.-D:5  will dump all records/resources as RAW bytes."
								"\n.The default is '-D:2'.",
								NULL_ARG_INFO,

								NULL,
								&opt_D
						      },
							  {
								OPT_NOT_MODAL,
								"h", "showHeap",

								"Show the heap list and all heap headers",
								NULL_ARG_INFO,

								NULL,
								&opt_h
						      },
							  {
								OPT_NOT_MODAL,
								"H", "showHeadersOnly:headersOnly",

								"Show the card and storage headers only",
								NULL_ARG_INFO,

								NULL,
								&opt_H
						      },
							  {
								OPT_NOT_MODAL,
								"N", "showNV",

								"Show the System NV parameters (from the storage header)",
								NULL_ARG_INFO,

								NULL,
								&opt_N
						      },
							  {
								OPT_NOT_MODAL,
								"V", "Verbose",

								"Verbose output (equivalent to '-NDch')",
								NULL_ARG_INFO,

								NULL,
								&opt_V
						      },
						      NULL_OPTS  };

void	Usage(char*	stat)
{
	int		idex;
	char*	prog	= strrchr(progName, '/');
	char*	rest	= "[options] <image>...";

	if (! prog)
		prog = progName;
	else
		prog++;

	if (stat && *stat)
		opts_fprintf(stderr, opts,
		             "\nError: *** %s\n\nUsage: %s %s\n",
		             stat, prog, rest);
	else
		opts_fprintf(stderr, opts,
		             "\nUsage: %s %s\n", prog, rest);

	fprintf (stderr, " Available PalmOS versions are:\n   ");
	for (idex = 0; idex < sizeof(ROMVers)/sizeof(ROMVers[0]); idex++)
	{
		UInt16	major	= (ROMVers[idex].palmOSVer >> 8) & 0xFF;
		UInt16	minor	= ROMVers[idex].palmOSVer & 0xFF;

		fprintf (stderr, "%s%x.%x", (idex ? ", " : ""), major, minor);
	}
	fprintf (stderr, "\n\n");

	fflush (stderr);
}

int	Assemble	()
{
	/************************************************************
	 * Assemble a new ROM
	 */
	ROMPtr		pROM;
	int			hROMOut		= fileno(stdout);
	ROMVersion*	pVersion	= &(ROMVers[0]);
	char		error[128];
	int hROMIn = 0;
	
	if (opt_a.optarg_cnt > 0)
	{
		UInt16	major;
		UInt16	minor;
		UInt16	osver;

		sscanf(opt_a.optarg[0], "%hx.%hx", &major, &minor);
		osver = (major << 8) | minor;

		pVersion = LocateROMVerByOSVer(osver);
		if (! pVersion)
		{
			sprintf (error, "Unsupported PalmOS Version [%hx.%hx]",
					 major, minor);
			Usage(error);
			return(1);
		}
	}

	if ((opt_al.optidex) && (opt_al.optmod))
	{
		// strtoul can automatically deal with
		// hex '0xa', octal '012', or decimal '123'
		//
		// (We could also do special processing and provide any base
		//  between 2 and 36 to strtoul...)
		UInt32	Size = strtoul(opt_al.optmod, NULL, 0);
		if (Size)
		{
			pVersion->big_cardSize = Size;
		}
		else
		{
			sprintf (error, "Please specify the large ROM size (-l:<size>) as an integer");
			Usage (error);
			return 1;
		}
	}

	if ((opt_as.optidex) && (opt_as.optmod))
	{
		// strtoul can automatically deal with
		// hex '0xa', octal '012', or decimal '123'
		//
		// (We could also do special processing and provide any base
		//  between 2 and 36 to strtoul...)
		UInt32	Size = strtoul(opt_as.optmod, NULL, 0);
		if (Size)
		{
			pVersion->small_cardSize = Size;
		}
		else
		{
			sprintf (error, "Please specify the small ROM size (-s:<size>) as an integer");
			Usage (error);
			return 1;
		}
	}

	if (opt_af.optidex)
	{
		pVersion->small_layout		= opt_af.optarg[0];
		pVersion->small_addLayout	= opt_af.optarg[0];
	}

	if (opt_aF.optidex)
	{
		pVersion->big_layout	= opt_aF.optarg[0];
		pVersion->big_addLayout = opt_aF.optarg[0];
	}

	if (opt_ao.optidex)
	{
		if ((hROMOut = open(opt_ao.optarg[0], O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)
		{
			fprintf (stderr, "*** Error opening ROM output file [%s]: %d - %s\n",
							opt_ao.optarg[0], errno, sys_errlist[errno]);
			return(0);
		}
	}

	if (opt_aR.optidex)
	{
		/*
		 * Open the ROM file
		 */
		hROMIn = open(opt_aR.optarg[0], O_RDONLY);
		if (hROMIn < 0)
		{
			fprintf (stderr, "*** Cannot open ROM file '%s'\n", 
							opt_aR.optarg[0]);
			return(1);
		}
	}
	
	if (opt_as.optidex || hROMIn)
	{
		if (hROMIn)
		{
			pROM = ReadROM (hROMIn, RT_SMALL);
		}
		else
		{
			pROM = InitializeROM(pVersion, RT_SMALL);
		}
			
		if (pROM)
		{
			if (AddPRCs (pROM, opt_as.optarg_cnt, opt_as.optarg, 0) != opt_as.optarg_cnt)
			{
				fprintf (stderr, "*** Error adding PRCs to small ROM\n");
				FreeROM (pROM);
				return (0);
			}

			//Output_ROM(pROM, (UInt32)(pROM->pROM),
			//           bShowNV, bShowHeap, bShowChunks,bShowDB);

			fprintf (stderr, "\n");

			if (! SetSystem(pROM))
			{
				fprintf (stderr, "*** Error setting up small ROM boot code initialization vectors\n");
				FreeROM (pROM);
				return (0);
			}

			if (! WriteROM(pROM, hROMOut))
			{
				fprintf (stderr, "*** Error writing small ROM\n");
			}
			fprintf (stderr, "\n");

			FreeROM(pROM);
		}
		else if (! hROMIn)
		{
			fprintf (stderr, "*** Error creating small ROM\n");
		}
	}
		
	if (opt_al.optidex || hROMIn)
	{
		if (hROMIn)
		{
			pROM = ReadROM (hROMIn, RT_LARGE);
		}
		else
		{
			pROM = InitializeROM(pVersion, RT_LARGE);
		}

		if (pROM)
		{
			if (opt_as.optidex)
			{
				pROM->File_base = pROM->pVersion->card_bigROMOffset;
			}

			if (AddPRCs (pROM, opt_al.optarg_cnt, opt_al.optarg, 0) != opt_al.optarg_cnt)
			{
				fprintf (stderr, "*** Error adding PRCs to large ROM\n"
								 "*** Try using -l:<size> to increase the available space\n");
				FreeROM (pROM);
				return (0);
			}
	
			//Output_ROM(pROM, (UInt32)(pROM->pROM),
			//           bShowNV, bShowHeap, bShowChunks,bShowDB);

			fprintf (stderr, "\n");

			if (! SetSystem(pROM))
			{
				fprintf (stderr, "*** Error setting up large ROM boot code initialization vectors\n");
				FreeROM (pROM);
				return (0);
			}

			if (! WriteROM(pROM, hROMOut))
			{
				fprintf (stderr, "*** Error writing large ROM\n");
			}
			fprintf (stderr, "\n");

			FreeROM(pROM);
		}
		else if (hROMIn)
		{
			fprintf (stderr, "*** Error creating large ROM\n");
		}
	}

	return(0);
}

int	ViewRAM()
{
	int		idex;
	UInt32	RRFlags			= 0;
	char	error[128];
	UInt16	bShowNV			= (UInt16)(opt_N.optidex !=0||opt_V.optidex!=0);
	UInt16	bShowHeap		= (UInt16)(opt_h.optidex !=0||opt_V.optidex!=0);
	UInt16	bShowChunks		= (UInt16)(opt_c.optidex !=0||opt_V.optidex!=0);
	UInt16	bShowDB			= 0;

	SET_D(bShowDB);


	if (! opt_files.optidex)
	{
		sprintf (error,"You must provided at least 1 file for processing.");
		Usage(error);
		return(1);

	}

	for (idex = 0; idex < opt_files.optarg_cnt; idex++)
	{
		ROMPtr	pRAM;
		int		hRAM;
		char*	pRAMName		= opt_files.optarg[idex];
		/*
		 * Open the RAM file
		 */
		hRAM = open(pRAMName, O_RDONLY);
		if (hRAM < 0)
		{
			fprintf (stderr, "*** Cannot open RAM file '%s'\n", pRAMName);
			continue;
		}

		fprintf (stdout, "Processing '%s'\n", pRAMName);

		/************************************************************
		 * View a RAM image
		 */
		fprintf (stdout, "\n======= RAM Image =======\n");
		if ((pRAM = ReadRAM(hRAM, RRFlags)))
		{
			fprintf (stdout, "   PTR_base  [0x%08lX]\n", (UInt32)pRAM->pROM);
			fprintf (stdout, "   ROM_base  [0x%08lX]\n", pRAM->ROM_base);
			fprintf (stdout, "   Card_base [0x%08lX]\n", pRAM->Card_base);
			fprintf (stdout, "   File_base [0x%08lX]\n", pRAM->File_base);
			Output_ROM(pRAM, (UInt32)(pRAM->pROM),
			           bShowNV, bShowHeap, bShowChunks, bShowDB);

#if	0
			if (bExtract)
				WritePRCs(pROM, NULL, 0);
#endif

			FreeROM(pRAM);
		}
		else
		{
			fprintf (stdout, "  *** Cannot read the RAM image from '%s'\n",
			         pRAMName);
		}

		close(hRAM);
	}

	return (0);
}

int	ViewPRC()
{
	/************************************************************
	 * View a PRC/database file
	 */
	PRCPtr	pPRC;
	int		idex;
	char	error[128];
	UInt16	bShowDB			= 0;

	SET_D(bShowDB);
	if (bShowDB)	bShowDB++;

	if (! opt_files.optidex)
	{
		sprintf (error,"You must provided at least 1 file for processing.");
		Usage(error);
		return(1);
	}

	for (idex = 0; idex < opt_files.optarg_cnt; idex++)
	{
		if ((pPRC = ReadPRC(opt_files.optarg[idex])))
		{
			Output_DatabaseHdr(NULL, (UInt32)pPRC->pDB + pPRC->nBytes, 
							   pPRC->pDB, (UInt32)pPRC->pDB, 0, bShowDB);
		}
		else
		{
			fprintf (stderr, "*** Error reading PRC [%s]\n", opt_files.optarg[idex]);
		}

		FreePRC(pPRC);
	}

	return(0);
}

int	Extract()
{
	int		idex;
	UInt32	RRFlags			= 0;
	UInt16	bShowNV			= (UInt16)(opt_N.optidex != 0 || opt_V.optidex !=0);
	UInt16	bShowHeap		= (UInt16)(opt_h.optidex != 0 || opt_V.optidex !=0);
	UInt16	bShowChunks		= (UInt16)(opt_c.optidex != 0 || opt_V.optidex !=0);
	UInt16	bExtract		= (UInt16)(opt_x.optidex != 0);
	UInt16	bShowDB			= 0;
	UInt16	bSmallROM		= 0;
	UInt16	bLargeROM		= 0;
	char	error[128];

	SET_D(bShowDB);

	if (! opt_files.optidex)
	{
		sprintf (error, "You must provided at least 1 file for processing.");
		Usage(error);
		return(1);

	}

	if (opt_x.optidex)
	{
		bSmallROM = (UInt16)(opt_xs.optidex != 0);
		bLargeROM = (UInt16)(opt_xl.optidex != 0);
	}
	else if (opt_v.optidex)
	{
		bSmallROM = (UInt16)(opt_vs.optidex != 0);
		bLargeROM = (UInt16)(opt_vl.optidex != 0);
	}

	if (! bSmallROM && ! bLargeROM)
	{
		bSmallROM = 1;
		bLargeROM = 1;
	}

	for (idex = 0; idex < opt_files.optarg_cnt; idex++)
	{
		ROMPtr	pROM;
		int		hROM;
		char*	pROMName		= opt_files.optarg[idex];

		/*
		 * Open the ROM file
		 */
		hROM = open(pROMName, O_RDONLY);
		if (hROM < 0)
		{
			fprintf (stderr, "*** Cannot open ROM file '%s'\n", pROMName);
			return(1);
		}

		fprintf (stdout, "Processing '%s'\n", pROMName);

		/*
		 * Process the Small ROM if requested.
		 */
		if (bSmallROM)
		{
			/************************************************************
			 * View/Extract a small ROM
			 */
			fprintf (stdout, "\n======= Small ROM =======\n");
			if ((pROM = ReadROM(hROM, RRFlags | RT_SMALL)))
			{
				//fprintf (stdout, "   PTR_base  [0x%08lX]\n", (UInt32)pROM->pROM);
				fprintf (stdout, "   ROM_base  [0x%08lX]\n", pROM->ROM_base);
				fprintf (stdout, "   Card_base [0x%08lX]\n", pROM->Card_base);
				fprintf (stdout, "   File_base [0x%08lX]\n", pROM->File_base);
				Output_ROM(pROM, (UInt32)(pROM->pROM),
				           bShowNV, bShowHeap, bShowChunks, bShowDB);

				if (bExtract)
				{
					TypeCtorPtr	pTCList		= NULL;
					UInt32		nEntries	= 0;

					if ((opt_xs.optarg_cnt > 0) &&
						((pTCList = StrList2TypeCtorList(opt_xs.optarg,
														 opt_xs.optarg_cnt))
						            != NULL))
					{
						nEntries = opt_xs.optarg_cnt;
					}

					WritePRCs(pROM, pTCList, nEntries);

					if (nEntries)
						free(pTCList);
				}

				FreeROM(pROM);
			}
			else
			{
				fprintf (stdout, "  *** No Small ROM exists in '%s'\n",
				         pROMName);
			}
		}


		if (bLargeROM)
		{
			/************************************************************
			 * View/Extract a large ROM
			 */
			fprintf (stdout, "\n======= Large ROM =======\n");
			if ((pROM = ReadROM(hROM, RRFlags | RT_LARGE)))
			{
				//fprintf (stdout, "   PTR_base  [0x%08lX]\n", (UInt32)pROM->pROM);
				fprintf (stdout, "   ROM_base  [0x%08lX]\n", pROM->ROM_base);
				fprintf (stdout, "   Card_base [0x%08lX]\n", pROM->Card_base);
				fprintf (stdout, "   File_base [0x%08lX]\n", pROM->File_base);

				//Card_base - ROM_base == 0x8000

				Output_ROM(pROM, (UInt32)(pROM->pROM) - pROM->File_base,
								         //(pROM->Card_base - pROM->ROM_base),
				           bShowNV, bShowHeap, bShowChunks, bShowDB);

				if (bExtract)
				{
					TypeCtorPtr	pTCList		= NULL;
					UInt32		nEntries	= 0;

					if ((opt_xl.optarg_cnt > 0) &&
						((pTCList = StrList2TypeCtorList(opt_xl.optarg,
														 opt_xl.optarg_cnt))
						            != NULL))
					{
						nEntries = opt_xl.optarg_cnt;
					}

					WritePRCs(pROM, pTCList, nEntries);

					if (nEntries)
						free(pTCList);
				}

				FreeROM(pROM);
			}
			else
			{
				fprintf (stdout, "  *** No Large ROM exists in '%s'\n",
				         pROMName);
			}
		}

		close(hROM);
	}

	return (0);
}

int	ViewROM()
{
	return Extract();
}

int	main	(int	argc,
			 char*	argv[])
{
	char*	stat;

	progName = argv[0];

	if (argc < 2)
	{
		Usage(NULL);
		return(1);
	}

	if ((stat = opts_parse(opts, argc, argv)) != NULL)
	{
		Usage(stat);
		return(1);
	}

	if (opt_a.optidex)
		return Assemble();
	else if (opt_r.optidex)
		return ViewRAM();
	else if (opt_p.optidex)
		return ViewPRC();
	else if (opt_x.optidex)
		return Extract();
	else
	{
		if (! opt_v.optidex)
			opt_v.optidex = -1;

		return ViewROM();
	}

	return(1);
}
