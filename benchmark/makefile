DIR_ROOT := ..
DIR_BUILD := build
DIR_BIN := bin

CC := gcc

VPATH += $(DIR_ROOT)/src

INCLUDES += -I$(DIR_ROOT)/include

CFLAGS := -Wall -Werror -g $(INCLUDES) -O3

SRC := $(notdir $(wildcard $(DIR_ROOT)/src/*.c)) benchmark.c

OBJ := $(SRC:.c=.o)

.PHONY: clean

$(DIR_BIN)/benchmark: $(addprefix $(DIR_BUILD)/, $(OBJ))
	$(CC) $(LDFLAGS) $^ -o $@

$(DIR_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(DIR_BUILD)/*


