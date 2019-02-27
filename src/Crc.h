/******************************************************************************
 ******************************************************************************
 *** From <emulator>/SrcShared/Palm/Platform/Incs/Core/System/Crc.h
 ******************************************************************************
 ******************************************************************************/
/******************************************************************************
 *
 * Copyright (c) 1995-1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Crc.h
 *
 * Description:
 *		This is the header file for the CRC calculation routines for Pilot.
 *
 * History:
 *		May 10, 1995	Created by Vitaly Kruglikov
 *		05/10/95	vmk	Created by Vitaly Kruglikov.
 *		09/10/99	kwk	Crc16CalcBlock takes a const void *.
 *
 *****************************************************************************/

#ifndef __CRC_H__
#define __CRC_H__

#include "os_structs.h"

/********************************************************************
 * CRC Calculation Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------


// Crc16CalcBlock()
//
// Calculate the 16-bit CRC of a data block using the table lookup method.
// 
UInt16		Crc16CalcBigBlock(void *bufP, UInt32 count, UInt16 crc);

#endif  // __CRC_H__
