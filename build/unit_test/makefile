DIR_ROOT := ../..

CC := gcc

DIR_UNITY := $(DIR_ROOT)/vendor/unity

DIR_BUILD := build
DIR_BIN := bin
DIR_RUNNERS := runners

VPATH := $(DIR_ROOT)/src:$(DIR_ROOT)/test/unit_test:$(DIR_UNITY)/src:$(DIR_RUNNERS)

INCLUDES := -I$(DIR_ROOT)/include -I$(DIR_UNITY)/src

CFLAGS := -Wall -std=c99 -g -fprofile-arcs -ftest-coverage $(INCLUDES)
LDFLAGS := -fprofile-arcs

TEST_FILES := $(shell ls $(DIR_ROOT)/test/unit_test)

TESTS := $(TEST_FILES:.c=)
TEST_RUNNERS := $(addprefix run_, $(TESTS))

all: run

runners: $(addprefix $(DIR_RUNNERS)/, $(TEST_RUNNERS:=.c))

$(DIR_RUNNERS)/run_%.c: %.c
	ruby $(DIR_UNITY)/auto/generate_test_runner.rb $(^) $(@)

tests: $(TEST_FILES:.c=)

#
# below we pattern match similar groups of test binaries
# 

tc_BLINK%: $(addprefix $(DIR_BUILD)/, run_tc_BLINK%.o tc_BLINK%.o blink_schema.o unity.o  $(TARGET_OBJ))
	$(CC) $(LDFLAGS) $^ -o $(DIR_BIN)/$@

$(DIR_BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean clean_obj clean_runners coverage

clean: clean_obj clean_runners

clean_obj:
	rm -f $(DIR_BUILD)/*

clean_runners:
	rm -f $(DIR_RUNNERS)/*

run: $(addprefix run_, $(TESTS))

run_%: %
	@-./$(DIR_BIN)/$^

coverage: $(addprefix $(DIR_BUILD)/, blink_schema.gcda)
	@echo "coverage report:"
	gcov --no-output --object-directory $(DIR_BUILD) $(^)



