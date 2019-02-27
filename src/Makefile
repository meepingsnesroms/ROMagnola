CC		= gcc
CFLAGS	= -Wall -g -IfakeSysIncludes -m32 
LDFLAGS	= -L. -lromeo 
AR		= ar crv

LIB		= libromeo.a
LIBOBJS	= utils.o byte_swap.o relocate.o translate.o Crc.o rom.o \
		  extract.o assemble.o

all: romeo

romeo: $(LIB) romeo.o output.o output_resource.o opts_parse.o

read_prc_data: $(LIB) read_prc_data.o

$(LIB): $(LIBOBJS)
	$(AR) $(LIB) $(LIBOBJS)

depend: FORCE
	@makedepend -Y *.c > /dev/null 2>/dev/null

tags: FORCE
	ctags    *.[chCH] *.cpp 2>/dev/null
	ctags -e *.[chCH] *.cpp 2>/dev/null

clean: FORCE
	rm -f romeo *.[oa] *~

FORCE:

opts_parse: opts_parse.c opts_parse.h
	$(CC) $(CFLAGS) -c -DTEST opts_parse.c -o opts_parse-t.o
	$(CC) $(CFLAGS) opts_parse-t.o -o opts_parse

################################################################################
## Following are dependencies (created via the 'depend' rule)
# DO NOT DELETE

Crc.o: Crc.h os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h DataPrv.h
Crc.o: OverlayMgr.h
assemble.o: SystemResources.h rom.h os_structs.h types.h DataMgr.h
assemble.o: SystemPrv.h MemoryPrv.h DataPrv.h OverlayMgr.h utils.h assemble.h
assemble.o: translate.h extract.h Crc.h byte_swap.h
byte_swap.o: rom.h os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h
byte_swap.o: DataPrv.h OverlayMgr.h byte_swap.h
extract.o: os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h DataPrv.h
extract.o: OverlayMgr.h rom.h extract.h byte_swap.h translate.h utils.h
opts_parse.o: opts_parse.h
output.o: os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h DataPrv.h
output.o: OverlayMgr.h rom.h output.h utils.h byte_swap.h SystemResources.h
output.o: output_resource.h
output_resource.o: os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h
output_resource.o: DataPrv.h OverlayMgr.h rom.h utils.h byte_swap.h
output_resource.o: SystemResources.h output.h output_resource.h
read_prc_data.o: rom.h os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h
read_prc_data.o: DataPrv.h OverlayMgr.h SystemResources.h extract.h utils.h
read_prc_data.o: byte_swap.h
relocate.o: rom.h os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h
relocate.o: DataPrv.h OverlayMgr.h relocate.h byte_swap.h
rom.o: os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h DataPrv.h
rom.o: OverlayMgr.h rom.h SystemResources.h 350.rom.h 330.rom.h 325.rom.h
rom.o: 310.rom.h 300.rom.h 200.rom.h 100.rom.h
romeo.o: opts_parse.h rom.h os_structs.h types.h DataMgr.h SystemPrv.h
romeo.o: MemoryPrv.h DataPrv.h OverlayMgr.h extract.h assemble.h output.h
romeo.o: utils.h
translate.o: relocate.h rom.h os_structs.h types.h DataMgr.h SystemPrv.h
translate.o: MemoryPrv.h DataPrv.h OverlayMgr.h byte_swap.h translate.h
translate.o: utils.h
utils.o: rom.h os_structs.h types.h DataMgr.h SystemPrv.h MemoryPrv.h
utils.o: DataPrv.h OverlayMgr.h byte_swap.h utils.h SystemResources.h
