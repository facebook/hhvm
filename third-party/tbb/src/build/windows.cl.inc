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
# Define compiler-specific variables.
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Setting compiler flags.
#------------------------------------------------------------------------------
CPLUS = cl /nologo
LINK_FLAGS = /link /nologo
LIB_LINK_FLAGS=/link /nologo /DLL /MAP /DEBUG /fixed:no /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT

ifneq (,$(stdver))
    CXX_STD_FLAGS = /std:$(stdver)
endif

ifeq ($(arch), ia32)
    LIB_LINK_FLAGS += /SAFESEH
endif

ifeq ($(runtime), vc_mt)
    MS_CRT_KEY = /MT$(if $(findstring debug,$(cfg)),d)
else
    MS_CRT_KEY = /MD$(if $(findstring debug,$(cfg)),d)
endif
EH_FLAGS = $(if $(no_exceptions),/EHs-,/EHsc /GR)

# UWD binaries have to use static CRT linkage
ifeq ($(target_app), uwd)
    MS_CRT_KEY = /MT$(if $(findstring debug,$(cfg)),d)
endif

ifeq ($(cfg), release)
        CPLUS_FLAGS = $(MS_CRT_KEY) /O2 /Zi $(EH_FLAGS) /Zc:forScope /Zc:wchar_t /D__TBB_LIB_NAME=$(TBB.LIB)
        ASM_FLAGS =
endif
ifeq ($(cfg), debug)
        CPLUS_FLAGS = $(MS_CRT_KEY) /Od /Ob0 /Zi $(EH_FLAGS) /Zc:forScope /Zc:wchar_t /DTBB_USE_DEBUG /D__TBB_LIB_NAME=$(TBB.LIB)
        ASM_FLAGS = /DUSE_FRAME_POINTER
endif

ZW_KEY = /ZW:nostdlib

# These flags are general for Windows* universal applications
ifneq (,$(target_app))
    CPLUS_FLAGS += $(ZW_KEY) /D "_UNICODE" /D "UNICODE" /D "WINAPI_FAMILY=WINAPI_FAMILY_APP"
endif

ifeq ($(target_app), win8ui)
        _WIN32_WINNT = 0x0602
else ifneq (,$(filter $(target_app),uwp uwd))
        _WIN32_WINNT = 0x0A00
        LIB_LINK_FLAGS += /NODEFAULTLIB:kernel32.lib OneCore.lib
else
        CPLUS_FLAGS += /DDO_ITT_NOTIFY
endif
ifeq ($(target_mode), store)
#       it is necessary to source vcvars with 'store' argument in production
        LIB_LINK_FLAGS += /APPCONTAINER
endif

CPLUS_FLAGS += /GS

COMPILE_ONLY = /c
PREPROC_ONLY = /TP /EP
INCLUDE_KEY = /I
DEFINE_KEY = /D
OUTPUT_KEY = /Fe
OUTPUTOBJ_KEY = /Fo
WARNING_AS_ERROR_KEY = /WX
WARNING_SUPPRESS = $(if $(no_exceptions),/wd4530 /wd4577)
BIGOBJ_KEY = /bigobj

ifeq ($(runtime),vc7.1)
        WARNING_KEY = /W3
else
        WARNING_KEY = /W4
        OPENMP_FLAG = /openmp
endif

DYLIB_KEY = /DLL
EXPORT_KEY = /DEF:
NODEFAULTLIB_KEY = /Zl
NOINTRINSIC_KEY = /Oi-

INCLUDE_TEST_HEADERS = /FI$(tbb_root)/src/test/harness_preload.h

ifeq ($(runtime),vc8)
        WARNING_KEY += /Wp64
        CPLUS_FLAGS += /D_USE_RTM_VERSION
endif

# Since VS2012, VC++ provides /volatile option to control semantics of volatile variables.
# We want to use strict ISO semantics in the library and tests
ifeq (ok,$(call detect_js,/minversion cl 17))
        CPLUS_FLAGS += /volatile:iso
endif

# Since VS2013, VC++ uses the same .pdb file for different sources so we need
# to add /FS (Force Synchronous PDB Writes)
ifeq (ok,$(call detect_js,/minversion cl 18))
        CPLUS_FLAGS += /FS
endif

CPLUS_FLAGS += /DUSE_WINTHREAD /D_CRT_SECURE_NO_DEPRECATE \
        /D_WIN32_WINNT=$(_WIN32_WINNT)
C_FLAGS = $(subst $(ZW_KEY),,$(subst $(EH_FLAGS),,$(CPLUS_FLAGS)))

#------------------------------------------------------------------------------
# End of setting compiler flags.
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Setting assembler data.
#------------------------------------------------------------------------------
ASSEMBLY_SOURCE=$(arch)-masm
ifeq (intel64,$(arch))
    ASM=ml64 /nologo
    ASM_FLAGS += /DEM64T=1 /c /Zi
    TBB_ASM.OBJ = atomic_support.obj intel64_misc.obj itsx.obj
    MALLOC_ASM.OBJ = atomic_support.obj
else
ifeq (armv7,$(arch))
    ASM=
    TBB_ASM.OBJ= 
else
    ASM=ml /nologo
    ASM_FLAGS += /c /coff /Zi /safeseh
    TBB_ASM.OBJ = atomic_support.obj lock_byte.obj itsx.obj
endif
endif
#------------------------------------------------------------------------------
# End of setting assembler data.
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Setting tbbmalloc data.
#------------------------------------------------------------------------------
M_CPLUS_FLAGS = $(CPLUS_FLAGS)
#------------------------------------------------------------------------------
# End of setting tbbmalloc data.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# End of define compiler-specific variables.
#------------------------------------------------------------------------------
