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
TEST_WARNING_KEY = -Wshadow -Wcast-qual -Woverloaded-virtual -Wnon-virtual-dtor -Wextra
WARNING_SUPPRESS = -Wno-parentheses -Wno-non-virtual-dtor
DYLIB_KEY = -shared
LIBDL = -ldl

CPLUS = g++ 
CONLY = gcc
LIB_LINK_FLAGS = -shared
LIBS = -lpthread -lrt -ldl 
C_FLAGS = $(CPLUS_FLAGS) -x c

ifeq ($(cfg), release)
        CPLUS_FLAGS = -g -O2 -DUSE_PTHREAD
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -DTBB_USE_DEBUG -g -O0 -DUSE_PTHREAD
endif

ASM=
ASM_FLAGS=

TBB_ASM.OBJ=

ifeq (ia64,$(arch))
# Position-independent code (PIC) is a must for IA-64
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

# for some gcc versions on Solaris, -m64 may imply V9, but perhaps not everywhere (TODO: verify)
ifeq (sparc,$(arch))
    CPLUS_FLAGS    += -mcpu=v9 -m64
    LIB_LINK_FLAGS += -mcpu=v9 -m64
endif 

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------
ASSEMBLY_SOURCE=$(arch)-gas
ifeq (ia64,$(arch))
    ASM=ias
    TBB_ASM.OBJ = atomic_support.o lock_byte.o log2.o pause.o
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
