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

SRC_BLINK := $(filter-out blink_object.c blink_event_decoder.c, $(notdir $(wildcard $(DIR_ROOT)/src/*.c)))
SRC_CMOCKA := $(notdir $(wildcard $(DIR_CMOCKA)/src/*.c))

OBJ_BLINK := $(SRC_BLINK:.c=.o)
OBJ_CMOCKA := $(SRC_CMOCKA:.c=.o)

.PHONY: clean 

$(DIR_BIN)/tc_%: $(addprefix $(DIR_BUILD)/, tc_%.o $(OBJ_BLINK) $(OBJ_CMOCKA))
	$(CC) $(LDFLAGS) $^ -o $@

$(DIR_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(DIR_BUILD)/*
