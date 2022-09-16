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
DYLIB_KEY = -shared -Wl,-soname=$@
EXPORT_KEY = -Wl,--version-script,
NOINTRINSIC_KEY = -fno-builtin
LIBDL = -ldl
SDL_FLAGS = -fstack-protector -Wformat -Wformat-security
CPLUS = icpc
CONLY = icc

ifeq (release,$(cfg))
    SDL_FLAGS += -D_FORTIFY_SOURCE=2
    CPLUS_FLAGS = -O2 -g -DUSE_PTHREAD
else
    CPLUS_FLAGS = -O0 -g -DUSE_PTHREAD -DTBB_USE_DEBUG
endif

ifneq (,$(codecov))
    CPLUS_FLAGS += -prof-gen=srcpos
endif

ifneq (,$(shell icc -dumpversion | egrep  "^1[6-9]\."))
OPENMP_FLAG = -qopenmp
else
OPENMP_FLAG = -openmp
endif

LIB_LINK_FLAGS = -shared -static-intel -Wl,-soname=$(BUILDING_LIBRARY) -z relro -z now
LIBS += -lpthread -lrt
C_FLAGS = $(CPLUS_FLAGS)
CILK_AVAILABLE = yes

TBB_ASM.OBJ=
MALLOC_ASM.OBJ=

CPLUS_FLAGS += -DHARNESS_INCOMPLETE_SOURCES=1 -D__TBB_MIC_NATIVE -DTBB_USE_EXCEPTIONS=0 -qopt-streaming-stores never
CPLUS += -mmic
CONLY += -mmic
LINK_FLAGS = -Wl,-rpath-link=. -rdynamic
# Tell the icc to not link against libcilk*. Otherwise icc tries to link and emits a warning message.
LIB_LINK_FLAGS += -no-intel-extensions

#------------------------------------------------------------------------------
# Setting tbbmalloc data.
#------------------------------------------------------------------------------

M_CPLUS_FLAGS = $(CPLUS_FLAGS) -fno-rtti -fno-exceptions

#------------------------------------------------------------------------------
# End of setting tbbmalloc data.
#------------------------------------------------------------------------------



