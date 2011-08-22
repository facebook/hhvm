PROJECT_ROOT=$(HPHP_HOME)

override OUTDIR_BY_TYPE=
override OUTPUT_ROOT=

include $(PROJECT_ROOT)/src/rules.mk
MONO_TARGETS = run
TARGETS = $(MONO_TARGETS)

LDFLAGS += -ldl
TEST_DIRS = $(wildcard Test*)

.PHONY : all
all : $(TEST_DIRS) $(TARGETS) run.sh

.PHONY : $(TEST_DIRS)
$(TEST_DIRS):
	@$(MAKE) -C $@

run.sh : $(PROJECT_ROOT)/src/rules.mk
	@$(MAKE) -f script.mk
	@chmod +x $@

TMP = $(TEST_DIRS) \
      $(MONO_TARGETS) run.sh

clobber : clean

clean :
	@rm -fR $(TMP)
