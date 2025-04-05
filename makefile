CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDFLAGS = -lmenu -lncurses

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin
BIN = $(BIN_DIR)/madnc

CORE_SRCS = $(wildcard $(SRC_DIR)/core/*.c)
TUI_SRCS = $(wildcard $(SRC_DIR)/tui/*.c)
MAIN_SRC = $(SRC_DIR)/main.c

SRCS = $(CORE_SRCS) $(TUI_SRCS) $(MAIN_SRC)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(BIN): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
