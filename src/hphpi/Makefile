
PROJECT_ROOT = ../..
include $(PROJECT_ROOT)/src/dirs.mk

HPHP = $(if $(OUT_TOP),$(LIB_DIR),$(PROJECT_ROOT)/src/hphp)/hphp
HPHPI = $(OUT_TOP)hphpi

PHP_FILES = hphpi.php

all: $(HPHPI)

$(HPHPI): $(HPHP) $(PHP_FILES)
	@echo "Compiling hphpi..."
	+$(V)$(if $(OUT_TOP),HPHP_LIB=$(LIB_DIR)) $(HPHP) \
		-t cpp -f exe --input-dir . \
		-i $(PHP_FILES) -o $(OUT_DIR)gen \
		-vEnableEval=2 --log=1 \
		--program=$(if $(OUT_TOP),,$(CWD)/)$@

clobber:
	$(V)rm -rf $(OUT_DIR)gen $(HPHPI)

.PHONY: do-setup objects
do-setup objects:
	@true

