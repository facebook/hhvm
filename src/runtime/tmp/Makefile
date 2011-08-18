PROJECT_NAME=run
PROJECT_ROOT=$(HPHP_HOME)

CXX_SOURCES := run.cpp

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(APP_TARGET)

LDFLAGS += -ldl
override OUTDIR_BY_TYPE=
override OUTPUT_ROOT=

TEST_DIRS = $(wildcard Test*)

.PHONY : all
all : $(TEST_DIRS) $(TARGETS) run.sh

.PHONY : $(TEST_DIRS)
$(TEST_DIRS):
	@$(MAKE) -C $@

run.sh : $(HPHP_HOME)/src/rules.mk
	@$(MAKE) -f script.mk
	@chmod +x $@

TMP = $(TEST_DIRS) \
      run run.sh

clobber : clean

clean :
	@rm -fR $(TMP)
