
PROJECT_ROOT = ../..
include $(PROJECT_ROOT)/src/dirs.mk

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
