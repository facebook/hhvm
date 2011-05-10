###############################################################################
#
# Handle command line switches, and directories
#
# Author: mwilliams (02/2010)
#
###############################################################################
#
# NOTE: $(PWD) is not set correctly when make is invoked
# using "make -Cdir". We need the actual current
# directory, not the one that make was invoked from
CWD := $(shell readlink -f `pwd`)

ifeq ($(notdir $(MAKE)),emake)
export MAKE
override USE_CCACHE :=
override NO_DISTCC := 1
endif

ifneq ($(origin DIRS_INCLUDED), override)
override DIRS_INCLUDED := 1
unexport DIRS_INCLUDED

# This is to make sure "make" without any target will actually "make all".
overall: all quiet-1

# Add quiet as a dependent to prevent "nothing to do for... warnings from make"
.PHONY: quiet quiet-%
quiet quiet-%:
	@true

###############################################################################
# Command line switches. For example, "make RELEASE=1".

V ?= @
NO_PRINT = $(if $(V),--no-print-directory,)
INFINITE_LOOP_DETECTION = 1
INFINITE_RECURSION_DETECTION = 1
REQUEST_TIMEOUT_DETECTION = 1

# This requires the patched libmysql
MYSQL_MILLISECOND_TIMEOUT = 1

ENABLE_LATE_STATIC_BINDING = 1

-include $(wildcard $(PROJECT_ROOT)/local/*.mk)

# This normally generates debug symbols, but you may also use this in your
# code to output extra debugging information.
#DEBUG = 1
#DEBUG_MEMORY_LEAK = 1
#DEBUG_APC_LEAK = 1
#DEBUG_RACE_CONDITION = 1

ifdef RELEASE
override DEBUG=
override DEBUG_MEMORY_LEAK=
override DEBUG_RACE_CONDITION=
override RELEASE=1
endif

ifndef DEBUG_MEMORY_LEAK
ifndef DEBUG_RACE_CONDITION

# This normally adds -O3 tag to generate the most optimized code targeted for
# production build.
ifndef DEBUG
RELEASE = 1
endif

# For hotprofiler instrumentation
HOTPROFILER = 1

# Use jemalloc by default.
ifndef NO_JEMALLOC
USE_JEMALLOC = 1
endif

# Only use jemalloc *or* tcmalloc.
ifdef USE_JEMALLOC
override NO_TCMALLOC = 1
override GOOGLE_TCMALLOC =
endif # USE_JEMALLOC

ifndef NO_TCMALLOC
# For google profilers
#GOOGLE_CPU_PROFILER = 1
#GOOGLE_HEAP_PROFILER = 1

# Whether to link with tcmalloc.a
GOOGLE_TCMALLOC = 1
endif

# For GNU profiler - gprof.
#PROFILE = 1

# For GNU coverage - gcov.
#COVERAGE = 1

endif
endif

ifndef NO_SNAPPY
HAVE_SNAPPY = 1
endif

###############################################################################
# Directories

ifdef USE_ICC
ifndef ICC
override USE_ICC:=
endif
endif

ifdef OUTDIR_BY_TYPE
ifndef OUTPUT_ROOT
OUTPUT_ROOT := bin
endif
OUT_EXTS := \
	$(if $(USE_LLVM),-llvm) \
	$(if $(USE_ICC),-icc) \
	$(if $(USE_JEMALLOC),-je) \
	$(if $(NO_TCMALLOC),,-tc) \
	$(if $(PROFILE),-pg) \
	$(if $(DEBUG),-g,-O)

EMPTY:=
SPACE:=$(EMPTY) $(EMPTY)
OUT_EXT := $(subst $(SPACE),,$(strip $(OUT_EXTS)))
endif

ABS_PROJECT_ROOT := $(shell cd $(PROJECT_ROOT) && readlink -f `pwd`)

ifdef OUTPUT_ROOT

REL := $(patsubst $(ABS_PROJECT_ROOT)%,%,$(CWD))

OUTPUT_REL := $(patsubst /%,,$(patsubst ~%,,$(OUTPUT_ROOT)))
OUT_TOP_BASE := $(if $(OUTPUT_REL),$(ABS_PROJECT_ROOT)/)$(OUTPUT_ROOT)

OUT_TOP := $(OUT_TOP_BASE)$(OUT_EXT)
OUT_DIR := $(OUT_TOP)$(REL)/
LIB_DIR := $(OUT_TOP)
OUT_TOP := $(OUT_TOP)/
HPHP := $(OUT_TOP)hphp
HPHPI := $(OUT_TOP)hphpi

else

OUT_TOP :=
OUT_DIR :=
LIB_DIR := $(PROJECT_ROOT)/bin
ifdef HPHP_LIB
ifneq ($(HPHP_LIB),$(HPHP_ROOT)/bin)
LIB_DIR := $(HPHP_LIB)
endif
endif
HPHP := $(PROJECT_ROOT)/src/hphp/hphp
HPHPI := $(PROJECT_ROOT)/src/hphpi/hphpi

endif

ifndef HPHP_LIB
HPHP_LIB := $(ABS_PROJECT_ROOT)/bin
endif

ifdef SHARED
# Clear USE_JEMALLOC, since it may have already been set by the parent make.
# Ideally we would actually undefine USE_JEMALLOC:
#   override undefine USE_JEMALLOC
# However, the undefine feature is only available in GNU make 3.82 and later.
override USE_JEMALLOC =
override GOOGLE_TCMALLOC =
endif

MKDIR = mkdir -p
RMDIR = rm -fR
EXT_DIR = $(PROJECT_ROOT)/external-$(OS)

%/.mkdir :
	$(V)-$(MKDIR) $(@D)
	$(V)touch $@

dirinfo:
	@echo $(ABS_PROJECT_ROOT) $(OUT_TOP) $(if $(PROFILE),P)$(if $(DEBUG),D,R)$(if $(USE_ICC),-I)$(if $(USE_LLVM),-L)

endif
