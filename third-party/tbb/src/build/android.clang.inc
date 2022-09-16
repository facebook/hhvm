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
SDL_FLAGS = -fPIE -fPIC -fstack-protector -Wformat -Wformat-security
TEST_WARNING_KEY = -Wshadow -Wcast-qual -Woverloaded-virtual -Wnon-virtual-dtor -Wextra

WARNING_SUPPRESS = -Wno-parentheses -Wno-non-virtual-dtor
DYLIB_KEY = -shared
EXPORT_KEY = -Wl,--version-script,
LIBDL = -ldl

CPLUS = $(TARGET_CXX)
CONLY = $(TARGET_CC)

# -soname is necessary for proper linkage to TBB prebuilt libraries when building application with Android SDK
LIB_LINK_FLAGS = $(DYLIB_KEY) -Wl,-soname=$(BUILDING_LIBRARY) -z relro -z now

# pie is necessary for test executables to work and might be removed if newer NDK will add it implicitly
PIE_FLAG = -pie
ifeq ($(APP_PIE), false)
    PIE_FLAG=
endif

LINK_FLAGS = -Wl,-rpath-link=. -rdynamic 
C_FLAGS = $(CPLUS_FLAGS)

ifeq ($(cfg), release)
    SDL_FLAGS += -D_FORTIFY_SOURCE=2
    CPLUS_FLAGS = -O2
endif
ifeq ($(cfg), debug)
    CPLUS_FLAGS = -g -O0 $(DEFINE_KEY)TBB_USE_DEBUG
endif

CPLUS_FLAGS += $(DEFINE_KEY)USE_PTHREAD $(DEFINE_KEY)_GLIBCXX_HAVE_FENV_H

ifneq (,$(findstring $(arch),ia32 intel64))
    CPLUS_FLAGS += $(DEFINE_KEY)DO_ITT_NOTIFY
endif

ifeq (0, $(dynamic_load))
     CPLUS_FLAGS += $(DEFINE_KEY)__TBB_DYNAMIC_LOAD_ENABLED=0
endif

# Paths to the NDK prebuilt tools and libraries
ifneq (,$(findstring $(ndk_version),r16 r16b))
    # Since Android* NDK r16 another sysroot and isystem paths have to be specified
    CPLUS_FLAGS += --sysroot=$(NDK_ROOT)/sysroot -isystem $(NDK_ROOT)/sysroot/usr/include/$(TRIPLE) 
    # Android* version flag required since r16
    CPLUS_FLAGS += -D__ANDROID_API__=$(API_LEVEL)
else
    CPLUS_FLAGS += --sysroot=$(SYSROOT)
endif

# Library sysroot flag
LIB_LINK_FLAGS += --sysroot=$(SYSROOT)
# Flag for test executables
LINK_FLAGS     += --sysroot=$(SYSROOT)

LIBS           = -L$(CPLUS_LIB_PATH) -lc++_shared
ifeq (,$(findstring $(ndk_version),$(foreach v, 7 8 9 10 11,r$(v) r$(v)b r$(v)c r$(v)d r$(v)e)))
    LIBS +=  -lc++abi
    ifeq (arm,$(arch))
        LIBS += -lunwind 
    endif
endif

ifeq (arm,$(arch))
    CPLUS_FLAGS += $(DEFINE_KEY)__TBB_64BIT_ATOMICS=0
endif

CPLUS_FLAGS += $(TARGET_CFLAGS)
LIB_LINK_FLAGS += $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -L$(CPLUS_LIB_PATH)

#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------
TBB_ASM.OBJ=
MALLOC_ASM.OBJ=

ASM = $(tbb_tool_prefix)as
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
