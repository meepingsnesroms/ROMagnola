/******************************************************************************
 ******************************************************************************
 *** From <emulator>/SrcShared/Palm/Platform/Core/System/IncsPrv/SystemPrv.h
 ******************************************************************************
 ******************************************************************************//******************************************************************************
 *
 * Copyright (c) 1994-1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: SystemPrv.h
 *
 * Description:
 *		Private Pilot system equates
 *
 * History:
 *		08/09/95	RM		Created by Ron Marianetti   
 *		01/28/98	SCL	Added HotSync port "hint" for SerialHWMgr
 *		02/04/98 srj	Added HW_TARGET_EZ in appropriate places.
 *							HW_TARGET_TD1 in this file means "any
 *							handheld", not non-EZ platforms.
 *		04/05/99	jrb	bumped sysLowMemSize
 *		04/13/99	kwk	Bumped sysLowMemSize to 15B0.
 *		05/21/99	kwk	Bumped sysLowMemSize to 1700.
 *		06/24/99	kwk	Added four locale-related fields to SysNVParamsType.
 *		07/01/99	kwk	Added SysExtPrefsType structure & version/flag defs.
 *		07/13/99	kwk	Bumped sysLowMemSize to 1800.
 *		11/01/99	kwk	Moved SysWantEvent here from SystemMgr.h
 *		12/03/99	SCL	Moved SysAppInfoType, SysAppStartup, and SysAppExit
 *							to SystemMgr.h (for StartupCode/Runtime)
 *
 *****************************************************************************/

//#ifdef	NON_PORTABLE

#ifndef __SYSTEMPRV_H__
#define __SYSTEMPRV_H__

/************************************************************
 * Common Equates between Native and Emulation mode
 *************************************************************/
#define	sysCardSignature				0xFEEDBEEFL			// card signature long word
#define  sysStoreSignature				0xFEEDFACEL			// store signature long word

/************************************************************
 * This structure defines a section within the RAM storage header
 *  on Card#0 used to hold non-volatile System information. We store
 *  System information that can not be conveniently stored or accessed
 *  from a Database in this area because:
 *	1.) it can be accessed earlier during the boot-up process and 
 * 2.) It can be accessed from an interrupt routine.
 *************************************************************/ 
typedef struct SysNVParamsType {
	UInt32	rtcHours;									// Real-Time clock hours - add to value
																// in DragonBall RTC register to get
																// actual date & time.
	UInt32	rtcHourMinSecCopy;						// Copy of latest value in rtcHourMinSec reg of 
																// DBall. Used as default RTC value on Reset.
	UInt8		swrLCDContrastValue;						// Contrast Value for LCD on EZ-based products
																// that use the software contrast PWM (such as Sumo)
	UInt8 	swrLCDBrightnessValue;					// Brightness value for screens with adjustable brightness.
	
	// Note that in the ROM store, these next four fields contain the default
	// settings for card 0's RAM store, when it has to be initialized.
	void*		splashScreenPtr;							// MemPtr to splash screen bitmap
	void*		hardResetScreenPtr;						// MemPtr to hard reset screen bitmap.
	UInt16	localeLanguage;							// Language for locale.
	UInt16	localeCountry;								// Country for locale.
	
	// 11/15/99 SCL: New Globals added for Licensees, Partners, OEMs, etc.
	// These storage locations are reserved for the HAL running on a given device.
	UInt32	sysNVOEMStorage1;							// 4 bytes for Device OEM use only!
	UInt32	sysNVOEMStorage2;							// 4 bytes for Device OEM use only!

	} SysNVParamsType;
typedef SysNVParamsType *SysNVParamsPtr;

#endif  //__SYSTEMPRV_H__

//#endif // NON_PORTABLE
