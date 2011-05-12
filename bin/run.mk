
PROJECT_ROOT = $(HPHP_HOME)

ifndef PROJECT_NAME
PROJECT_NAME = program
endif

override OUTPUT_ROOT=
override OUTDIR_BY_TYPE=

# We want files to be sorted by size, so that larger files are dispatched by
# distcc earlier
CXX_NOOPT_SOURCES := $(shell echo `find . -name "*.no.cpp"`)
RECURSIVE_SOURCES := $(shell echo `find . -name "*.cpp"`)
SIZE_SORTED_SOURCES := $(wildcard ./sys/*.cpp) \
  $(shell echo `if [ -e cpp/ ]; then ls -S ./cpp/*.cpp; fi`)
CXX_SOURCES := $(filter-out $(CXX_NOOPT_SOURCES), $(SIZE_SORTED_SOURCES) \
  $(filter-out $(SIZE_SORTED_SOURCES), $(RECURSIVE_SOURCES)))

ifdef RANDOMIZE_CXX_SOURCES
CXX_SOURCES := $(shell echo $(CXX_SOURCES) | \
  perl -MList::Util=shuffle -ne'$$,=" "; print shuffle split/ /')
endif

-include sep_extensions.mk

CPPFLAGS += -I. $(SEP_EXTENSION_INCLUDE_PATHS)
LIBS = $(SEP_EXTENSION_LIBS) $(HPHP_LIB)/libhphp_runtime.a $(ALL_LIBS)

include $(HPHP_HOME)/src/rules.mk

ifdef HPHP_BUILD_LIBRARY

EXTERNAL += $(HPHP_LIB)/libhphp_runtime.so
HPHP_OBJ_DIR = $(HPHP_HOME)

ifdef HPHP_BUILD_FFI
EXTERNAL += $(HPHP_LIB)/libhphp_java.so
endif

RUNTIME_DIRS := $(wildcard \
	$(HPHP_LIB)/src/runtime/base \
	$(HPHP_LIB)/src/runtime/ext \
	$(HPHP_LIB)/src/runtime/eval \
	$(HPHP_LIB)/src/system/gen \
	$(HPHP_LIB)/src/system/lib \
	$(HPHP_LIB)/src/util)

ifeq ($(strip $(RUNTIME_DIRS)),)
RUNTIME_DIRS = \
	$(HPHP_OBJ_DIR)/src/runtime/base \
	$(HPHP_OBJ_DIR)/src/runtime/ext \
	$(HPHP_OBJ_DIR)/src/runtime/eval \
	$(HPHP_OBJ_DIR)/src/system/gen \
	$(HPHP_OBJ_DIR)/src/system/lib \
	$(HPHP_OBJ_DIR)/src/util
endif

ADDITIONAL_OBJS += \
	$(filter-out $(shell find $(RUNTIME_DIRS) -name "*.pic.o"), \
	$(shell find $(RUNTIME_DIRS) -name "*.o"))
TARGETS = $(STATIC_LIB) $(SHARED_LIB)

else # HPHP_BUILD_LIBRARY
TARGETS = $(APP_TARGET)
endif

all: $(TARGETS)

%.pp: %
	$(CXX) -E $(if $(OUT_TOP),-I$(OUT_TOP)src) $(CPPFLAGS) $(OPT) $(CXXFLAGS) $<
