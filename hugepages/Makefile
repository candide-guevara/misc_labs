.ONESHELL:
.SECONDARY:
.PHONY: all clean package

BIN_DIR := bin
SRC_DIR := .
STAGE_PATH := /tmp

CC       := gcc
CPPFLAGS := $(CPPFLAGS) -D_DEFAULT_SOURCE
CFLAGS   := -std=c11 -Wall -Werror
LDFLAGS  :=

c_files := $(wildcard $(SRC_DIR)/*.c)
x_files := $(addprefix $(BIN_DIR)/, $(basename $(c_files)))

all: $(x_files);
$(x_files): $(BIN_DIR)
$(BIN_DIR) :
	tmp_dir=`readlink -f .`
	tmp_dir=$(STAGE_PATH)/`basename "$$tmp_dir"`
	[[ -d "$$tmp_dir" ]]     || mkdir "$$tmp_dir"
	[[ -e "$(BIN_DIR)" ]]    || ln -s "$$tmp_dir" "$(BIN_DIR)"

clean:
	rm -rf $(BIN_DIR)/*

$(BIN_DIR)/$(SRC_DIR)/% : %.c
	$(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -o "$@" "$<"

