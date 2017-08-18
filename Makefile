.ONESHELL:
.SECONDARY:
.PHONY: all clean images

BIN_DIR := bin
SRC_DIR := src
INC_DIR := include

FLAME_ROOT := $(HOME)/Programation/Perl/FlameGraph
CC       := gcc
CPPFLAGS := 
opt: CPPFLAGS := $(CPPFLAGS) -D__LEVEL_LOG__=2 -D__LEVEL_ASSERT__=0
dbg: CPPFLAGS := $(CPPFLAGS) -D__LEVEL_LOG__=4 -D__LEVEL_ASSERT__=1
CFLAGS   := -std=c99 -I $(INC_DIR) -Wall -Werror
opt: CFLAGS := $(CFLAGS) -ggdb -mtune=native -march=native -O3
dbg: CFLAGS := $(CFLAGS) -ggdb -fsanitize=address
LDFLAGS  := -rdynamic
dbg: LDFLAGS := $(LDFLAGS) -fsanitize=address
LDLIBS   := -ldl

flavors = opt dbg
headers = $(wildcard $(INC_DIR)/*.h)
c_files = $(wildcard $(SRC_DIR)/*.c)
objects = $(addsuffix .o, $(basename $(notdir $(c_files))))

all: $(flavors);
%: init_% $(BIN_DIR)/%/project ;
init_% :
	mkdir -p "$(BIN_DIR)/$*"

images : 
	cd $(BIN_DIR)
	echo building images for *.dot
	for f in *.dot; do cat $$f | neato -Tsvg > "$${f}.svg"; done

%_complexity: %
	cd $(BIN_DIR)
	$*/project $*_traversal_timing
	python3 ../etc/plot_complexity.py $*_traversal_timing

%_stat : %
	cd $(BIN_DIR)
	events=(
		"cycles,instructions,branch-misses,branches,cycle_activity.cycles_no_execute,uops_issued.any,uops_retired.all"
		"L1-dcache-load-misses,L1-dcache-loads,LLC-load-misses,LLC-loads,dTLB-load-misses,dTLB-loads,mem_trans_retired.load_latency_gt_256"
		"page-faults,context-switches"
	)
	algos=(
		destructive_std_depth_first_traversal
		destructive_pointer_reversal_traversal
		pointer_reversal_traversal
		destructive_pointer_back_and_forth_traversal
	)
	for algo in "$${algos[@]}"; do
		: > "$*_$${algo}.stat"
		for event in "$${events[@]}"; do
			sudo perf stat -x, --append -o "$*_$${algo}.stat" --append -e "$$event" -- $*/project "$$algo" 
		done
	done
	echo python3 ../etc/plot_stats.py *.stat

%_prof : %
	cd $(BIN_DIR)
	sudo perf record -F 997 --call-graph dwarf -- $*/project
	sudo chown $$USER:$$USER perf.data
	perf script | perl $(FLAME_ROOT)/stackcollapse-perf.pl | perl $(FLAME_ROOT)/flamegraph.pl --hash --minwidth=5 > flame_prof_$*.svg

package :
	tar -hzcf package_$(shell date +%d_%m_%y).tar $(BIN_DIR) $(SRC_DIR) $(INC_DIR)

clean:
	rm -rf $(BIN_DIR)/*

%/project : $(addprefix %/, $(objects))
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

$(BIN_DIR)/opt/%.o : $(SRC_DIR)/%.c $(headers)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BIN_DIR)/dbg/%.o : $(SRC_DIR)/%.c $(headers)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

