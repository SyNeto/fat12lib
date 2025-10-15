CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -Iinclude
TARGET = fat12
SRC = src/fat12.c src/fat12_layout.c
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
TEST_DIR = $(BUILD_DIR)/tests
OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
LIB = $(LIB_DIR)/lib$(TARGET).a

# Test sources
TEST_COMMON_SRC = tests/test_common.c
TEST_CALC_SRC = $(TEST_COMMON_SRC) tests/test_fat12_calculations.c
TEST_BOUNDS_SRC = $(TEST_COMMON_SRC) tests/test_fat12_bounds.c
TEST_VALID_SRC = $(TEST_COMMON_SRC) tests/test_validation.c
TEST_ALIGN_SRC = $(TEST_COMMON_SRC) tests/test_struct_alignment.c
TEST_FIXED_SRC = $(TEST_COMMON_SRC) tests/test_fixed_boot_reading.c
TEST_LAYOUT_SRC = tests/test_layout.c

TEST_CALC_OBJ = $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_CALC_SRC))
TEST_BOUNDS_OBJ = $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_BOUNDS_SRC))
TEST_VALID_OBJ = $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_VALID_SRC))
TEST_ALIGN_OBJ = $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_ALIGN_SRC))
TEST_FIXED_OBJ = $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_FIXED_SRC))
TEST_LAYOUT_OBJ = $(patsubst tests/%.c,$(OBJ_DIR)/%.o,$(TEST_LAYOUT_SRC))

all: $(LIB)

# Test targets
test: test-calculations test-bounds test-validation test-alignment test-fixed test-layout

test-calculations: $(TEST_DIR)/test_fat12_calculations
	./$<

test-bounds: $(TEST_DIR)/test_fat12_bounds
	./$<

test-validation: $(TEST_DIR)/test_validation
	./$<

test-alignment: $(TEST_DIR)/test_struct_alignment
	./$<

test-fixed: $(TEST_DIR)/test_fixed_boot_reading
	./$<

test-layout: $(TEST_DIR)/test_layout
	./$<

$(TEST_DIR)/test_fat12_calculations: $(TEST_CALC_OBJ) $(LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DIR)/test_fat12_bounds: $(TEST_BOUNDS_OBJ) $(LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DIR)/test_validation: $(TEST_VALID_OBJ) $(LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DIR)/test_struct_alignment: $(TEST_ALIGN_OBJ) $(LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DIR)/test_fixed_boot_reading: $(TEST_FIXED_OBJ) $(LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DIR)/test_layout: $(TEST_LAYOUT_OBJ) $(LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(LIB): $(OBJ) | $(LIB_DIR)
	ar rcs $(LIB) $(OBJ)

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test object files
$(OBJ_DIR)/%.o: tests/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean test