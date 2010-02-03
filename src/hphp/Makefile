
PROJECT_ROOT = ../..
PROJECT_NAME = hphp

AUTO_SOURCES = 1

LIBS = \
	$(LIB_DIR)/libhphp_analysis.a \
	$(LIB_DIR)/libhphp_runtime.a \
	$(ALL_LIBS)

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(APP_TARGET)

HOOK_RULE_FILE = $(wildcard hook.mk)
ifneq ($(strip $(HOOK_RULE_FILE)),)
 include hook.mk
endif

all: $(TARGETS)
