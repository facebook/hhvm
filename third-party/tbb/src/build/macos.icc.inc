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

CPLUS = icpc
CONLY = icc
COMPILE_ONLY = -c -MMD
PREPROC_ONLY = -E -x c++
INCLUDE_KEY = -I
DEFINE_KEY = -D
OUTPUT_KEY = -o #
OUTPUTOBJ_KEY = -o #
PIC_KEY = -fPIC
WARNING_AS_ERROR_KEY = -Werror
WARNING_KEY = -w1
ifneq (,$(shell icc -dumpversion | egrep "1[2-9]\."))
SDL_FLAGS = -fstack-protector -Wformat -Wformat-security
endif

DYLIB_KEY = -dynamiclib
EXPORT_KEY = -Wl,-exported_symbols_list,
LIBDL = -ldl

LIBS = -lpthread
LINK_FLAGS =
LIB_LINK_FLAGS = -dynamiclib -static-intel -install_name @rpath/$(BUILDING_LIBRARY)
C_FLAGS = $(CPLUS_FLAGS)

ifneq (,$(shell icc -dumpversion | egrep  "^1[6-9]\."))
OPENMP_FLAG = -qopenmp
else
OPENMP_FLAG = -openmp
endif

# ICC 12.0 and higher provide Intel(R) Cilk(TM) Plus
ifneq (,$(shell icc -dumpversion | egrep  "^1[2-9]\."))
    CILK_AVAILABLE = yes
endif

ifeq ($(cfg), release)
    SDL_FLAGS += -D_FORTIFY_SOURCE=2
    CPLUS_FLAGS = -g -O2 -fno-omit-frame-pointer
else
    CPLUS_FLAGS = -g -O0 -DTBB_USE_DEBUG
endif

ITT_NOTIFY = -DDO_ITT_NOTIFY
CPLUS_FLAGS += -DUSE_PTHREAD $(ITT_NOTIFY)

ifneq (,$(codecov))
    CPLUS_FLAGS += -prof-gen=srcpos
endif

# ICC 14.0 and higher support usage of libc++, clang standard library
MACOSX_DEPLOYMENT_TARGET ?= 10.11
ifneq (,$(shell icc -dumpversion | egrep  "^1[4-9]\."))
ifneq (,$(stdlib))
    CPLUS_FLAGS    += -stdlib=$(stdlib) -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
    LIB_LINK_FLAGS += -stdlib=$(stdlib) -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
endif
endif

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------

ASM = as
ifeq (intel64,$(arch))
    ASM_FLAGS += -arch x86_64
endif
ifeq (ia32,$(arch))
    CPLUS_FLAGS += -m32
    LINK_FLAGS += -m32
    LIB_LINK_FLAGS += -m32
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
