PROJECT_ROOT = .
include src/dirs.mk

TEST := $(if $(OUT_TOP),$(OUT_TOP),test/)test

ifneq ($(OUT_TOP),)
COPY = && cp $(OUT_TOP)/hphpi src/hphpi/hphpi && cp $(OUT_TOP)/hphp src/hphp/hphp
else
COPY =
endif

ifneq ($(filter fast_tests slow_tests,$(MAKECMDGOALS)),)
# run all tests, even if some fail
MAKEFLAGS += k
.NOTPARALLEL:
endif

QuickTests = "" "" $@
TestExt = "" "" $@
FAST_TESTS := QuickTests TestExt TestCodeRunEval
SLOW_TESTS := TestCodeRun TestServer

all: fast_tests

$(FAST_TESTS) $(SLOW_TESTS): % : setup
	cd src && $(TEST) $(if $($@),$($@),$@)

setup:
	$(MAKE) -C src $(COPY)
fast_tests: $(FAST_TESTS)
slow_tests: $(SLOW_TESTS)

.PHONY: $(FAST_TESTS) $(SLOW_TESTS)

.PHONY: debug release both check_by_type fast_tests slow_tests setup
check_by_type:
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
