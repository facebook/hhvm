
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

EXCEPTIONS := $(EXT_DIR)/mozilla/%.a $(EXT_DIR)/imap/lib/c-client.a \
              $(EXT_DIR)/google-perftools/lib/libtcmalloc.a \
              $(EXT_DIR)/google-perftools/lib/libtcmalloc_minimal.a \
              $(EXT_DIR)/jemalloc/lib/libjemalloc.a \

STATIC_LIBS := $(filter-out $(EXCEPTIONS), $(ALL_LIBS))

SHARED_LIBS = $(EXT_DIR)/binutils/libbinutils.so \
              $(EXT_DIR)/imap/lib/libc-client.so.2007 \
              $(shell find $(EXT_DIR)/mozilla -name "*.so") \
              $(patsubst %.a, %.so, $(STATIC_LIBS)) \
              $(LINK_LIBS) \

EXTERNAL += $(SHARED_LIBS)

TARGETS = $(PROGRAMS) $(STATIC_LIB)

all: $(TARGETS)

$(PROGRAMS) : $(STATIC_LIB)
hphp-setup: compiler-setup
hphp: compiler
hphpi: hphp
test-setup: compiler-setup
test: compiler shared-lib-target

.PHONY: shared-lib-target
shared-lib-target:
	$(V)$(MAKE) SHARED=1 $(SHARED_LIB)
