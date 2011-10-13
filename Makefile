PROJECT_ROOT = .
include src/dirs.mk

TEST := $(if $(OUT_TOP),$(OUT_TOP),test/)test

TOBUILD := $(filter clean% clobber% both debug release, $(MAKECMDGOALS))
CLEAN := $(filter clean% clobber%,$(MAKECMDGOALS))
TOTEST := $(filter-out $(TOBUILD), $(MAKECMDGOALS))

ifeq ($(if $(TOBUILD),1)$(if $(TOTEST),1),11)
$(TOTEST) : $(TOBUILD)
endif

ifneq ($(filter fast_tests slow_tests $(FAST_TESTS) $(SLOW_TESTS) TestCodeRun%,$(MAKECMDGOALS)),)
# run all tests, even if some fail
MAKEFLAGS += k
.NOTPARALLEL:
endif

QuickTests = "" "" $@
TestExt = "" "" $@
FAST_TESTS := QuickTests TestExt TestCodeRunEval
SLOW_TESTS := TestCodeRun TestServer

all: fast_tests
tags: ctags etags
ctags:
	-$(V)cd src && ct
etags:
	-$(V)cd src && ct -e
.PHONY: tags ctags etags

$(FAST_TESTS) $(SLOW_TESTS) TestCodeRunStatic: % : setup
	cd src && $(TEST) $(if $($@),$($@),$@)

setup: $(CLEAN)
	$(MAKE) -C src

fast_tests: $(FAST_TESTS)
slow_tests: $(SLOW_TESTS)

.PHONY: $(FAST_TESTS) $(SLOW_TESTS) TestCodeRun%

TestCodeRun-% TestCodeRunEval-% TestCodeRunStatic-% TestServer-% : setup
	cd src && $(TEST) $(patsubst %-$*,%,$@) Test$*

.PHONY: debug release both check_by_type fast_tests slow_tests setup
check_by_type: $(CLEAN)
	@if [ -z "$$OUTDIR_BY_TYPE" ] ; then \
	  echo "You need to set OUTDIR_BY_TYPE to build both DEBUG and RELEASE into the same client"; \
	  exit 1; \
	fi

release: check_by_type
	$(MAKE) -Csrc RELEASE=1 DEBUG=

debug: check_by_type
	$(MAKE) -Csrc RELEASE= DEBUG=1

both: debug release

clobber:
	$(MAKE) -C src clobber
	$(MAKE) -C facebook clobber

clean: clobber
