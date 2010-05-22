
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

-include sep_extensions.mk

CPPFLAGS += -I. $(SEP_EXTENSION_INCLUDE_PATHS)
LIBS = $(SEP_EXTENSION_LIBS) $(HPHP_LIB)/libhphp_runtime.a $(ALL_LIBS)

include $(HPHP_HOME)/src/rules.mk

ifdef HPHP_BUILD_LIBRARY
TARGETS = $(STATIC_LIB) $(SHARED_LIB)
else
TARGETS = $(APP_TARGET)
endif

all: $(TARGETS)

ifdef HPHP_BUILD_FFI
EXTERNAL += $(HPHP_LIB)/libhphp_java.so
endif
