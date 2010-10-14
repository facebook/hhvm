
PROJECT_ROOT = ..
PROJECT_NAME = hphp_runtime

EVAL_DIR=runtime/eval

EXCLUDED_CXX_SOURCES := runtime/ext/sep/%.cpp $(if $(BOOTSTRAP),system/gen/%)
RECURSIVE_SOURCE_SUBDIRS := runtime/base runtime/ext system/gen util $(EVAL_DIR)
REC_SOURCES := $(filter-out $(EXCLUDED_CXX_SOURCES), \
	$(shell find $(RECURSIVE_SOURCE_SUBDIRS) -name "*.cpp" -o -name "*.c"))
CXX_NOOPT_SOURCES := $(filter %.no.cpp, $(REC_SOURCES))

PROGRAMS = test hphp hphpi compiler
SUB_CLEAN_DIRS = idl hphp runtime ../facebook/extensions

C_SOURCES := $(filter-out $(GENERATED_CPP_SOURCES) $(GENERATED_C_SOURCES),\
	$(filter %.c, $(REC_SOURCES)))
CXX_SOURCES := $(filter-out $(GENERATED_CXX_SOURCES) $(CXX_NOOPT_SOURCES),\
	$(filter %.cpp,$(REC_SOURCES)))

include $(PROJECT_ROOT)/src/rules.mk

# embed the git sha1 and branch information
GIT_SHA1 := $(shell git describe --all --long --abbrev=40 --always)
CPPFLAGS += -DCOMPILER_ID=\"$(GIT_SHA1)\"

EXCEPTIONS := $(EXT_DIR)/mozilla/%.a

STATIC_LIBS := $(subst libsicu,libicu,$(filter-out $(EXCEPTIONS), $(ALL_LIBS)))

SHARED_LIBS = $(EXT_DIR)/icu/lib/libicuuc.so \
              $(EXT_DIR)/binutils/libbinutils.so \
              $(shell find $(EXT_DIR)/mozilla -name "*.so") \
              $(patsubst %.a, %.so, $(STATIC_LIBS)) \
              $(LINK_LIBS) \

EXTERNAL += $(SHARED_LIBS)

TARGETS = $(PROGRAMS) $(SHARED_LIB) $(STATIC_LIB)

all: $(TARGETS)

$(PROGRAMS) : $(STATIC_LIB)
hphp-setup: compiler-setup
hphp: compiler
hphpi: hphp
test-setup: compiler-setup
test: compiler

INTERMEDIATE_DIRS += $(PROJECT_ROOT)/output_gd

shared-lib-gd:
	$(MAKE) OUTPUT_ROOT=output_gd OUTDIR_BY_TYPE= TLS_GD=1 DIRS_INCLUDED= $(ABS_PROJECT_ROOT)/output_gd/libhphp_runtime_gd.so
	cp $(PROJECT_ROOT)/output_gd/libhphp_runtime_gd.so $(PROJECT_ROOT)/bin
