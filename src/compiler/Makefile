
PROJECT_ROOT = ../..
PROJECT_NAME = hphp_analysis

AUTO_SOURCES = 1
SOURCE_SUBDIRS = expression statement analysis util parser
INTERMEDIATE_FILES += $(OUT_DIR)analysis/code_error.inc $(OUT_DIR)analysis/dependency.inc
ALWAYS_ASSERT = 1

ERROR_INC := analysis/core_code_error.inc
DEPENDENCY_INC := analysis/core_dependency.inc
HOOK_RULE_FILE = $(wildcard hook.mk)
ifneq ($(strip $(HOOK_RULE_FILE)),)
 include hook.mk
endif
include $(PROJECT_ROOT)/src/rules.mk

TARGETS = $(STATIC_LIB)
all: $(TARGETS)

$(OUT_DIR)analysis/code_error.inc: $(ERROR_INC)
	$(V)cat $^ > $@

$(OUT_DIR)analysis/dependency.inc: $(DEPENDENCY_INC)
	$(V)cat $^ > $@

INC_FILES := $(OUT_DIR)analysis/code_error.inc $(OUT_DIR)analysis/dependency.inc
do-setup: $(INC_FILES)

$(OBJECTS) : $(INC_FILES)
