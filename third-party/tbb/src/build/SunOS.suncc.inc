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

COMPILE_ONLY = -c -xMMD -errtags
PREPROC_ONLY = -E -xMMD
INCLUDE_KEY = -I
DEFINE_KEY = -D
OUTPUT_KEY = -o #
OUTPUTOBJ_KEY = -o #
PIC_KEY = -KPIC
DYLIB_KEY = -G
LIBDL = -ldl
# WARNING_AS_ERROR_KEY = -errwarn=%all
WARNING_AS_ERROR_KEY = Warning as error
# Supported Solaris Studio* 12.2 and above, remove ',inlasmpnu' in the line below to build by compiler prior Solaris Studio* 12.2 
WARNING_SUPPRESS = -erroff=unassigned,attrskipunsup,badargtype2w,badbinaryopw,wbadasg,wvarhidemem,inlasmpnu
tbb_strict=0

CPLUS = CC
CONLY = cc

OPENMP_FLAG = -xopenmp
LIB_LINK_FLAGS = -G -R . -M$(tbb_root)/build/suncc.map.pause
LINK_FLAGS += -M$(tbb_root)/build/suncc.map.pause
LIBS = -lpthread -lrt -R .
C_FLAGS = $(CPLUS_FLAGS)

#TODO: the $(stdlib) instead of hard-wiring STLPort 
ifeq ($(cfg), release)
        CPLUS_FLAGS = -mt -xO2 -g -library=stlport4 -DUSE_PTHREAD $(WARNING_SUPPRESS)
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -mt -DTBB_USE_DEBUG -g -library=stlport4 -DUSE_PTHREAD $(WARNING_SUPPRESS)
endif

ASM=
ASM_FLAGS=

TBB_ASM.OBJ=

ifeq (intel64,$(arch))
    CPLUS_FLAGS += -m64
    ASM_FLAGS += -m64
    LIB_LINK_FLAGS += -m64
endif 

ifeq (ia32,$(arch))
    CPLUS_FLAGS += -m32
    LIB_LINK_FLAGS += -m32
endif 

# TODO: verify whether -m64 implies V9 on relevant Sun Studio versions
#       (those that handle gcc assembler syntax)
ifeq (sparc,$(arch))
    CPLUS_FLAGS    += -m64
    LIB_LINK_FLAGS += -m64
endif 

export TBB_CUSTOM_VARS_SH=export CXXFLAGS="-I$${TBBROOT}/include -library=stlport4 $(CXXFLAGS) -M$${TBBROOT}/build/suncc.map.pause"
export TBB_CUSTOM_VARS_CSH=setenv CXXFLAGS "-I$${TBBROOT}/include -library=stlport4 $(CXXFLAGS) -M$${TBBROOT}/build/suncc.map.pause"

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------
ASSEMBLY_SOURCE=$(arch)-fbe
#------------------------------------------------------------------------------
# End of setting assembler data.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Setting tbbmalloc data.
#------------------------------------------------------------------------------
M_INCLUDES = $(INCLUDES) -I$(MALLOC_ROOT) -I$(MALLOC_SOURCE_ROOT)
M_CPLUS_FLAGS = $(CPLUS_FLAGS)
#------------------------------------------------------------------------------
# End of setting tbbmalloc data.
#------------------------------------------------------------------------------
