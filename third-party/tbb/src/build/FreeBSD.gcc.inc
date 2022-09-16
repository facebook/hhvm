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
PIC_KEY = -fPIC
WARNING_AS_ERROR_KEY = -Werror
WARNING_KEY = -Wall
DYLIB_KEY = -shared
WARNING_SUPPRESS = -Wno-parentheses

CPLUS = g++ 
CONLY = gcc
LIB_LINK_FLAGS = -shared
LIBS = -lpthread 
C_FLAGS = $(CPLUS_FLAGS)

# gcc 6.0 and later have -flifetime-dse option that controls
# elimination of stores done outside the object lifetime
ifneq (,$(shell gcc -dumpversion | egrep  "^([6-9])"))
    # keep pre-contruction stores for zero initialization
    DSE_KEY = -flifetime-dse=1
endif

ifeq ($(cfg), release)
        CPLUS_FLAGS = -g -O2 -DUSE_PTHREAD
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -DTBB_USE_DEBUG -g -O0 -DUSE_PTHREAD
endif

ASM=
ASM_FLAGS=

TBB_ASM.OBJ=
MALLOC_ASM.OBJ=

ifeq (ia64,$(arch))
# Position-independent code (PIC) is a must on IA-64 architecture, even for regular (not shared) executables
    CPLUS_FLAGS += $(PIC_KEY)
endif 

ifeq (intel64,$(arch))
    CPLUS_FLAGS += -m64
    LIB_LINK_FLAGS += -m64
endif 

ifeq (ia32,$(arch))
    CPLUS_FLAGS += -m32
    LIB_LINK_FLAGS += -m32
endif 

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------
ASSEMBLY_SOURCE=$(arch)-gas
ifeq (ia64,$(arch))
    ASM=as
    TBB_ASM.OBJ = atomic_support.o lock_byte.o log2.o pause.o
    MALLOC_ASM.OBJ = atomic_support.o lock_byte.o pause.o
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
