CXX := g++
STD := -std=c++1z
CPPFLAGS := $(STD) -Iinclude
CXXFLAGS := $(STD) -Wall -O3 -flto -Iinclude -fmax-errors=3
# CXXFLAGS := $(STD) -Wall -g -Iinclude -fmax-errors=3
LDFLAGS := $(STD) -O3 -flto
LDLIBS :=

ifeq (,${PREFIX})
PREFIX := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif
CXXFLAGS += -DPREFIX="$(PREFIX)"

SRC := src
BIN := bin
BLD := .build
EXT := .cc

.PHONY: all clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean)))

SRCS := $(shell find $(SRC) -type f -name '*$(EXT)')
DEPS := $(patsubst $(SRC)/%$(EXT),$(BLD)/%.d,$(SRCS))

GREP_EXES := grep -rl '^[[:blank:]]*int \+main *(' $(SRC) --include='*$(EXT)'
EXES := $(patsubst $(SRC)%$(EXT),$(BIN)%,$(shell $(GREP_EXES)))

HISTS := $(filter $(BIN)/hist_%,$(EXES))

all: $(EXES)

-include $(DEPS)

.SECONDEXPANSION:

$(DEPS): $(BLD)/%.d: $(SRC)/%$(EXT) | $(BLD)/$$(dir %)
	$(CXX) $(CPPFLAGS) $(C_$*) -MM -MT '$(@:.d=.o)' $< -MF $@

$(BLD)/%.o: | $(BLD)
	$(CXX) $(CXXFLAGS) $(C_$*) -c $(filter %$(EXT),$^) -o $@

$(BIN)/%: $(BLD)/%.o | $(BIN)
	$(CXX) $(LDFLAGS) $(filter %.o,$^) -o $@ $(LDLIBS) $(L_$*)

$(BIN):
	mkdir -p $@

$(BLD)/%/:
	mkdir -p $@

endif

clean:
	@rm -rfv $(BLD) $(BIN)
