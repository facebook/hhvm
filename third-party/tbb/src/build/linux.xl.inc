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

####### Detections and Commands ###############################################

COMPILE_ONLY = -c
PREPROC_ONLY = -E -qsourcetype=c
INCLUDE_KEY = -I
DEFINE_KEY = -D
OUTPUT_KEY = -o #
OUTPUTOBJ_KEY = -o #
PIC_KEY = -qpic
WARNING_AS_ERROR_KEY = -qhalt=w
WARNING_KEY =
TEST_WARNING_KEY =

WARNING_SUPPRESS =
DYLIB_KEY = -qmkshrobj
EXPORT_KEY = -Wl,--version-script,
LIBDL = -ldl

CPLUS = xlc++_r
CONLY = xlc_r
LIB_LINK_FLAGS = $(DYLIB_KEY) -Wl,-soname=$(BUILDING_LIBRARY)
LIBS = -lpthread -lrt
C_FLAGS = $(CPLUS_FLAGS)

ifeq ($(cfg), release)
        CPLUS_FLAGS = $(ITT_NOTIFY) -O2 -DUSE_PTHREAD
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = -DTBB_USE_DEBUG $(ITT_NOTIFY) -g -O0 -DUSE_PTHREAD
endif

# Adding directly to CPLUS_FLAGS instead of to WARNING_SUPPRESS because otherwise it would not be used in several tests (why not?).
# Suppress warnings like:
# - "1500-029: (W) WARNING: subprogram [...] could not be inlined into [...]."
# - "1501-201: (W) Maximum number of common component diagnostics, 10  has been exceeded."
#     see http://www-01.ibm.com/support/docview.wss?uid=swg1LI72843
#     it seems that the internal compiler error that would ensue has now been avoided, making the condition harmless
# - "1540-0198 (W) The omitted keyword "private" is assumed for base class "no_copy"."
# - "1540-0822 (W) The name "__FUNCTION__" must not be defined as a macro."
CPLUS_FLAGS += -qsuppress=1500-029:1501-201:1540-0198:1540-0822

ASM=
ASM_FLAGS=

TBB_ASM.OBJ=

ifeq (intel64,$(arch))
    ITT_NOTIFY = -DDO_ITT_NOTIFY
    CPLUS_FLAGS += -q64
    LIB_LINK_FLAGS += -q64
endif

# TODO: equivalent for -march=pentium4 in CPLUS_FLAGS
ifeq (ia32,$(arch))
    ITT_NOTIFY = -DDO_ITT_NOTIFY
    CPLUS_FLAGS += -q32 -qarch=pentium4
    LIB_LINK_FLAGS += -q32
endif

ifeq (ppc64,$(arch))
    CPLUS_FLAGS += -q64
    LIB_LINK_FLAGS += -q64
endif

ifeq (ppc32,$(arch))
    CPLUS_FLAGS += -q32
    LIB_LINK_FLAGS += -q32
endif

ifeq (bg,$(arch))
    CPLUS = bgxlC_r
    CONLY = bgxlc_r
endif

#------------------------------------------------------------------------------
# Setting tbbmalloc data.
#------------------------------------------------------------------------------

# Suppress innumerable warnings like "1540-1088 (W) The exception specification is being ignored."
# Suppress             warnings like "1540-1090 (I) The destructor of "lock" might not be called."
# TODO: aren't these warnings an indication that -qnoeh might not be appropriate?
M_CPLUS_FLAGS = $(CPLUS_FLAGS) -qnortti -qnoeh -qsuppress=1540-1088:1540-1090

#------------------------------------------------------------------------------
# End of setting tbbmalloc data.
#------------------------------------------------------------------------------
