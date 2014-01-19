
# This Makefile is only needed to run when zend_ini.x or zend_ini.y is modified

PROJECT_ROOT = ../../../..

zend_ini.lex.yy.cpp: zend_ini.x zend_ini.tab.cpp
	@echo "Generating scanner code..."
	$(V)flex -Pini_ -w -i -o$@ $<

zend_ini.tab.cpp: zend_ini.y
	@echo "Generating parser code..."
	$(V)bison -pini_ -d -o$@ $<

include $(PROJECT_ROOT)/src/rules.mk
