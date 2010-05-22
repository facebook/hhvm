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

# only want to do this once per invocation of make
# (particularly the build rules)
# unfortunately, the variables get passed down to
# recursive makes (but with the wrong values), so
# check here whether we've already run at this level
ifneq ($(CWD),$(DIRS_INCLUDED))

DIRS_INCLUDED := $(CWD)

###############################################################################
# Command line switches. For example, "make RELEASE=1".

V ?= @
NO_PRINT = $(if $(V),--no-print-directory,)
INFINITE_LOOP_DETECTION = 1
INFINITE_RECURSION_DETECTION = 1
REQUEST_TIMEOUT_DETECTION = 1

#MYSQL_MILLISECOND_TIMEOUT = 1;

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

# Only use jemalloc *or* tcmalloc.
ifdef USE_JEMALLOC
NO_TCMALLOC = 1
endif

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

###############################################################################
# Directories

ifdef OUTDIR_BY_TYPE
ifndef OUTPUT_ROOT
OUTPUT_ROOT := bin
OUT_EXT=$(if $(DEBUG),-g,-O)
endif
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

else

OUT_TOP :=
OUT_DIR :=
LIB_DIR := $(PROJECT_ROOT)/bin
ifdef HPHP_LIB
ifneq ($(HPHP_LIB),$(HPHP_ROOT)/bin)
LIB_DIR := $(HPHP_LIB)
endif
endif

endif

ifndef HPHP_LIB
HPHP_LIB := $(ABS_PROJECT_ROOT)/bin
endif

MKDIR = mkdir -p
RMDIR = rm -fR
EXT_DIR = $(PROJECT_ROOT)/external-$(OS)

%/.mkdir :
	$(V)-$(MKDIR) $(@D)
	$(V)touch $@

endif
