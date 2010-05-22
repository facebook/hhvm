override OUTDIR_BY_TYPE=
override OUTPUT_ROOT=

TEST_DIRS = $(wildcard Test*)

.PHONY : all
all : $(TEST_DIRS) run run.sh

.PHONY : $(TEST_DIRS)
$(TEST_DIRS):
	@$(MAKE) -C $@

run : run.cpp
	@g++ $@.cpp -ldl -o $@

run.sh : $(HPHP_HOME)/src/rules.mk
	@$(MAKE) -f script.mk
	@chmod +x $@

TMP = $(TEST_DIRS) \
      run run.sh

clobber : clean

clean :
	@rm -fR $(TMP)
