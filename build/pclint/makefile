DIR_ROOT := ../..

PCLINT_LIB := /usr/local/lib/pclint
PCLINT_BIN := /usr/local/bin/pclint

# change compiler bins as required
CC := gcc
GXX := g++

# you can also define CFLAGS, CXXFLAGS CPPFLAGS COMMON_FLAGS as required

# rules to apply (order is significant)
RULES := au-misra.lnt co-gcc.lnt settings.lnt   

# sources to lint
SRC := $(addprefix $(DIR_ROOT)/src/, blink_schema.c)

# included from sources to lint
INCLUDES := -I$(DIR_ROOT)/include

# this will ensure the settings folder is initialised (or re-initialised)
install: clean_settings settings

# run co-gcc.mak to gather toolchain settings in the pclint folder
settings:
	cd pclint && make -f $(PCLINT_LIB)/lnt/co-gcc.mak GCC_BIN=$(CC) GXX_BIN=$(GXX) CFLAGS=$(CFLAGS) CXXFLAGS=$(CXXFLAGS) CPPFLAGS=$(CPPFLAGS) COMMON_FLAGS=$(COMMON_FLAGS)

# clear contents of pclint folder
clean_settings:
	rm -f pclint/*.lnt pclint/*.h

# lint a number of source files
lint:
	$(PCLINT_BIN) "+rw(__is_pod)" -I$(PCLINT_LIB)/lnt -Ipclint $(INCLUDES) $(RULES) $(SRC)




