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
    else
      ifneq (,$(findstring IA-64, $(COMPILER_VERSION)))
        export arch:=ia64
      endif
    endif
  endif
  ifeq (,$(arch))
    $(warning "Unknown Intel compiler")
  endif
endif

ifndef arch
        uname_m:=$(shell uname -m)
        ifeq ($(uname_m),i686)
                export arch:=ia32
        endif
        ifeq ($(uname_m),ia64)
                export arch:=ia64
        endif
        ifeq ($(uname_m),x86_64)
                export arch:=intel64
        endif
        ifeq ($(uname_m),sparc64)
                export arch:=sparc
        endif
        ifeq ($(uname_m),armv7l)
                export arch:=armv7
        endif
        ifndef arch
                export arch:=$(uname_m)
        endif
endif

ifndef runtime
        export gcc_version:=$(shell gcc -dumpversion)
        os_version:=$(shell uname -r)
        os_kernel_version:=$(shell uname -r | sed -e 's/-.*$$//')
        export os_glibc_version_full:=$(shell getconf GNU_LIBC_VERSION | grep glibc | sed -e 's/^glibc //')
        os_glibc_version:=$(shell echo "$(os_glibc_version_full)" | sed -e '2,$$d' -e 's/-.*$$//')
        export runtime:=cc$(gcc_version)_libc$(os_glibc_version)_kernel$(os_kernel_version)
endif

native_compiler := gcc
export compiler ?= gcc
debugger ?= gdb

CMD=sh -c
CWD=$(shell pwd)
CP=cp
RM?=rm -f
RD?=rmdir
MD?=mkdir -p
NUL= /dev/null
SLASH=/
MAKE_VERSIONS=sh $(tbb_root)/build/version_info_linux.sh $(VERSION_FLAGS) >version_string.ver
MAKE_TBBVARS=sh $(tbb_root)/build/generate_tbbvars.sh

ifdef LD_LIBRARY_PATH
        export LD_LIBRARY_PATH := .:$(LD_LIBRARY_PATH)
else
        export LD_LIBRARY_PATH := .
endif

####### Build settings ########################################################

OBJ = o
DLL = so
MALLOC_DLL?=$(DLL)
LIBEXT = so
SONAME_SUFFIX =$(shell grep TBB_COMPATIBLE_INTERFACE_VERSION $(tbb_root)/include/tbb/tbb_stddef.h | egrep -o [0-9.]+)

ifeq ($(arch),ia64)
        def_prefix = lin64ipf
endif
ifneq (,$(findstring $(arch),sparc s390x))
        def_prefix = lin64
endif
ifeq ($(arch),armv7)
        def_prefix = lin32
endif
ifeq (,$(def_prefix))
    ifeq (64,$(findstring 64,$(arch)))
            def_prefix = lin64
    else
            def_prefix = lin32
    endif
endif
TBB.LST = $(tbb_root)/src/tbb/$(def_prefix)-tbb-export.lst
TBB.DEF = $(TBB.LST:.lst=.def)

TBB.DLL = $(TBB_NO_VERSION.DLL).$(SONAME_SUFFIX)
TBB.LIB = $(TBB.DLL)
TBB_NO_VERSION.DLL=libtbb$(CPF_SUFFIX)$(DEBUG_SUFFIX).$(DLL)
LINK_TBB.LIB = $(TBB_NO_VERSION.DLL)

MALLOC_NO_VERSION.DLL = libtbbmalloc$(DEBUG_SUFFIX).$(MALLOC_DLL)
MALLOC.DEF = $(MALLOC_ROOT)/$(def_prefix)-tbbmalloc-export.def
MALLOC.DLL = $(MALLOC_NO_VERSION.DLL).$(SONAME_SUFFIX)
MALLOC.LIB = $(MALLOC_NO_VERSION.DLL)
LINK_MALLOC.LIB = $(MALLOC_NO_VERSION.DLL)

MALLOCPROXY_NO_VERSION.DLL = libtbbmalloc_proxy$(DEBUG_SUFFIX).$(DLL)
MALLOCPROXY.DEF = $(MALLOC_ROOT)/$(def_prefix)-proxy-export.def
MALLOCPROXY.DLL = $(MALLOCPROXY_NO_VERSION.DLL).$(SONAME_SUFFIX)
MALLOCPROXY.LIB = $(MALLOCPROXY_NO_VERSION.DLL)
LINK_MALLOCPROXY.LIB = $(MALLOCPROXY.LIB)

RML_NO_VERSION.DLL = libirml$(DEBUG_SUFFIX).$(DLL)
RML.DEF = $(RML_SERVER_ROOT)/lin-rml-export.def
RML.DLL = $(RML_NO_VERSION.DLL).1
RML.LIB = $(RML_NO_VERSION.DLL)

TEST_LAUNCHER=sh $(tbb_root)/build/test_launcher.sh $(largs)

OPENCL.LIB = -lOpenCL
