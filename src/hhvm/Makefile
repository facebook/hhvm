PROJECT_ROOT = ../..
include $(PROJECT_ROOT)/src/dirs.mk

HHVM_EXTRA =

HHVM_EXT_LIB := $(HPHP_LIB)/libext_hhvm.a

HOOK_RULE_FILE = $(wildcard hook.mk)
ifeq ($(filter objects picobjects, $(MAKECMDGOALS)),)
  ifneq ($(strip $(HOOK_RULE_FILE)),)
   include hook.mk
  endif
endif

PHP_FILES = hhvm.php

all: $(HHVM)

$(HHVM): $(HPHP) $(PHP_FILES) $(HHVM_EXT_LIB) | $(OUT_DIR)gen/.mkdir
	@echo "Compiling hhvm..."
	$(V)HPHP_EMITTER=1 \
		HHVM_EXT_LIB=$(HHVM_EXT_LIB) $(HPHP) \
		-t cpp -f exe --input-dir . \
		-i $(PHP_FILES) -o $(OUT_DIR)gen --sync-dir=$(OUT_DIR)sync \
		-vEnableEval=2 --log=1 \
		$(HHVM_EXTRA) \
		--program=$(if $(OUT_TOP),,$(CWD)/)$@

clobber:
	$(V)rm -rf $(OUT_DIR)gen $(OUT_DIR)sync $(HHVM)

.PHONY: do-setup objects picobjects
do-setup objects picobjects:
	@true
