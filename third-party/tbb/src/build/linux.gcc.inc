# Copyright (c) 2005-2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
#
#

COMPILE_ONLY = -c -MMD
PREPROC_ONLY = -E -x c++
INCLUDE_KEY = -I
DEFINE_KEY = -D
OUTPUT_KEY = -o #
OUTPUTOBJ_KEY = -o #
PIC_KEY = #-fPIC
WARNING_AS_ERROR_KEY = -Werror
WARNING_KEY = -Wall
TEST_WARNING_KEY = -Wshadow -Wcast-qual -Woverloaded-virtual -Wnon-virtual-dtor

WARNING_SUPPRESS = -Wno-parentheses -Wno-non-virtual-dtor
DYLIB_KEY = -shared
EXPORT_KEY = -Wl,--version-script,
LIBDL = -ldl

CPLUS = g++
CONLY = gcc
LIB_LINK_FLAGS = $(DYLIB_KEY) -Wl,-soname=$(BUILDING_LIBRARY)
LIBS += -lpthread -lrt
LINK_FLAGS = -Wl,-rpath-link=. -rdynamic
C_FLAGS = $(CPLUS_FLAGS)

# gcc 4.2 and higher support OpenMP
ifneq (,$(shell gcc -dumpversion | egrep  "^(4\.[2-9]|[5-9])"))
    OPENMP_FLAG = -fopenmp
endif

# @ntv note
# https://software.intel.com/en-us/forums/topic/558688
# custom built compiler built without -mrtm support apparently
## gcc 4.8 and later support RTM intrinsics, but require command line switch to enable them
#ifneq (,$(shell gcc -dumpversion | egrep  "^(4\.[8-9]|[5-9])"))
#    RTM_KEY = -mrtm
#endif

# gcc 4.0 and later have -Wextra that is used by some our customers.
ifneq (,$(shell gcc -dumpversion | egrep  "^([4-9])"))
    TEST_WARNING_KEY += -Wextra
endif

# gcc 5.0 and later have -Wsuggest-override option
# enable it via a pre-included header in order to limit to C++11 and above
ifneq (,$(shell gcc -dumpversion | egrep  "^([5-9])"))
    INCLUDE_TEST_HEADERS = -include $(tbb_root)/src/test/harness_preload.h
endif

# gcc 6.0 and later have -flifetime-dse option that controls
# elimination of stores done outside the object lifetime
ifneq (,$(shell $(CONLY) -dumpfullversion -dumpversion | egrep  "^([6-9]|1[0-9])"))
    # keep pre-contruction stores for zero initialization
    DSE_KEY = -flifetime-dse=1
endif

ifeq ($(cfg), release)
        CPLUS_FLAGS = $(ITT_NOTIFY) -g -O2 -DUSE_PTHREAD
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -DTBB_USE_DEBUG $(ITT_NOTIFY) -g -O0 -DUSE_PTHREAD
endif

TBB_ASM.OBJ=
MALLOC_ASM.OBJ=

ifeq (ia64,$(arch))
# Position-independent code (PIC) is a must on IA-64 architecture, even for regular (not shared) executables
    CPLUS_FLAGS += $(PIC_KEY)
endif

ifeq (intel64,$(arch))
    ITT_NOTIFY = -DDO_ITT_NOTIFY
    CPLUS_FLAGS += -m64 $(RTM_KEY)
    LIB_LINK_FLAGS += -m64
endif

ifeq (ia32,$(arch))
    ITT_NOTIFY = -DDO_ITT_NOTIFY
    CPLUS_FLAGS += -m32 -march=pentium4 $(RTM_KEY)
    LIB_LINK_FLAGS += -m32
endif

ifeq (ppc64,$(arch))
    CPLUS_FLAGS += -m64
    LIB_LINK_FLAGS += -m64
endif

ifeq (ppc32,$(arch))
    CPLUS_FLAGS += -m32
    LIB_LINK_FLAGS += -m32
endif

ifeq (bg,$(arch))
    CPLUS = $(firstword $(notdir $(shell which powerpc{64,32,}-bg{z..a}-linux-g++ 2>/dev/null)))
    CONLY = $(firstword $(notdir $(shell which powerpc{64,32,}-bg{z..a}-linux-gcc 2>/dev/null)))
endif

# for some gcc versions on Solaris, -m64 may imply V9, but perhaps not everywhere (TODO: verify)
ifeq (sparc,$(arch))
    CPLUS_FLAGS    += -mcpu=v9 -m64
    LIB_LINK_FLAGS += -mcpu=v9 -m64
endif

# automatically generate "IT" instructions when compiling for Thumb ISA
ifeq (armv7,$(arch))
    CPLUS_FLAGS    += -Wa,-mimplicit-it=thumb
endif

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------
ASM = as
ifeq (intel64,$(arch))
    ASM_FLAGS += --64
endif
ifeq (ia32,$(arch))
    ASM_FLAGS += --32
endif
ifeq ($(cfg),debug)
    ASM_FLAGS += -g
endif

ASSEMBLY_SOURCE=$(arch)-gas
ifeq (ia64,$(arch))
    ASM_FLAGS += -xexplicit
    TBB_ASM.OBJ += atomic_support.o lock_byte.o log2.o pause.o ia64_misc.o
    MALLOC_ASM.OBJ += atomic_support.o lock_byte.o pause.o log2.o
endif
#------------------------------------------------------------------------------
# End of setting assembler data.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Setting tbbmalloc data.
#------------------------------------------------------------------------------

M_CPLUS_FLAGS = $(CPLUS_FLAGS) -fno-rtti -fno-exceptions

#------------------------------------------------------------------------------
# End of setting tbbmalloc data.
#------------------------------------------------------------------------------
