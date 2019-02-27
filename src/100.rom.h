#ifndef	__ROMS_100_rom_H__
#define	__ROMS_100_rom_H__
PRCIDType	small_PRC_ROMS_100_rom[]	= { 
                           {          sysFileTBoot,        sysFileCSystem, 0 },
                           { 0, 0, 0 }
};
PRCIDType	large_PRC_ROMS_100_rom[]	= { 
                           {        sysFileTSystem,        sysFileCSystem, 0 },
                           {   sysFileTApplication,       sysFileCAddress, 0 },
                           {        sysFileTKernel,        sysFileCSystem, 0 },
                           {   sysFileTApplication,      sysFileCDatebook, 0 },
                           {   sysFileTApplication,    sysFileCCalculator, 0 },
                           {    sysFileTUIAppShell,        sysFileCSystem, 0 },
                           {   sysFileTApplication,          sysFileCToDo, 0 },
                           {   sysFileTApplication,          sysFileCSync, 0 },
                           {   sysFileTApplication,      sysFileCSecurity, 0 },
                           {   sysFileTApplication,   sysFileCPreferences, 0 },
                           {   sysFileTApplication,        sysFileCMemory, 0 },
                           {   sysFileTApplication,          sysFileCMemo, 0 },
                           { 0, 0, 0 }
};
#endif	/* __ROMS_100_rom_H__ */
