
PROJECT_ROOT = ../../..

all: lex.yy.cpp
	$(MAKE) -C $(PROJECT_ROOT)/src/compiler/parser
	$(MAKE) -C $(PROJECT_ROOT)/src/runtime/eval/parser
	@php $(PROJECT_ROOT)/bin/license.php

.PHONY: lex.yy.cpp
lex.yy.cpp: hphp.x
	@echo "Generating scanner code..."
	$(V)$(EXT_DIR)/flex/bin/flex -i -f -Phphp -R -8 --bison-locations \
		-o$@ $<

include $(PROJECT_ROOT)/src/rules.mk
