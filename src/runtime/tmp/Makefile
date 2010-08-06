override OUTDIR_BY_TYPE=
override OUTPUT_ROOT=

TEST_DIRS = $(wildcard Test*)

.PHONY : all
all : $(TEST_DIRS) run run.sh

.PHONY : $(TEST_DIRS)
$(TEST_DIRS):
	@$(MAKE) -C $@

ifndef CXX
CXX = g++
CC = gcc
endif

run : run.cpp
	@$(CXX) $@.cpp -ldl -g -o $@

run.sh : $(HPHP_HOME)/src/rules.mk
	@$(MAKE) -f script.mk
	@chmod +x $@

TMP = $(TEST_DIRS) \
      run run.sh

clobber : clean

clean :
	@rm -fR $(TMP)
