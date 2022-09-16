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

#------------------------------------------------------------------------------
# Defines settings for building the TBB run-time as a static library.
# Use these only on platforms where dynamic linking is impractical.
#
# IF YOU USE TBB AS A STATIC LIBRARY, YOU MUST GUARANTEE THAT ONLY ONE COPY OF 
# THE TBB RUN-TIME IS LINKED INTO AN APPLICATION!  LINKING IN MULTIPLE COPIES
# OF THE TBB RUN-TIME, DIRECTLY OR INDIRECTLY, MAY CAUSE PROGRAM FAILURE!
#------------------------------------------------------------------------------

# Note that ITT_NOTIFY allows to selectively remove the definition of
# DO_ITT_NOTIFY without sabotaging deferred expansion of CPLUS_FLAGS.
# TODO: currently only in linux.{gcc,xl}.inc

# Note that -pthread with xl gives "1501-210 (W) command option t contains an incorrect subargument";
# multithreading is instead achieved by using the _r affix in the compiler name.
# TODO: is -lpthread still relevant/needed with XL and _r affix?

# Note that usage of dynamic (shared) libraries is disabled
# (via -D__TBB_DYNAMIC_LOAD_ENABLED=0 and LIBDL emptied) primarily for performance.

# OS specific settings =>
    LIB_LINK_CMD = ar rcs
    LIB_LINK_FLAGS =
    LIB_LINK_LIBS =
    LIB_OUTPUT_KEY =
    DYLIB_KEY =
    ifeq ($(tbb_os),linux)
        ifeq ($(compiler),clang)
            LIBS = -pthread -lrt
        endif
        ifeq ($(compiler),gcc)
            LIBS = -pthread -lrt
        endif
        ifeq ($(compiler),xl)
            LIBS = -lpthread -lrt
        endif
        LINK_FLAGS =
    endif
    override CXXFLAGS += -D__TBB_DYNAMIC_LOAD_ENABLED=0 -D__TBB_SOURCE_DIRECTLY_INCLUDED=1
    ITT_NOTIFY =
    DLL = a
    LIBEXT = a
    LIBPREF = lib
    LIBDL =
# <= OS specific settings

TBB.DLL = $(LIBPREF)tbb$(DEBUG_SUFFIX).$(LIBEXT)
LINK_TBB.LIB = $(TBB.DLL)
TBB.LST =
TBB.DEF =
TBB_NO_VERSION.DLL =

MALLOC.DLL = $(LIBPREF)tbbmalloc$(DEBUG_SUFFIX).$(LIBEXT)
LINK_MALLOC.LIB = $(MALLOC.DLL)
MALLOC.DEF =
MALLOC_NO_VERSION.DLL =
MALLOCPROXY.DLL =
MALLOCPROXY.DEF =
