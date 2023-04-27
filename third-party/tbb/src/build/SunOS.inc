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

ifndef arch
        arch:=$(shell uname -p)
        ifeq ($(arch),i386)
            ifeq ($(shell isainfo -b),64)
                arch:=intel64
            else
                arch:=ia32
            endif
        endif
        export arch
# For non-IA systems running Sun OS, 'arch' will contain whatever is printed by uname -p.
# In particular, for SPARC architecture it will contain "sparc".
endif

ifndef runtime
        gcc_version:=$(shell gcc -dumpversion)
        os_version:=$(shell uname -r)
        os_kernel_version:=$(shell uname -r | sed -e 's/-.*$$//')
        export runtime:=cc$(gcc_version)_kernel$(os_kernel_version)
endif

ifeq ($(arch),sparc)
        native_compiler := gcc
        export compiler ?= gcc
else
        native_compiler := suncc
        export compiler ?= suncc
endif
# debugger ?= gdb

CMD=$(SHELL) -c
CWD=$(shell pwd)
RM?=rm -f
RD?=rmdir
MD?=mkdir -p
NUL= /dev/null
SLASH=/
MAKE_VERSIONS=bash $(tbb_root)/build/version_info_sunos.sh $(VERSION_FLAGS) >version_string.ver
MAKE_TBBVARS=bash $(tbb_root)/build/generate_tbbvars.sh

ifdef LD_LIBRARY_PATH
        export LD_LIBRARY_PATH := .:$(LD_LIBRARY_PATH)
else
        export LD_LIBRARY_PATH := .
endif

####### Build settings ########################################################

OBJ = o
DLL = so
LIBEXT=so

TBB.LST =
TBB.DEF =
TBB.DLL = libtbb$(CPF_SUFFIX)$(DEBUG_SUFFIX).$(DLL)
TBB.LIB = $(TBB.DLL)
LINK_TBB.LIB = $(TBB.LIB)

MALLOC.DLL = libtbbmalloc$(DEBUG_SUFFIX).$(DLL)
MALLOC.LIB = $(MALLOC.DLL)
LINK_MALLOC.LIB = $(MALLOC.LIB)

MALLOCPROXY.DLL = libtbbmalloc_proxy$(DEBUG_SUFFIX).$(DLL)

TEST_LAUNCHER=sh $(tbb_root)/build/test_launcher.sh $(largs)
