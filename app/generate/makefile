DIR_ROOT := ../..
DIR_BUILD := build
DIR_BIN := bin

CC := gcc

VPATH += $(DIR_ROOT)/src

INCLUDES += -I$(DIR_ROOT)/include

CFLAGS := -Wall -Werror $(INCLUDES) -g
LDFLAGS := -g

SRC += blink_schema.c
SRC += blink_lexer.c
SRC += blink_pool.c
SRC += generate.c
SRC += generate_c.c

OBJ := $(addprefix $(DIR_BUILD)/, $(SRC:.c=.o))

all: $(DIR_BIN)/generate

.PHONY: clean 

$(DIR_BIN)/generate: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(DIR_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(DIR_BUILD)/*
