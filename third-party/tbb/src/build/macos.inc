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

ifeq (icc,$(compiler))
  export COMPILER_VERSION := ICC: $(shell icc -V </dev/null 2>&1 | grep 'Version')
  ifneq (,$(findstring running on IA-32, $(COMPILER_VERSION)))
    export arch:=ia32
  else
    ifneq (,$(findstring running on Intel(R) 64, $(COMPILER_VERSION)))
      export arch:=intel64
    endif
  endif
  ifeq (,$(arch))
    $(warning "Unknown Intel compiler")
  endif
endif

ifndef arch
 ifeq ($(shell /usr/sbin/sysctl -n hw.machine),Power Macintosh)
   ifeq ($(shell /usr/sbin/sysctl -n hw.optional.64bitops),1)
     export arch:=ppc64
   else
     export arch:=ppc32
   endif
 else
   ifeq ($(shell /usr/sbin/sysctl -n hw.optional.x86_64 2>/dev/null),1)
     export arch:=intel64
   else
     export arch:=ia32
   endif
 endif
endif

ifndef runtime
  clang_version:=$(shell clang -v 2>&1 >/dev/null | grep version | sed -e "s/.*version \(.*[0-9]\) .*/\1/")
  ifndef os_version
    os_version:=$(shell /usr/bin/sw_vers -productVersion)
  endif
  export runtime:=cc$(clang_version)_os$(os_version)
endif

native_compiler := clang
export compiler ?= clang
debugger ?= lldb

export stdlib ?= libc++

CMD=$(SHELL) -c
CWD=$(shell pwd)
RM?=rm -f
RD?=rmdir
MD?=mkdir -p
NUL= /dev/null
SLASH=/
MAKE_VERSIONS=sh $(tbb_root)/build/version_info_macos.sh $(VERSION_FLAGS) >version_string.ver
MAKE_TBBVARS=sh $(tbb_root)/build/generate_tbbvars.sh DY

ifdef DYLD_LIBRARY_PATH
        export DYLD_LIBRARY_PATH := .:$(DYLD_LIBRARY_PATH)
else
        export DYLD_LIBRARY_PATH := .
endif

####### Build settings ########################################################

OBJ=o
DLL=dylib
MALLOC_DLL?=$(DLL)
LIBEXT=dylib

def_prefix = $(if $(findstring 64,$(arch)),mac64,mac32)

TBB.LST = $(tbb_root)/src/tbb/$(def_prefix)-tbb-export.lst
TBB.DEF = $(TBB.LST:.lst=.def)
TBB.DLL = libtbb$(CPF_SUFFIX)$(DEBUG_SUFFIX).$(DLL)
TBB.LIB = $(TBB.DLL)
LINK_TBB.LIB = $(TBB.LIB)

MALLOC.DEF = $(MALLOC_ROOT)/$(def_prefix)-tbbmalloc-export.def
MALLOC.DLL = libtbbmalloc$(DEBUG_SUFFIX).$(MALLOC_DLL)
MALLOC.LIB = $(MALLOC.DLL)
LINK_MALLOC.LIB = $(MALLOC.LIB)

MALLOCPROXY.DLL = libtbbmalloc_proxy$(DEBUG_SUFFIX).$(MALLOC_DLL)
MALLOCPROXY.LIB = $(MALLOCPROXY.DLL)
LINK_MALLOCPROXY.LIB = $(MALLOCPROXY.LIB)

TEST_LAUNCHER=sh $(tbb_root)/build/test_launcher.sh $(largs)

OPENCL.LIB = -framework OpenCL
