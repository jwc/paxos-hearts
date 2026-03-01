CXX = g++
CXXFLAGS = -g -pthread -std=c++20
#MAKEFLAGS += -j4

.PRECIOUS: %.o %.gch %.out
.PHONY: all clean test

SRC_DIR =./src
INC_DIR =./include
OBJ_DIR =./src
BLD_DIR =.
TST_DIR =./test

OVERRIDES = -D PAXOS_LOG_SIZE=100 

TARGETS := main.out 
SRCS := $(shell find $(SRC_DIR) -name '*.cc')
OBJS := $(SRCS:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)
OBJS := $(filter-out $(TARGETS:%.out=$(OBJ_DIR)/%.o),$(OBJS))
INCS := $(shell find $(INC_DIR) -name '*.hh')
TSTS := $(shell find $(TST_DIR) -name '*.cc')
TSTS := $(TSTS:%.cc=%.out)

all: $(TARGETS) 

test: $(INC_DIR)/header.hh.gch $(TSTS)
	./test.sh

clean:
	rm -rf */*.o */*.gch */*.out */*.log *.out

# Automatically make necessary directories.
./%:
	mkdir $@
$(OBJS): | $(OBJ_DIR)

# Making object files.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(INC_DIR)/%.hh 
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(INC_DIR)

# Making Executables.
$(BLD_DIR)/%.out: $(SRC_DIR)/%.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -I$(INC_DIR)

# Compiling testcases. 
$(TST_DIR)/%.out: $(TST_DIR)/%.cc $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -I$(INC_DIR)

# Rule for creating precompiled header files.
$(INC_DIR)/%.hh.gch: $(INC_DIR)/%.hh $(INC_DIR)/*.hh
	$(CXX) $(CXXFLAGS) -c $< 

