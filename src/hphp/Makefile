
PROJECT_ROOT = ../..
PROJECT_NAME = hphp

AUTO_SOURCES = 1

LIBS = \
	$(LIB_DIR)/libhphp_analysis.a \
	$(LIB_DIR)/libhphp_runtime.a \
	$(ALL_LIBS)

include $(PROJECT_ROOT)/src/dirs.mk

HOOK_RULE_FILE = $(wildcard hook.mk)
ifneq ($(strip $(HOOK_RULE_FILE)),)
 include hook.mk
endif

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(APP_TARGET)

GIT_SHA1 := $(shell git describe --all --long --abbrev=40 --always)
CPPFLAGS += -DCOMPILER_ID=\"$(GIT_SHA1)\"

all: $(TARGETS)
