.ONESHELL:
.SECONDARY:
.PHONY: all clean package

BIN_DIR := bin
SRC_DIR := src
INC_DIR := include
STAGE_PATH := /tmp

CC       := gcc
CPPFLAGS := 
opt: CPPFLAGS := $(CPPFLAGS) -D_GNU_SOURCE -D__LEVEL_LOG__=2 -D__LEVEL_ASSERT__=0
dbg: CPPFLAGS := $(CPPFLAGS) -D_GNU_SOURCE -D__LEVEL_LOG__=4 -D__LEVEL_ASSERT__=1
CFLAGS   := -std=c11 -I $(INC_DIR) -Wall -Werror
opt: CFLAGS := $(CFLAGS) -mtune=native -march=native -O3
dbg: CFLAGS := $(CFLAGS) -ggdb 
LDFLAGS  :=
dbg: LDFLAGS := $(LDFLAGS)
LDLIBS   := -lpthread

flavors = opt dbg
headers = $(wildcard $(INC_DIR)/*.h)
c_files = $(wildcard $(SRC_DIR)/*.c)
objects = $(addsuffix .o, $(basename $(notdir $(c_files))))

all: $(flavors);
%: init_% $(BIN_DIR)/%/project ;
init_% :
	tmp_dir=`readlink -f .`
	tmp_dir=$(STAGE_PATH)/`basename "$$tmp_dir"`
	[[ -d "$$tmp_dir" ]]     || mkdir "$$tmp_dir"
	[[ -e "$(BIN_DIR)" ]]    || ln -s "$$tmp_dir" "$(BIN_DIR)"
	[[ -d "$(BIN_DIR)/$*" ]] || mkdir -p "$(BIN_DIR)/$*"

clean:
	rm -rf $(BIN_DIR)/*

%/project : $(addprefix %/, $(objects))
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

$(BIN_DIR)/opt/%.o : $(SRC_DIR)/%.c $(headers)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BIN_DIR)/dbg/%.o : $(SRC_DIR)/%.c $(headers)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

