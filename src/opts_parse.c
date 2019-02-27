/*****************************************************************************
 * opts_parse -- command line option parsing and help output
 *
 * NOTE: Edit this file with tabstop=4 !
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * See file COPYING for information on distribution conditions.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "opts_parse.h"

/*
 * opts_cols defines the column sizes for output
 * 	column 1	== initial indentation
 * 	column 2	== option/argument area
 * 	column 3	== separator
 * 	column 4	== description
 */
int				opts_cols[]		= { 2, 28, 2, 47 };


static char		__opts_error[128];

/*****************************************************************************
 * Local work-horse routines
 */

static int
__opt_short		(char*	pOptStr,
				 char*	pArg)
{
	if (! pOptStr || ! pArg)
		return 0;

	while (*pOptStr)
	{
		if (*pOptStr == *pArg)
			return 1;

		pOptStr++;
	}

	return 0;
}

static int
__opt_long		(char*	pOptStr,
				 char*	pArg)
{
	static char	OptStr[64];
	       int	ArgLen;
		   int	CmpLen;

	if (! pOptStr || ! pArg)
		return 0;

	strcpy(OptStr, pOptStr);
	pOptStr = OptStr;
	ArgLen  = strlen(pArg);
	while (pOptStr)
	{
		char*	pNxt	= (char*)strchr(pOptStr, ':');
		if (pNxt)
		{
			*pNxt = '\0';
			pNxt++;
		}

		CmpLen = strlen(pOptStr);
		if (ArgLen < CmpLen)
			CmpLen = ArgLen;
		if (! strncmp(pOptStr, pArg, CmpLen))
			return 1;

		pOptStr = pNxt;
	}

	return 0;
}

/*
 * See if 'pOpt' or any of it's sub-options know about 'arg'
 */
static parsed_opts*
__opt_parse		(parsed_opts*	pOpt,
				 int			bShortArg,
				 int			nArg,
				 char*			argv[],
				 char*			arg)
{
	/*
	 * Is this a short or long argument?
	 */
	int	bHandled	= 0;

	if (! pOpt || ! pOpt->status || ! arg || ! *arg)
		return NULL;

	if (bShortArg)
	{
		if (__opt_short(pOpt->shortopts, arg))
			bHandled = 1;
	}
	else
	{
		if (__opt_long(pOpt->longopts, arg))
			bHandled = 1;
	}

	if (! bHandled)
	{
		parsed_opts*	pSubOpts	= pOpt->subopts;
		int	idex;

		pOpt = NULL;
		for (idex = 0; pSubOpts && ! NULL_OPT(pSubOpts[idex]); idex++)
		{
			if ((pOpt = __opt_parse(&(pSubOpts[idex]), bShortArg,
			                        nArg, argv, arg)) != NULL)
				break;
		}
	}
	else if (pOpt)
	{
		if (pOpt->status->optidex)
		{
			/* We've already seen this option */
			pOpt->status->optidex = -1;
		}
		else
		{
			/* We have consumed this argument */
			pOpt->status->optidex    = nArg;
			pOpt->status->opt        = argv;
			pOpt->status->optmod     = NULL;
			pOpt->status->optarg     = NULL;
			pOpt->status->optarg_cnt = 0;

			/* Save any option modifier */
			if (bShortArg)
			{
				if ((*(arg+1) == ':') && (*(arg+2)))
					pOpt->status->optmod = arg+2;
			}
			else
			{
				char*	pCol	= strchr(arg, ':');
				if (pCol && (*(pCol+1)))
					pOpt->status->optmod = pCol+1;
			}
		}
	}

	return (pOpt);
}

/*
 * Verify that supplied arguments have the proper
 * number of arguments
 */
static char*
__opts_check_args	(parsed_opts	opts[])
{
	int		idex;
	char*	pReturn	= NULL;

	for (idex = 0; ! NULL_OPT(opts[idex]); idex++)
	{
		if (! opts[idex].status || ! opts[idex].status->optidex)
			/* We didn't see this argument */
			continue;

		if ((opts[idex].args.mincnt > 0) &&
			(opts[idex].status->optarg_cnt < (opts[idex].args.mincnt *
											  opts[idex].args.argblk)))
		{
			/* Not enough arguments for this option! */
			sprintf(__opts_error,
			        "Not enough arguments for option [%s]:  %ld < %ld",
			        opt_str(&(opts[idex])),
					opts[idex].status->optarg_cnt,
					(opts[idex].args.mincnt * opts[idex].args.argblk));
			pReturn = __opts_error;
		}

		if ((opts[idex].args.argblk > 1) &&
		    (opts[idex].status->optarg_cnt % opts[idex].args.argblk))
		{
			/* We do NOT have full argument blocks */
			sprintf(__opts_error,
			        "Arguments for option [%s] must be in blocks of %ld: %ld left-overs.",
			        opt_str(&(opts[idex])),
					opts[idex].args.argblk,
					opts[idex].status->optarg_cnt % opts[idex].args.argblk);
			pReturn = __opts_error;
		}

		if (opts[idex].subopts)
			pReturn = __opts_check_args(opts[idex].subopts);
	}

	return pReturn;
}

/*
 * Check all arguments 'argv' against our options list 'opts'
 */
static char*
__opts_parse	(parsed_opts	opts[],
				 int			level,
				 int			argc,
				 char**			argv)
{
	int				idex;
	int				jdex;
	int				odex;
	int				bConsumed	= 0;
	int				bRestDef	= 0;
	parsed_opts*	pCurOpt		= NULL;
	parsed_opts*	pOpted		= NULL;
	parsed_opts*	pArgOpt		= NULL;
	parsed_opts*	pDefOpt		= NULL;

	if (! opts || (argc < 2) || ! argv)
		return 0;

	if (opts[0].flags & OPT_DEFAULT_ARGS)
		pDefOpt = &(opts[0]);

	for (idex = 1; idex < argc; idex++)
	{
		char*	arg			= argv[idex];
		int		bShortArg	= 1;
		int		argStep		= 1;

		if ((*arg != '-') || (bRestDef))
		{
			/* This argument is NOT a flag */
			if ((! bRestDef) && (pArgOpt))
			{
				/* Consumable non-flag argument */
				if (! pArgOpt->status->optarg)
				{
					/* First argument for this option */
					pArgOpt->status->optarg     = &(argv[idex]);
					pArgOpt->status->optarg_cnt++;
					continue;
				}

				if (pArgOpt->status->optarg_cnt >= (pArgOpt->args.maxcnt *
				                                    pArgOpt->args.argblk))
				{
					/* Too many arguments */
					pArgOpt = NULL;
				}
				else
				{
					pArgOpt->status->optarg_cnt++;
					continue;
				}
			}

			/* Non-consumable non-flag argument */
			if (! pDefOpt || ! pDefOpt->status)
			{
				/* ERROR -- no default argument option */
				sprintf(__opts_error,
				        "Unexpected non-option argument: %s",
				        argv[idex]);
				return __opts_error;
			}

			if (! pDefOpt->status->optidex)
			{
				pDefOpt->status->optidex = idex;
				pDefOpt->status->opt     = &(argv[idex]);
				pDefOpt->status->optarg  = &(argv[idex]);
			}

			if (idex > pDefOpt->status->optidex + pDefOpt->status->optarg_cnt)
			{
				/* We cannot deal with non-contiguous default arguments... */
				sprintf(__opts_error,
				        "Non-contiguous non-option argument: %s",
				        argv[idex]);
				return __opts_error;
			}

			pDefOpt->status->optarg_cnt++;
			continue;
		}

#if	0
		if (pDefOpt && pDefOpt->status && pDefOpt->status->optidex)
		{
			sprintf(__opts_error,
			        "Unexpected option argument: %s",
			        argv[idex]);
			return __opts_error;
		}
#endif

		/*******************************************************
		 * See if this is one of the options that we
		 * handle by default...
		 */
		if (pDefOpt && pDefOpt->status && ! strcmp(arg, "--"))
		{
			/* The default argument should consume the remainder of arguments */
			bRestDef = 1;
			continue;
		}

		if (! strcmp(arg, "--help") || ! strcmp(arg, "-?"))
		{
			__opts_error[0] = '\0';
			return __opts_error;
		}
		/*******************************************************/

		/* Reset our argument consuming option */
		pArgOpt = NULL;

		bShortArg = 1;
		arg++;
		if (*arg == '-')
		{
			/* long argument */
			arg++;
			bShortArg = 0;
			argStep   = strlen(arg);
		}

		for (jdex = 0;jdex < strlen(arg);jdex += argStep)
		{
			if (*(arg+jdex) == ':')
				break;

			if (pCurOpt)
			{
				/*
				 * We have a "current option mode"
				 */
				if ((pOpted = __opt_parse(pCurOpt, bShortArg, idex,
				                          &(argv[idex]), arg+jdex)) != NULL)
				{
					if (pOpted->status->optidex < 0)
					{
						sprintf(__opts_error,
						        "Duplicate option: %s", arg);
						return __opts_error;
					}

					if (HAS_ARGS(pOpted))
						pArgOpt = pOpted;

					continue;
				}
			}

			/*
			 * We have yet to consume this argument -- check all options
			 * 	1) check all non-modal options
			 */
			bConsumed = 0;
			for (odex = 0; ! NULL_OPT(opts[odex]); odex++)
			{
				if (opts[odex].flags & OPT_MODAL)
					/* Ignore Modal options for this pass... */
					continue;

				if ((pOpted = __opt_parse(&(opts[odex]), bShortArg, idex,
				                          &(argv[idex]), arg+jdex)) != NULL)
				{
					if (pOpted->status->optidex < 0)
					{
						sprintf(__opts_error,
						        "Duplicate option: %s", arg);
						return __opts_error;
					}

					/* We have consumed this argument */
					pCurOpt = &(opts[odex]);

					if (HAS_ARGS(pOpted))
						pArgOpt = pOpted;
					bConsumed = 1;
					break;
				}
			}

			if (bConsumed)
				continue;


			/*
			 * We still have not consumed this argument...
			 * 	2) if the current option is NOT modal, check all modal options
			 */
			if (pCurOpt && (pCurOpt->flags & OPT_MODAL))
			{
				/* The current option is modal -- we cannot continue! */
				sprintf(__opts_error,
				        "Invalid option [%s] with modal option [%s]",
				        argv[idex],
						opt_str(pCurOpt));

				return __opts_error;
			}

			for (odex = 0; ! NULL_OPT(opts[odex]); odex++)
			{
				if (! (opts[odex].flags & OPT_MODAL))
					/*
					 * Non-Modal options are ignored -- they've already been
					 *                                  checked
					 */
					continue;


				if ((pOpted = __opt_parse(&(opts[odex]), bShortArg, idex,
				                          &(argv[idex]), arg+jdex)) != NULL)
				{
					if (pOpted->status->optidex < 0)
					{
						sprintf(__opts_error,
						        "Duplicate option: %s",
						        arg);
						return __opts_error;
					}

					/* We have consumed this argument */
					pCurOpt = &(opts[odex]);

					if (HAS_ARGS(pOpted))
						pArgOpt = pOpted;
					bConsumed = 1;
					break;
				}
			}

			if (bConsumed)
				continue;


			/*
			 * This is a flag that we cannot consume.
			 * 	ERROR
			 */
			sprintf(__opts_error,"Invalid/Unknown option: %s",
			        argv[idex]);
			return __opts_error;
		}
	}

	/*
	 * Final validation -- ensure that all options which may have
	 *                     arguments have the minimum number required.
	 */
	if (__opts_check_args(opts))
		return __opts_error;

	return NULL;
}

/*
 * Generate a string to represent the arguments of the given option.
 */
static
char*	__opt_arg_str	(parsed_opts*	pOpt)
{
	static char	arg_str[128];
	static char	numarg_str[16];

	if (! pOpt || ! HAS_ARGS(pOpt))
		return NULL;

	arg_str[0]    = '\0';
	numarg_str[0] = '\0';
	if (pOpt->args.maxcnt > 0)
	{
		if (pOpt->args.mincnt > 0)
		{
			int		nWS	= 0;
			char*	pWS	= strchr(pOpt->args.argname, ' ');
			while (pWS)
			{
				nWS++;
				pWS	= strchr(pWS+1, ' ');
			}

			nWS++;	// Count EOL as white-space


			if (pOpt->args.maxcnt == ARGS_INFINITE)
			{
				if (nWS < (pOpt->args.mincnt * pOpt->args.argblk))
				{
					if (pOpt->args.argblk > 1)
						sprintf(numarg_str, "{%ld:%ld,*}", pOpt->args.argblk,
						                                   pOpt->args.mincnt);
					else
						sprintf(numarg_str, "{%ld,*}", pOpt->args.mincnt);
				}
				else
					strcpy(numarg_str, "...");
			}
			else
			{
				if (nWS < (pOpt->args.maxcnt * pOpt->args.argblk))
				{
					if (pOpt->args.argblk > 1)
						sprintf(numarg_str, "{%ld:%ld,%ld}", pOpt->args.argblk,
						                                     pOpt->args.mincnt,
						                                     pOpt->args.maxcnt);
					else
						sprintf(numarg_str, "{%ld,%ld}", pOpt->args.mincnt,
						                                 pOpt->args.maxcnt);

				}
			}

		}
		else
			strcpy(numarg_str, "...");
	}

	if (HAS_OPTIONAL_ARGS(pOpt))
		strcat(arg_str, "[");

	if (pOpt->args.argname)
		strcat(arg_str, pOpt->args.argname);
	else
		strcat(arg_str, "arg");

	strcat(arg_str, numarg_str);

	if (HAS_OPTIONAL_ARGS(pOpt))
		strcat(arg_str, "]");

	return arg_str;
}

/*
 * Output the single given option
 */
static
void	__opt_print	(int			hFile,
					 parsed_opts*	pOpt,
					 int			cols[])
{
	char*	sopt		= NULL;
	char*	lopt		= NULL;
	char*	nxt_lopt	= NULL;
	char*	opt			= NULL;
	char*	nxt_opt		= NULL;
	int		optIdent	= 0;
	char*	desc		= NULL;
	char*	nxt_desc	= NULL;
	int		indent		= 0;
	int		indent_nxt	= -1;
	int		indent_hang	= -1;
	int		bHaveShort	= 0;
	int		bHaveLong	= 0;
	char	out_opt[64];

	if (! pOpt)
		return;

	sopt = pOpt->shortopts;
	if (pOpt->longopts)
	{
		lopt = (char*)malloc(strlen(pOpt->longopts)+1);
		if (! lopt)
			return;
		strcpy(lopt, pOpt->longopts);
		nxt_lopt = lopt;
	}
	if (pOpt->description)
	{
		desc = (char*)malloc(strlen(pOpt->description)+1);
		if (! desc)
		{
			if (lopt)	free(lopt);
			return;
		}
		strcpy(desc, pOpt->description);
		nxt_desc = desc;
	}

	/********************************************************************* 
	 *
	 * Output format:
	 *	'  -o, --option <arg>           Option Description'
	 *	  [3........................28][31..... ... ....80]
	 */
	while ((sopt && *sopt) || (lopt && *lopt) ||
	       (opt  && *opt)  || (desc && *desc))
	{
		if (! nxt_opt || ! *nxt_opt)
		{
			bHaveShort	= 0;
			bHaveLong	= 0;
			out_opt[0]	= '\0';

			/* append the current shortoption */
			if (sopt && *sopt)
			{
				char	out[3]	= { '-', 0, 0 };
				out[1] = *sopt;

				strcat(out_opt, out);
				sopt++;
				bHaveShort = 1;

				if (nxt_lopt && *nxt_lopt)
					strcat(out_opt, ", ");
			}
			else
			{
				strcat(out_opt, "    ");
			}

			/* append the current longoption */
			if (lopt && *lopt)
			{
				lopt = nxt_lopt;

				if (lopt)
				{
					if ((nxt_lopt = (char*)strchr(nxt_lopt, ':')) != NULL)
					{
						*nxt_lopt = '\0';
						nxt_lopt++;
					}

					bHaveLong = 1;

					strcat(out_opt, "--");
					strcat(out_opt, lopt);
				}
			}

			/* append the argument name (if provided) */
			if (bHaveShort || bHaveLong)
			{
				char*	arg_str	= __opt_arg_str(pOpt);

				if (arg_str)
				{
					strcat(out_opt, " ");
					strcat(out_opt, arg_str);
				}

				nxt_opt = out_opt;
			}
			else
			{
				opt     = NULL;
				nxt_opt = NULL;
			}

			optIdent = 0;
		}
		else if (optIdent)
		{
			char	hold[128];
			int		thisIdent	= optIdent;
			int		leftOver	= cols[1] - (thisIdent + strlen(nxt_opt));

			strcpy(hold, nxt_opt);
			
			if (leftOver < 0)
			{
				thisIdent += leftOver;
			}

			sprintf (out_opt, "%*s%s", thisIdent, " ", hold);
			nxt_opt = out_opt;
		}

		/* Prepare the option - split out the first cols[1] chars */
		if (nxt_opt && *nxt_opt)
		{
			opt = nxt_opt;

			if (opt && (strlen(opt) > cols[1]))
			{
				char	c	= opt[cols[1]];
				opt[cols[1]] = '\0';
				nxt_opt = (char*)strrchr(opt, ' ');
				opt[cols[1]] = c;

				if (nxt_opt)
				{
					*nxt_opt = '\0';
					nxt_opt++;

					if (! optIdent)
					{
						char*	pSpace = (char*)strchr(opt, ' ');

						if (pSpace && bHaveLong)
							pSpace = (char*)strchr(pSpace+1, ' ');

						if (pSpace)
							optIdent = (unsigned long)(pSpace+1) -
							           (unsigned long)opt;
					}
				}
			}
			else
			{
				nxt_opt = NULL;
			}
		}

		/* Prepare the description - split out the first cols[3] chars */
		if (desc && *desc)
		{
			char*	pNL		= NULL;

			desc = nxt_desc;

			if (indent_nxt != -1)
			{
				indent      = indent_nxt;
				indent_nxt  = -1;
			}
			else if (indent_hang != -1)
			{
				indent      = indent_hang;
				indent_hang = -1;
			}

			if (desc && (strlen(desc)+indent > cols[3]))
			{
				char	c	= desc[cols[3]-indent];
				desc[cols[3]-indent] = '\0';
				nxt_desc = (char*)strrchr(desc, ' ');
				desc[cols[3]-indent] = c;

				if (nxt_desc)
				{
					*nxt_desc = '\0';
					nxt_desc++;
				}
			}
			else
			{
				nxt_desc = NULL;
			}

			/* Allow '\n' in a description string (and handle it properly ;^) */
			if (desc && ((pNL = strchr(desc, '\n')) != NULL))
			{
				/* We have a '\n'! */
				if (nxt_desc)
				{
					nxt_desc--;
					*nxt_desc = ' ';
				}

				nxt_desc = pNL;
				*nxt_desc = '\0';
				nxt_desc++;

				/* Are the formatting instructions? */
				if (*nxt_desc == '.')
				{
					/* YES! - Formatting = '.<indent>[:<hanging indent>].' */
					indent_nxt  = 0;
					indent_hang = 0;


					nxt_desc++;
					indent_nxt = strtoul(nxt_desc, &nxt_desc, 0);

					if (nxt_desc && (*nxt_desc == ':'))
					{
						nxt_desc++;
						indent_hang = strtoul(nxt_desc, &nxt_desc, 0);

						if (nxt_desc && (*nxt_desc == '.'))
							nxt_desc++;
					}
				}
			}
		}


		if ((opt && *opt) || (desc && *desc))
		{
			char	opt_line[256];

			sprintf(opt_line, "%*s%-*s%*s",
							cols[0],        " ",
							cols[1],        (opt && *opt ? opt : ""),
							cols[2],        " ");
			write  (hFile, opt_line, strlen(opt_line));

			if (indent)
			{
				sprintf(opt_line, "%*s", indent, " ");
				write  (hFile, opt_line, strlen(opt_line));
			}
							
			sprintf(opt_line, "%-*s\n",
							cols[3]-indent, (desc && *desc ? desc : "..."));
			write  (hFile, opt_line, strlen(opt_line));
		}
	}

	if (lopt)	free(lopt);
	if (desc)	free(desc);
}

/*
 * Output usage information for the give 'parse_opts'
 */
static
void	__opts_vprintf	(int			hFile,
						 parsed_opts	opts[],
						 parsed_opts*	pParent,
						 int			level,
						 char*			fmt,
						 va_list		args)
{
	int		idex;
	char	opt_line[256];
	int		bFirst		= 1;
	int		cols[4];

	/* Copy in the global column settings */
	memcpy(cols, opts_cols, sizeof(cols));

	if (level)
	{
		if (cols[3] > 25)
		{
			cols[0] += level*2;
			cols[3] -= level*2;
		}
	}
	else if (fmt && args)
	{
		vsprintf(opt_line, fmt, args);
		write   (hFile, opt_line, strlen(opt_line));
	}

	/*
	 * First, output all modal options
	 */
	bFirst = 1;
	for (idex = 0; ! NULL_OPT(opts[idex]); idex++)
	{
		if (! (opts[idex].flags & OPT_MODAL))
			continue;

		if (bFirst)
		{
			write (hFile, "\n", 1);
			if (! pParent)
			{
				sprintf(opt_line, "Modal Options:\n");
			}
			else
			{
				char*	parent	= opt_str(pParent);

				sprintf(opt_line, "%*sModal Suboptions%s%s:\n",
				        cols[0]-1, " ", (parent ? " for " : ""),
						                (parent ? parent : ""));
			}
			write  (hFile, opt_line, strlen(opt_line));
			bFirst = 0;
		}

		__opt_print(hFile, &(opts[idex]), cols);

		if (opts[idex].subopts)
		{
			/* Recurse */
			__opts_vprintf(hFile, opts[idex].subopts, &(opts[idex]),
			               level+1, fmt, args);
			write(hFile, "\n", 1);
		}
	}

	/*
	 * Second, output all non-modal options
	 */
	bFirst = 1;
	for (idex = 0; ! NULL_OPT(opts[idex]); idex++)
	{
		if (opts[idex].flags & OPT_MODAL)
			continue;

		if (bFirst)
		{
			write (hFile, "\n", 1);
			if (! pParent)
			{
				sprintf(opt_line, "Global Options:\n");
			}
			else
			{
				char*	parent	= opt_str(pParent);

				sprintf(opt_line, "%*sNon-Modal Suboptions%s%s:\n",
				        cols[0]-1, " ", (parent ? " for " : ""),
						                (parent ? parent : ""));
			}
			write  (hFile, opt_line, strlen(opt_line));
			bFirst = 0;
		}

		__opt_print(hFile, &(opts[idex]), cols);

		if (opts[idex].subopts)
		{
			/* Recurse */
			__opts_vprintf(hFile, opts[idex].subopts, &(opts[idex]),
			               level+1, fmt, args);
			write(hFile, "\n", 1);
		}
	}

	if (! level)
	{
		/*
		 * Output our help/usage option.
		 */
		sprintf(opt_line, "%*s%-*s%*s%-*s\n\n",
						cols[0], " ",
						cols[1], "-?, --help",
						cols[2], " ",
						cols[3], "This help/usage message.");
		write  (hFile, opt_line, strlen(opt_line));
	}
}

/*****************************************************************************
 * Public interfaces
 */
char*	opt_str	(parsed_opts*	pOpt)
{
	static char	str[128];

	if (! pOpt)
		return NULL;

	str[0] = '\0';

	if (pOpt->shortopts)
	{
		strcpy(str, "-");
		strcat(str, pOpt->shortopts);
		if (pOpt->longopts)
			strcat(str, "|");
	}

	if (pOpt->longopts)
	{
		strcat(str, "--");
		strcat(str, pOpt->longopts);
	}

	return str;
}

char*	opts_parse		(parsed_opts	opts[],
						 int			argc,
						 char**			argv)
{
	return __opts_parse(opts, 0, argc, argv);
}

void	opts_vprintf	(int			hFile,
						 parsed_opts	opts[],
						 char*			fmt,
						 va_list		args)
{
	__opts_vprintf(hFile, opts, NULL, 0, fmt, args);
}

void	opts_printf		(int			hFile,
						 parsed_opts	opts[],
						 char*			fmt,
						 ...)
{
	va_list	args;
	va_start(args, fmt);

	opts_vprintf(hFile, opts, fmt, args);

	va_end(args);
}

void	opts_fprintf	(FILE*			pFile,
						 parsed_opts	opts[],
						 char*			fmt,
						 ...)
{
	va_list	args;
	va_start(args, fmt);

	opts_vprintf(fileno(pFile), opts, fmt, args);

	va_end(args);
}

#ifdef	TEST
/******************************************************************************
 * Test mode
 */
opt_info	opt_a3			= EMPTY_OPT_INFO;
opt_info	opt_av			= EMPTY_OPT_INFO;
opt_info	opt_a			= EMPTY_OPT_INFO;
opt_info	opt_d			= EMPTY_OPT_INFO;
opt_info	opt_p			= EMPTY_OPT_INFO;
opt_info	opt_P			= EMPTY_OPT_INFO;
opt_info	opt_r			= EMPTY_OPT_INFO;
opt_info	opt_t			= EMPTY_OPT_INFO;
opt_info	opt_v			= EMPTY_OPT_INFO;
opt_info	opt_V			= EMPTY_OPT_INFO;
opt_info	opt_y			= EMPTY_OPT_INFO;
opt_info	opt_x			= EMPTY_OPT_INFO;
opt_info	opt_files		= EMPTY_OPT_INFO;


parsed_opts		a_subopts[]	= { {
								OPT_NOT_MODAL,
								"3", "three",

								"Limit to 3 files",
								NULL_ARG_INFO,

								NULL,
								&opt_a3,
							  },
							  {
								OPT_NOT_MODAL,
								"v", NULL,

								"Verbosity for -a.",
								{ "<level>", 1, 1, 1 },

								NULL,
								&opt_av
						      },NULL_OPTS  };

parsed_opts		opts[]		= { { DEFAULT_ARGS, &opt_files },
							  {
								OPT_MODAL,
								"ai", "add:insert",

								"Add <file> to the archive.  If <file> is not provided, then list all entries in the archive.",
								{ "<file>", 1, 0, ARGS_INFINITE },

								a_subopts,
								&opt_a
						      },
							  {
								OPT_MODAL,
								"d", "del:remove",

								"Delete <file> from the archive.",
								{ "<file>", 1, 0, ARGS_INFINITE },

								NULL,
								&opt_d
							  },
							  {
								OPT_MODAL,
								"p", "patterns",

								"Search for <pat> in <filepat>.",
								{ "<pat> <filepat>", 2, 1, ARGS_INFINITE },

								NULL,
								&opt_p
							  },
							  {
								OPT_MODAL,
								"P", NULL,

								"Search for <pattern_number_1> in <pattern_number_2>.",
								{ "<pattern_number_1> <pattern_number_2>", 2, 1, ARGS_INFINITE },

								NULL,
								&opt_P
							  },
							  {
								OPT_MODAL,
								"r", NULL,

								"Replace <item1> with <item2>. This option may also take an option modifier:"
								"\n.2:8.-r:2  Replace only 2 but ignore all the other parameters no matter how many there might be because we don't need to deal with them..."
								"\n.2:8.-r:*  Replace all",
								{ "<item1> <item2>", 2, 1, ARGS_INFINITE },

								NULL,
								&opt_r
							  },
							  {
								OPT_NOT_MODAL,
								"t", "test",

								"Test mode (set to <level>)",
								{ "<level>", 1, 1, 1 },

								NULL,
								&opt_t
						      },
							  {
								OPT_NOT_MODAL,
								"v", NULL,

								"Be <level> verbose.",
								{ "<level>", 1, 0, 1 },

								NULL,
								&opt_v
						      },
							  {
								OPT_NOT_MODAL,
								"V", "label",

								"Create archive with volume name <name>.  At list/extract time, apply the globbing pattern <pat>",
								{ "<name> <pat>", 2, 1, 1 },

								NULL,
								&opt_V
						      },
							  {
								OPT_NOT_MODAL,
								"x", "Extract",

								"Extract the 2-5 files provided.",
								{ "<file>", 1, 2, 5 },

								NULL,
								&opt_x
						      },
							  {
								OPT_MODAL,
								"y", "Yank",

								"Yank 2 or more files.",
								{ "<file>", 1, 2, ARGS_INFINITE },

								NULL,
								&opt_y
						      },
							  NULL_OPTS  };


static void
output_optargs	(parsed_opts*	opt,
				 int			level)
{
	int		idex;

	if (! opt || ! opt->status)
		return;

	fprintf (stdout, "%*s%d:%s[%s]: [%s]: ",
					(level*2), " ",
					opt->status->optidex,
					(opt->status->opt ? opt->status->opt[0] : ""),
					(opt->status->optmod ? opt->status->optmod : ""),
					(opt->longopts ? opt->longopts : opt->shortopts));

	if (opt->status->optarg_cnt > 0)
		fprintf (stdout, "[");

	for (idex = 0; idex < opt->status->optarg_cnt; idex++)
	{
		if (idex)
			fprintf (stdout, ", ");
		fprintf (stdout, "%s", opt->status->optarg[idex]);
	}
	if (opt->status->optarg_cnt > 0)
		fprintf (stdout, "]");

	fprintf (stdout, "\n");

	for (idex = 0; opt->subopts && ! NULL_OPT(opt->subopts[idex]); idex++)
		output_optargs(&(opt->subopts[idex]), level+1);
}

int	main	(int	argc,
			 char*	argv[])
{
	int		idex;
	char*	stat;

	if ((stat = opts_parse(opts, argc, argv)) != NULL)
	{
		if (*stat)
			opts_fprintf(stderr, opts,
			             "\nError: *** %s\n\nUsage: %s [options] <file>...\n",
			             stat, argv[0]);
		else
			opts_fprintf(stderr, opts,
			             "\nUsage: %s [options] <file>...\n", argv[0]);

		return(1);
	}

	for (idex = 0; ! NULL_OPT(opts[idex]); idex++)
	{
		output_optargs(&(opts[idex]), 0);
	}

	return(0);
}
#endif	// TEST
