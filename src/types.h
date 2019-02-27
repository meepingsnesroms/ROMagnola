#ifndef	__TYPE_H__
#define	__TYPE_H__

/********************************************************************
 * Elementary data types
 ********************************************************************/
#include <endian.h>

#define CPU_ENDIAN_LITTLE	__LITTLE_ENDIAN
#define CPU_ENDIAN_BIG		__BIG_ENDIAN
#define CPU_ENDIAN_PDP		__PDP_ENDIAN
#define CPU_ENDIAN			__BYTE_ORDER

#ifndef	NULL
#  define	NULL	0
#endif	// NULL


// Fixed size data types
typedef signed char		Int8;
typedef signed short	Int16;	
typedef signed long		Int32;

typedef unsigned char	UInt8;
typedef unsigned short  UInt16;
typedef unsigned long   UInt32;


// Logical data types
typedef unsigned char	Boolean;

typedef char			Char;
typedef UInt16			WChar;		// 'wide' int'l character type.

typedef UInt16			Err;

typedef UInt32			LocalID;	// local (card relative) chunk ID

typedef Int16 			Coord;		// screen/window coordinate


typedef void *			MemPtr;		// global pointer
typedef struct _opaque*	MemHandle;	// global handle


#endif	// __TYPE_H__
