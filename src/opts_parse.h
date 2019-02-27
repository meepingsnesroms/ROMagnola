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

#ifndef	__OPTS_PARSE_H__
#define	__OPTS_PARSE_H__

#include <stdarg.h>

/*
 * opts_cols defines the column sizes for output
 * 	column 1	== initial indentation   (default  2)
 * 	column 2	== option/argument area  (default 28)
 * 	column 3	== separator             (default  2)
 * 	column 4	== description           (default 47)
 */
extern int		opts_cols[];

/* parsed_opts flags */
#define	OPT_NOT_MODAL			0x0000
#define	OPT_MODAL				0x0001
#define	OPT_DEFAULT_ARGS		0x0002	// Collect all unconsumed arguments

#define	OPT_MOD_MASK			0x000F

/* arg_info -- max count */
#define	ARGS_INFINITE			0xFFFFFFFF

/* Access the modality portion of the parse_opts flags */
#define	OPT_MOD(f)				((f) & OPT_MOD_MASK)

/* Is the option null? */
#define	NULL_OPT(o)				(! (o).status )


/* Several checks about the number of arguments for a given option */
#define	HAS_ARGS(p)				((p)->args.maxcnt != 0)
#define	HAS_MULTIPLE_ARGS(p)	((p)->args.maxcnt > 1)
#define	HAS_OPTIONAL_ARGS(p)	(((p)->args.mincnt == 0) &&			\
								 ((p)->args.maxcnt > 1))
#define	HAS_MANDATORY_ARGS(p)	((p)->args.mincnt > 0)


/*
 * Structure initialization macros
 */
#define	NULL_ARG_INFO			{ NULL, 0, 0, 0 }
#define	NULL_OPTS				{ 0, NULL, NULL, NULL, NULL_ARG_INFO, NULL,NULL}

#define	EMPTY_OPT_INFO			{ 0, NULL, NULL, NULL, 0 }

#define	DEFAULT_ARGS			OPT_DEFAULT_ARGS,					\
								NULL, NULL, NULL, NULL_ARG_INFO, NULL


typedef	struct arg_info		arg_info;
typedef	struct opt_info		opt_info;
typedef	struct parsed_opts	parsed_opts;

/*
 * Argument information
 */
struct	arg_info
{
	// Output information
	char*			argname;		// Name of argument (for output)

	unsigned long	argblk;			// # arguments per "block"
	unsigned long	mincnt;			// Minimum # argument blocks
	unsigned long	maxcnt;			// Maximum # argument blocks (-1 = no limit)
};

/*
 * Option parse information
 */
struct	opt_info
{
	int				optidex;		// Index of 'opt' in argv[]
	char**			opt;			// pointer to this option in argv[]

	char*			optmod;			// option modified (everything after ':')

	char**			optarg;			// pointer to first argv[] option argument
	unsigned long	optarg_cnt;		// count of arguments for this option
};

/*
 * Description of options to parse
 */
struct parsed_opts
{
	// Parse information
	unsigned short	flags;			// flags - modality (is modal or not),
									//       - default argument catcher

	char*			shortopts;		// Short options: e.g. "ai"
	char*			longopts;		// Long options : e.g. "add:insert"

	char*			description;	// Option description (for output)

	arg_info		args;			// Argument parsing information

	// Sub options
	parsed_opts*	subopts;		// sub-options

	// Parse status
	opt_info*		status;
};


/******************************************************************************
 * Prototypes
 */
extern
char*	opt_str			(parsed_opts*	pOpt);

extern
void	opts_vprintf	(int			hFile,
						 parsed_opts	opts[],
						 char*			fmt,
						 va_list		args);

extern
char*	opts_parse		(parsed_opts	opts[],
						 int			argc,
						 char**			argv);
extern
void	opts_printf		(int			hFile,
						 parsed_opts	opts[],
						 char*			fmt,
						 ...);

extern
void	opts_fprintf	(FILE*			pFile,
						 parsed_opts	opts[],
						 char*			fmt,
						 ...);


#endif	// __OPTS_PARSE_H__
