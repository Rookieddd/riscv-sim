# riscv-sim 构建脚本
# 目标：
#   make                          正式版 build/sim（-O2，警告即错误）
#   make debug                    调试版 build/sim-dbg（-O0 + ASan/UBSan，查段错误用）
#   make run HEX=tests/demo.hex   构建并运行
#   make clean
# 注意：规则行（命令）前必须是 Tab 缩进，用空格会报 "missing separator"。

CC      := gcc
CFLAGS  := -std=c17 -Wall -Wextra -Werror -O2
DFLAGS  := -std=c17 -Wall -Wextra -Werror -O0 -g -fsanitize=address,undefined
BUILD   := build
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:src/%.c=$(BUILD)/%.o)

.PHONY: all sim debug run clean

all: sim

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/sim: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

sim: $(BUILD)/sim

debug: $(SRC) | $(BUILD)
	$(CC) $(DFLAGS) $(SRC) -o $(BUILD)/sim-dbg

run: sim
	./$(BUILD)/sim $(HEX)

clean:
	rm -rf $(BUILD)
