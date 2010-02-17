
PROJECT_ROOT = ../..
PROJECT_NAME = hphp_analysis

AUTO_SOURCES = 1
SOURCE_SUBDIRS = expression statement analysis system util

GENERATED_CXX_SOURCES = parser/lex.yy.cpp parser/hphp.tab.cpp
CXX_SOURCES = $(filter-out $(GENERATED_CXX_SOURCES), $(wildcard parser/*.cpp))
INTERMEDIATE_FILES = $(wildcard parser/*~) $(wildcard parser/*.d) \
	parser/hphp.tab.hpp parser/hphp.tab.cpp parser/lex.yy.cpp

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(STATIC_LIB)
all: $(TARGETS)

parser/lex.yy.cpp: parser/hphp.x parser/hphp.tab.cpp
	@echo "Generating scanner code..."
	$(V)cd parser && flex -w -i -olex.yy.cpp hphp.x

parser/hphp.tab.cpp: parser/hphp.y
	@echo "Generating parser code..."
	$(V)cd parser && bison -d -o hphp.tab.cpp hphp.y

parser/ylmm/basic_scanner.hh: \
	parser/ylmm/basic_buffer.hh \
	parser/ylmm/basic_location.hh \
	parser/ylmm/basic_messenger.hh \

parser/ylmm/basic_parser.hh: \
	parser/ylmm/basic_location.hh \
	parser/ylmm/basic_messenger.hh \

ERROR_INC := analysis/core_code_error.inc
DEPENDENCY_INC := analysis/core_dependency.inc
HOOK_RULE_FILE = $(wildcard hook.mk)
ifneq ($(strip $(HOOK_RULE_FILE)),)
 include hook.mk
endif

$(PROJECT_ROOT)/src/lib/analysis/code_error.inc: $(ERROR_INC)
	$(V)cat $^ > $@

$(PROJECT_ROOT)/src/lib/analysis/dependency.inc: $(DEPENDENCY_INC)
	$(V)cat $^ > $@

$(OBJECTS): $(PROJECT_ROOT)/src/lib/analysis/code_error.inc
$(OBJECTS): $(PROJECT_ROOT)/src/lib/analysis/dependency.inc

clobber:
	$(V)rm -f analysis/code_error.inc
	$(V)rm -f analysis/dependency.inc
	$(V)rm -f $(INTERMEDIATE_FILES)
