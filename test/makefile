DIR_ROOT := ..
DIR_CMOCKA := $(DIR_ROOT)/vendor/cmocka
DIR_BUILD := build
DIR_BIN := bin

CC := gcc

VPATH += $(DIR_ROOT)/src
VPATH += $(DIR_CMOCKA)/src

INCLUDES += -I$(DIR_ROOT)/include
INCLUDES += -I$(DIR_CMOCKA)/include

CMOCKA_DEFINES += -DHAVE_STRINGS_H
CMOCKA_DEFINES += -DHAVE_SIGNAL_H
CMOCKA_DEFINES += -DHAVE_INTTYPES_H
CMOCKA_DEFINES += -DHAVE_MALLOC_H

CFLAGS := -Wall -Werror -g -fprofile-arcs -ftest-coverage $(INCLUDES) $(CMOCKA_DEFINES)
LDFLAGS := -fprofile-arcs -g

SRC := $(notdir $(wildcard $(DIR_ROOT)/src/*.c))
SRC_CMOCKA := $(notdir $(wildcard $(DIR_CMOCKA)/src/*.c))

OBJ := $(SRC:.c=.o)
OBJ_CMOCKA := $(SRC_CMOCKA:.c=.o)

TESTS := $(basename $(wildcard tc_*.c))

LINE := ================================================================

.PHONY: clean build_and_run coverage

build_and_run: 
	@ for tc in $(addprefix $(DIR_BIN)/, $(TESTS)); do \
		echo $(LINE); \
		echo ""; \
		make clean $$tc \
		&& echo "" \
		&& ./$$tc; \
		echo ""; \
	done

$(DIR_BIN)/tc_%: $(addprefix $(DIR_BUILD)/, tc_%.o $(OBJ) $(OBJ_CMOCKA))
	@ echo linking $@
	@ $(CC) $(LDFLAGS) $^ -o $@

$(DIR_BIN)/tc_blink_compact_decode: $(addprefix $(DIR_BUILD)/, tc_blink_compact_decode.o $(OBJ) $(OBJ_CMOCKA))

$(DIR_BUILD)/%.o: %.c
	@ echo building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

coverage:
	@ echo $(LINE)
	@ echo
	@ echo Test Coverage:
	@ echo
	@ make clean > /dev/null
	@ for tc in $(addprefix $(DIR_BIN)/, $(TESTS)); do \
		make $$tc > /dev/null 2>&1 \
		&& ./$$tc > /dev/null 2>&1; \
	done
	@ gcov --no-output --object-directory $(DIR_BUILD) $(OBJ)

clean:
	rm -f $(DIR_BUILD)/*


