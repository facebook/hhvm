
# This Makefile is only needed to run when hphp.x or hphp.y is modified

PROJECT_ROOT = ../../../..

lex.eval_.cpp: hphp.x hphp.tab.cpp
	@echo "Generating scanner code..."
	$(V)flex -w -i -o$@ -Peval_ $<
	@php $(PROJECT_ROOT)/bin/license.php

hphp.tab.cpp: hphp.y Makefile
	@echo "Generating parser code..."
	$(V)bison -v -d -o $@ -p eval_ $<
	@perl -p -i -n -e "s/union/struct/" $@
	@perl -p -i -n -e "s/YYSTACK_ALLOC \(YYSTACK_BYTES \(yystacksize\)\);\n/YYSTACK_ALLOC \(YYSTACK_BYTES \(yystacksize\)\);\n        memset(yyptr, 0, YYSTACK_BYTES (yystacksize));\n/" $@

include $(PROJECT_ROOT)/src/rules.mk
