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

#------------------------------------------------------------------------------
# Overriding settings from windows.inc
#------------------------------------------------------------------------------

SLASH= $(strip \)
OBJ = o
LIBEXT = dll # MinGW allows linking with DLLs directly

TBB.RES =
MALLOC.RES =
RML.RES =
TBB.MANIFEST =
MALLOC.MANIFEST =
RML.MANIFEST =

ifeq (ia32,$(arch))
    TBB.LST = $(tbb_root)/src/tbb/lin32-tbb-export.lst
else
    TBB.LST = $(tbb_root)/src/tbb/win64-gcc-tbb-export.lst
endif
MALLOC.DEF = $(MALLOC_ROOT)/$(def_prefix)-gcc-tbbmalloc-export.def
RML.DEF = $(RML_SERVER_ROOT)/lin-rml-export.def

LINK_TBB.LIB = $(TBB.LIB)
# no TBB proxy for the configuration
PROXY.LIB =

#------------------------------------------------------------------------------
# End of overridden settings
#------------------------------------------------------------------------------
# Compiler-specific variables
#------------------------------------------------------------------------------

CPLUS = g++
COMPILE_ONLY = -c -MMD
PREPROC_ONLY = -E -x c++
INCLUDE_KEY = -I
DEFINE_KEY = -D
OUTPUT_KEY = -o #
OUTPUTOBJ_KEY = -o #
PIC_KEY =
WARNING_AS_ERROR_KEY = -Werror
WARNING_KEY = -Wall
TEST_WARNING_KEY = -Wextra -Wshadow -Wcast-qual -Woverloaded-virtual -Wnon-virtual-dtor -Wno-uninitialized
WARNING_SUPPRESS = -Wno-parentheses -Wno-uninitialized -Wno-non-virtual-dtor
DYLIB_KEY = -shared
LIBDL =
EXPORT_KEY = -Wl,--version-script,
LIBS = -lpsapi
BIGOBJ_KEY = -Wa,-mbig-obj

#------------------------------------------------------------------------------
# End of compiler-specific variables
#------------------------------------------------------------------------------
# Command lines
#------------------------------------------------------------------------------

LINK_FLAGS = -Wl,--enable-auto-import
LIB_LINK_FLAGS = $(DYLIB_KEY)

# gcc 4.8 and later support RTM intrinsics, but require command line switch to enable them
ifeq (ok,$(call detect_js,/minversion gcc 4.8))
    RTM_KEY = -mrtm
endif

# gcc 6.0 and later have -flifetime-dse option that controls
# elimination of stores done outside the object lifetime
ifeq (ok,$(call detect_js,/minversion gcc 6.0))
    # keep pre-contruction stores for zero initialization
    DSE_KEY = -flifetime-dse=1
endif

ifeq ($(cfg), release)
        CPLUS_FLAGS = -g -O2
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -g -O0 -DTBB_USE_DEBUG
endif

CPLUS_FLAGS += -DUSE_WINTHREAD
CPLUS_FLAGS += -D_WIN32_WINNT=$(_WIN32_WINNT)

# MinGW specific
CPLUS_FLAGS += -DMINGW_HAS_SECURE_API=1 -D__MSVCRT_VERSION__=0x0700 -msse -mthreads

CONLY = gcc
debugger = gdb
C_FLAGS = $(CPLUS_FLAGS)

ifeq (intel64,$(arch))
    CPLUS_FLAGS += -m64 $(RTM_KEY)
    LIB_LINK_FLAGS += -m64
endif

ifeq (ia32,$(arch))
    CPLUS_FLAGS += -m32 -march=i686 $(RTM_KEY)
    LIB_LINK_FLAGS += -m32
endif

# For examples
export UNIXMODE = 1

#------------------------------------------------------------------------------
# End of command lines
#------------------------------------------------------------------------------
# Setting assembler data
#------------------------------------------------------------------------------

ASM=
ASM_FLAGS=
TBB_ASM.OBJ=
ASSEMBLY_SOURCE=$(arch)-gas

#------------------------------------------------------------------------------
# End of setting assembler data
#------------------------------------------------------------------------------
# Setting tbbmalloc data
#------------------------------------------------------------------------------

M_CPLUS_FLAGS = $(CPLUS_FLAGS) -fno-rtti -fno-exceptions

#------------------------------------------------------------------------------
# End of setting tbbmalloc data
#------------------------------------------------------------------------------
