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

CPLUS = clang++
CONLY = clang
COMPILE_ONLY = -c -MMD
PREPROC_ONLY = -E -x c++
INCLUDE_KEY = -I
DEFINE_KEY = -D
OUTPUT_KEY = -o #
OUTPUTOBJ_KEY = -o #
PIC_KEY = -fPIC
WARNING_AS_ERROR_KEY = -Werror
WARNING_KEY = -Wall
TEST_WARNING_KEY = -Wextra -Wshadow -Wcast-qual -Woverloaded-virtual -Wnon-virtual-dtor
WARNING_SUPPRESS = -Wno-non-virtual-dtor -Wno-dangling-else
DYLIB_KEY = -dynamiclib
EXPORT_KEY = -Wl,-exported_symbols_list,
LIBDL = -ldl

LIBS = -lpthread
LINK_FLAGS =
LIB_LINK_FLAGS = -dynamiclib -install_name @rpath/$(BUILDING_LIBRARY)
C_FLAGS = $(CPLUS_FLAGS)

ifeq ($(cfg), release)
    CPLUS_FLAGS = -g -O2
else
    CPLUS_FLAGS = -g -O0 -DTBB_USE_DEBUG
endif

CPLUS_FLAGS += -DUSE_PTHREAD $(ITT_NOTIFY)

# For Clang, we add the option to support RTM intrinsics *iff* xtest is found in <immintrin.h>
ifneq (,$(shell grep xtest `echo "\#include<immintrin.h>" | clang -E -M - 2>&1 | grep immintrin.h` 2>/dev/null))
    RTM_KEY = -mrtm
endif

ifneq (,$(stdlib))
    CPLUS_FLAGS    += -stdlib=$(stdlib)
    LIB_LINK_FLAGS += -stdlib=$(stdlib)
endif

ifeq (intel64,$(arch))
    ITT_NOTIFY = -DDO_ITT_NOTIFY
    CPLUS_FLAGS += -m64 $(RTM_KEY)
    LINK_FLAGS += -m64
    LIB_LINK_FLAGS += -m64
endif

ifeq (ia32,$(arch))
    ITT_NOTIFY = -DDO_ITT_NOTIFY
    CPLUS_FLAGS += -m32 $(RTM_KEY)
    LINK_FLAGS += -m32
    LIB_LINK_FLAGS += -m32
endif

ifeq (ppc64,$(arch))
    CPLUS_FLAGS += -arch ppc64
    LINK_FLAGS += -arch ppc64
    LIB_LINK_FLAGS += -arch ppc64
endif

ifeq (ppc32,$(arch))
    CPLUS_FLAGS += -arch ppc
    LINK_FLAGS += -arch ppc
    LIB_LINK_FLAGS += -arch ppc
endif

ifeq ($(arch),$(filter $(arch),armv7 armv7s arm64))
    CPLUS_FLAGS += -arch $(arch)
    LINK_FLAGS += -arch $(arch)
    LIB_LINK_FLAGS += -arch $(arch)
endif

ifdef SDKROOT
    CPLUS_FLAGS += -isysroot $(SDKROOT)
    LINK_FLAGS += -L$(SDKROOT)/usr/lib/system -L$(SDKROOT)/usr/lib/
    LIB_LINK_FLAGS += -L$(SDKROOT)/usr/lib/system -L$(SDKROOT)/usr/lib/
endif

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------

ASM = as
ifeq (intel64,$(arch))
    ASM_FLAGS += -arch x86_64
endif
ifeq (ia32,$(arch))
    ASM_FLAGS += -arch i386
endif
ifeq ($(cfg), debug)
    ASM_FLAGS += -g
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

