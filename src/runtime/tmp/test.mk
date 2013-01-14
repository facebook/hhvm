
PROJECT_ROOT = ..
include $(PROJECT_ROOT)/src/dirs.mk

ifneq ($(filter TestCodeRunEval,$(SUITE)),)
TARGET = hphpi
endif
ifneq ($(filter TestCodeRunVM,$(SUITE)),)
TARGET = hhvm
endif
ifneq ($(filter TestCodeRunJit,$(SUITE)),)
TARGET = jit
endif
ifneq ($(filter TestCodeRun TestCodeRunStatic,$(SUITE)),)
TARGET = hphp
endif
ifneq ($(filter TestCodeRunRepo TestCodeRunRepoJit,$(SUITE)),)
TARGET = hphp
EXTRA_RUNTIME_OPTIONS:=-vEval.Jit=$(if $(filter TestCodeRunRepoJit,$(SUITE)),1,0)
endif

ZEND = /home/engshare/externals/cpp/hphp/centos-dev/php/bin/php
ZEND_WARN = -ddisplay_errors=stderr -dapc.enable_cli=1
ZEND_NOWARN = -ddisplay_errors=off -dapc.enable_cli=1

RUNCMD_jit = $(HHVM) -vEval.Jit=1 $(STD_OPTIONS)
RUNCMD_hphp = $(HPHP) -l1 $(call GET_OPTIONS,build.opts) $*/main.php --args=" $(OPTIONS)"
RUNCMD_hhvm = $(HHVM) -vEval.Jit=0 $(STD_OPTIONS)
RUNCMD_hphpi = $(HPHPI) $(STD_OPTIONS)

BUILD_hphp = $(HPHP) -tcpp -fexe -l1 -k1 -o $*/main $*/main.php $(if $(wildcard $*/config.hdf),-c$*/config.hdf)

GET_OPTIONS = $(if $(wildcard $*/$1),$(shell cat $*/$1) )

ifdef BUILD_$(TARGET)
BUILD_DEPS = %/test.build
BUILD = $(BUILD_$(TARGET))
ifdef LINK_SHARED
include $(LIB_DIR)/hphp_options
include $(LIB_DIR)/$(EXTERNALS).mk
SPACE:=
SPACE+=
export LD_LIBRARY_PATH := $(subst $(SPACE),:,$(foreach T,$(EXTERNAL_LIB_PATHS) $(LIB_DIR),$T))
endif
endif

VG = $(if $(VALGRIND),valgrind )

unexport SUB_OBJECTS

OPTIONS = --config=test/config.hdf $(call GET_OPTIONS,test.opts)
OPTIONS+= $(EXTRA_RUNTIME_OPTIONS)$(if $(VG), -vEval.VMStackElms=8192)
STD_OPTIONS = $(OPTIONS) -vRepo.Central.Path=$*/hhvm.hhbc -f $*/main.php
RUNCMD = $(VG)$(if $(RUNCMD_$(TARGET)),$(RUNCMD_$(TARGET)),$(TARGET) $(STD_OPTIONS))

ROOT := runtime/tmp/$(SUITE)
GROUP_PATHS := $(wildcard $(ROOT)/$(if $(GROUPS),$(GROUPS),Test*))
TESTS := $(wildcard $(addsuffix /tcr-*,$(GROUP_PATHS)))
GROUP_RESULTS := $(addsuffix /group.out,$(GROUP_PATHS))
GROUP_FAILURES := $(addsuffix /group.failures,$(GROUP_PATHS))

all: $(ROOT)/results
.PHONY: all group-failures

$(ROOT)/results : $(GROUP_RESULTS)
	$(V)$(RM) $@
	$(V)find $(GROUP_PATHS) -name test.status | xargs grep -L '^0$$' | wc -l >> $@
	$(V)find $(GROUP_PATHS) -name test.status | xargs grep -l '^0$$' | wc -l >> $@
	$(V)find $(GROUP_PATHS) -type d -name "tcr-*" | wc -l >> $@

# force the group.out files to be generated serially
$(foreach T, $(join $(addsuffix :|,$(wordlist 2,$(words $(GROUP_FAILURES)),$(GROUP_FAILURES)) group-failures), $(GROUP_FAILURES)), $(eval $T))

# make each group.failures file depend on the corresponding test.status files
$(foreach T, $(GROUP_PATHS), $(eval $T/group.failures : $(addsuffix /test.status,$(wildcard $T/tcr-*))))

define STATUS_DEPEND
  $(addsuffix /test.out,$(wildcard $(word 1,$1)/tcr-*)) :| $(word 2,$1)/group.out
endef

# make the test.out files depend on an earlier group, to force the group.out files to be generated reasonably early
# without this, the entire run completes before the first results appear
$(foreach T, $(join $(wordlist 4,$(words $(GROUP_PATHS)),$(GROUP_PATHS)), $(addprefix :,$(GROUP_PATHS))), $(if $(filter-out :%,$T),$(eval $(call STATUS_DEPEND,$(subst :, ,$T)))))


$(addsuffix /group.failures,$(GROUP_PATHS)) : %/group.failures :
	$(V)grep -L '^0$$' $(filter %/test.status,$^) >& $@ || true

$(GROUP_RESULTS) : %/group.out : %/group.failures
	$(V)sed -e 's%/test\.status$$%%' $< | \
	xargs -I% sh -c 'echo FAILED: % && cat %/main.php && echo && cat %/test.err && echo && cat %/test.diff' > $@
	$(V)if [ `wc -c $@ | cut -d' ' -f1` -eq 0 ] ; then echo $(notdir $*) passed; else \
		cat $@; echo $(notdir $*) Failed; fi

GENERATED_TEST_FILES = \
	main hhvm.hhbc test.exp test.build test.out test.err \
	test.diff "test.status*"

clean-failed:
	$(V)find $(GROUP_PATHS) -name test.status | xargs grep -L '^0$$' | xargs -rn1 dirname | \
	xargs -I% $(RM) -r $(addprefix %/,$(GENERATED_TEST_FILES))

clean:
	$(V)find $(GROUP_PATHS) -name main -o -name group.failures -o -name group.out \
	$(addprefix -o -name ,$(GENERATED_TEST_FILES)) | xargs $(RM) -r

$(addsuffix /test.exp,$(TESTS)) : %/test.exp :
	$(V)$(if $(wildcard $*/test.result),cp $*/test.result $@, \
	$(ZEND) $(if $(wildcard $*/nowarnings),$(ZEND_NOWARN),$(ZEND_WARN)) $*/main.php 2> /dev/null > $@ || true)

$(addsuffix /test.build,$(TESTS)) : %/test.build :
	$(V)$(BUILD)
	$(V)touch $@

$(addsuffix /test.out,$(TESTS)) : %/test.out :
	$(V)$(call GET_OPTIONS,test.env)$(RUNCMD) 2> $*/test.err > $@ || true

$(addsuffix /test.diff,$(TESTS)) : %/test.diff : %/test.exp %/test.out
	$(V)diff -u $^ >& $@ || true

$(addsuffix /test.status,$(TESTS)) : %/test.status : %/test.diff
	$(V)$(RM) $@*
	$(V)cat $*/test.diff $(if $(wildcard $*/nowarnings),,$*/test.err) | wc -c | cut -d' ' -f1 > $@
	$(V)if [ `cat $@` -eq 0 ] ; then touch $@.0 ; else touch $@.1 ; fi

