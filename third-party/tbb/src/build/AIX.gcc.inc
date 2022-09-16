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
LIBDL = -ldl

CPLUS = g++ 
CONLY = gcc
LIB_LINK_FLAGS = -shared
LIBS = -lpthread -ldl 
C_FLAGS = $(CPLUS_FLAGS) -x c

ifeq ($(cfg), release)
        CPLUS_FLAGS = -O2 -DUSE_PTHREAD -pthread
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -DTBB_USE_DEBUG -g -O0 -DUSE_PTHREAD -pthread
endif

ASM=
ASM_FLAGS=

TBB_ASM.OBJ=

ifeq (powerpc,$(arch))
    CPLUS_FLAGS    += -maix64 -Wl,-G
    LIB_LINK_FLAGS += -maix64 -Wl,-b64 -Wl,-brtl -Wl,-G
endif 

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------

ASSEMBLY_SOURCE=ibm_aix51
ifeq (powerpc,$(arch))
    TBB_ASM.OBJ = atomic_support.o
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
