
PROJECT_ROOT = ..
PROJECT_NAME = hphp_runtime

EVAL_DIR=runtime/eval

EXCLUDED_CXX_SOURCES := \
	runtime/ext/sep/%.cpp \
	$(if $(BOOTSTRAP),system/gen/%) \
	runtime/ext/fql/lemon.c \
	runtime/ext/fql/lempar.c \

RECURSIVE_SOURCE_SUBDIRS := runtime/base runtime/ext system/gen system/lib \
  util $(EVAL_DIR)
REC_SOURCES := $(filter-out $(EXCLUDED_CXX_SOURCES), \
	$(shell find $(RECURSIVE_SOURCE_SUBDIRS) -name "*.cpp" -o -name "*.c" -o -name "*.S"))
CXX_NOOPT_SOURCES := $(filter %.no.cpp, $(REC_SOURCES))

PROGRAMS = test hphp hphpi compiler tools
SUB_CLEAN_DIRS = idl hphp runtime ../facebook/extensions

include $(PROJECT_ROOT)/src/dirs.mk

C_SOURCES := $(filter-out $(GENERATED_CPP_SOURCES) $(GENERATED_C_SOURCES),\
	$(filter %.c, $(REC_SOURCES)))
CXX_SOURCES := $(filter-out $(GENERATED_CXX_SOURCES) $(CXX_NOOPT_SOURCES),\
	$(filter %.cpp,$(REC_SOURCES)))

ASM_SOURCES := $(filter %.S, $(REC_SOURCES))

ASM_SOURCES += $(wildcard $(PROJECT_ROOT)/facebook/src/util/*.S)

include $(PROJECT_ROOT)/src/rules.mk

# embed the git sha1 and branch information
GIT_SHA1 := $(shell git describe --all --long --abbrev=40 --always)
CPPFLAGS += -DCOMPILER_ID=\"$(GIT_SHA1)\"

EXTERNAL += $(EXTERNAL_SHARED_LIBS)

TARGETS = $(PROGRAMS) $(STATIC_LIB)

all: $(TARGETS)

$(PROGRAMS) : $(STATIC_LIB)
hphp-setup: compiler-setup
hphp: compiler
hphpi: hphp | test-obj
test-setup: compiler-setup
test: compiler shared-lib-target

SMART_ALLOCATOR_INC = runtime/base/memory/smart_allocator.core.inc

HOOK_RULE_FILE = $(wildcard compiler/hook.mk)
ifneq ($(strip $(HOOK_RULE_FILE)),)
 include hook.mk
endif

SA_INC_GEN = runtime/base/memory/smart_allocator.inc_gen

$(SA_INC_GEN): $(SMART_ALLOCATOR_INC)
	$(V)echo $(SMART_ALLOCATOR_INC)
	$(V)cat $(SMART_ALLOCATOR_INC) > $(SA_INC_GEN)

.PHONY: shared-lib-target
shared-lib-target:
	$(V)$(MAKE) SHARED=1 $(SHARED_LIB)

compiler-setup: $(SA_INC_GEN)