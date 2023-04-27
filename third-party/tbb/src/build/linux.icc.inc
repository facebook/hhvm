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
WARNING_KEY = -w1
DYLIB_KEY = -shared
EXPORT_KEY = -Wl,--version-script,
NOINTRINSIC_KEY = -fno-builtin
LIBDL = -ldl
ifneq (,$(shell icc -dumpversion | egrep "1[2-9]\."))
SDL_FLAGS = -fstack-protector -Wformat -Wformat-security
endif

CPLUS = icpc
CONLY = icc

ITT_NOTIFY = -DDO_ITT_NOTIFY
ifeq (release,$(cfg))
SDL_FLAGS += -D_FORTIFY_SOURCE=2
CPLUS_FLAGS = $(ITT_NOTIFY) -O2 -g -DUSE_PTHREAD
else
CPLUS_FLAGS = $(ITT_NOTIFY) -O0 -g -DUSE_PTHREAD -DTBB_USE_DEBUG
endif

LIB_LINK_FLAGS = -shared -static-intel -Wl,-soname=$(BUILDING_LIBRARY) -z relro -z now
LIBS += -lpthread -lrt
LINK_FLAGS = -rdynamic
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

TBB_ASM.OBJ=
MALLOC_ASM.OBJ=

ifeq (ia32,$(arch))
    CPLUS_FLAGS += -m32 -falign-stack=maintain-16-byte
    LIB_LINK_FLAGS += -m32
endif

ifeq (ia64,$(arch))
    ITT_NOTIFY =
# Position-independent code (PIC) is a must on IA-64 architecture, even for regular (not shared) executables
# strict-ansi does not work with <signal.h> on RHEL 4 AS
    CPLUS_FLAGS += $(PIC_KEY) $(if $(findstring cc3.,$(runtime)),-ansi,-strict-ansi)
else
# For ICC 16 and older, in std=c++14 mode -strict-ansi does not work with GNU C++ library headers
# egrep returns 0 or 1, compare it in concatenation
    CPLUS_FLAGS += $(if $(findstring c++14_1,$(stdver)_$(shell icc -dumpversion| egrep -c "^1[1-6]\.")),-ansi,-strict-ansi)
endif

ifneq (,$(codecov))
# no tool support for code coverage, need profile data generation
    ITT_NOTIFY = -prof-gen=srcpos
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

