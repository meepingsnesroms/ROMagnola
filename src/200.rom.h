#ifndef	__ROMS_200_rom_H__
#define	__ROMS_200_rom_H__
PRCIDType	small_PRC_ROMS_200_rom[]	= { 
                           {          sysFileTBoot,        sysFileCSystem, 0 },
                           { 0, 0, 0 }
};
PRCIDType	large_PRC_ROMS_200_rom[]	= { 
                           {        sysFileTSystem,        sysFileCSystem, 0 },
                           {        sysFileTKernel,        sysFileCSystem, 0 },
                           {    sysFileTUIAppShell,        sysFileCSystem, 0 },
                           {       sysFileTHtalLib,       sysFileCPADHtal, 0 },
                           {       sysFileTLibrary,           sysFileCNet, 0 },
                           {          sysFileTneti,           sysFileCppp, 0 },
                           {          sysFileTneti,          sysFileCslip, 0 },
                           {          sysFileTneti,          sysFileCloop, 0 },
                           {         sysFileTPanel,  sysFileCNetworkPanel, 0 },
                           {   sysFileTApplication,       sysFileCAddress, 0 },
                           {   sysFileTApplication,    sysFileCCalculator, 0 },
                           {   sysFileTApplication,      sysFileCDatebook, 0 },
                           {   sysFileTApplication,          sysFileCMemo, 0 },
                           {   sysFileTApplication,        sysFileCMemory, 0 },
                           {   sysFileTApplication,   sysFileCPreferences, 0 },
                           {   sysFileTApplication,      sysFileCSecurity, 0 },
                           {   sysFileTApplication,          sysFileCSync, 0 },
                           {   sysFileTApplication,          sysFileCToDo, 0 },
                           {         sysFileTPanel,     sysFileCDigitizer, 0 },
                           {         sysFileTPanel,       sysFileCGeneral, 0 },
                           {         sysFileTPanel,       sysFileCFormats, 0 },
                           {         sysFileTPanel,     sysFileCShortCuts, 0 },
                           {         sysFileTPanel,         sysFileCOwner, 0 },
                           {         sysFileTPanel,       sysFileCButtons, 0 },
                           {         sysFileTPanel,    sysFileCModemPanel, 0 },
                           {   sysFileTApplication,          sysFileCMail, 0 },
                           {   sysFileTApplication,       sysFileCExpense, 0 },
						   {   sysFileTApplication,       sysFileCGira,    0 },
                           { 0, 0, 0 }
};
#endif	/* __ROMS_200_rom_H__ */
