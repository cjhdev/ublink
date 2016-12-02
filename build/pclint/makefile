DIR_ROOT := ../..

PCLINT_LIB := /usr/local/lib/pclint
PCLINT_BIN := /usr/local/bin/pclint

CC := gcc
GXX := g++

SRC := $(filter-out $(DIR_ROOT)/src/blink_event_decoder.c, $(wildcard $(DIR_ROOT)/src/*.c))

INCLUDES += -Ipclint
INCLUDES += -I$(PCLINT_LIB)/lnt
INCLUDES += -I$(DIR_ROOT)/include

RULES += '-zero'

RULES += au-misra.lnt
RULES += co-gcc.lnt
RULES += '-dNDEBUG'
RULES += '-passes(3)'
RULES += '-width(0,4)'
RULES += '-esym(970,_Bool)'


.PHONY: settings clean_settings report.txt

all: report.txt

# this will ensure the settings folder is initialised (or re-initialised)
install: clean_settings settings

# run co-gcc.mak to gather toolchain settings in the pclint folder
settings:
	cd pclint && make -f $(PCLINT_LIB)/lnt/co-gcc.mak GCC_BIN=$(CC) GXX_BIN=$(GXX) CFLAGS=$(CFLAGS) CXXFLAGS=$(CXXFLAGS) CPPFLAGS=$(CPPFLAGS) COMMON_FLAGS=$(COMMON_FLAGS)

# clear contents of pclint folder
clean_settings:
	rm -f pclint/*.lnt pclint/*.h

report.txt:
	$(PCLINT_BIN) '-os($@)' ++b $(INCLUDES) $(RULES) $(SRC)
	@ echo "mandatory:" 
	@ cat $@ | grep -s mandatory || true
	@ echo ""
	@ echo "required:" 
	@ cat $@ | grep required || true
	@ echo ""
	@ echo "advisory:" 
	@ cat $@ | grep advisory || true


