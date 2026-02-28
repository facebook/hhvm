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
# Setting default configuration to release.
#------------------------------------------------------------------------------
cfg ?= release
#------------------------------------------------------------------------------
# End of setting default configuration to release.
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Setting compiler flags.
#------------------------------------------------------------------------------
CPLUS = icl /nologo $(VCCOMPAT_FLAG)
LINK_FLAGS = /link /nologo
LIB_LINK_FLAGS= /link /nologo /DLL /MAP /DEBUG /fixed:no /INCREMENTAL:NO /DYNAMICBASE /NXCOMPAT

ifeq ($(arch), ia32)
    LIB_LINK_FLAGS += /SAFESEH
endif

ifneq (,$(stdver))
    CXX_STD_FLAGS = /Qstd=$(stdver)
endif

# ICC 12.0 and higher provide Intel(R) Cilk(TM) Plus
ifeq (ok,$(call detect_js,/minversion icl 12))
    CILK_AVAILABLE = yes
endif

# ICC 17.0.4 and higher provide support for VS2017
ifeq (ok,$(call detect_js,/minversion icl 17 4))
    VS2017_SUPPORT = yes
endif

ifeq ($(runtime), vc_mt)
    MS_CRT_KEY = /MT$(if $(findstring debug,$(cfg)),d)
else
    MS_CRT_KEY = /MD$(if $(findstring debug,$(cfg)),d)
endif
EH_FLAGS = $(if $(no_exceptions),/EHs-,/EHsc /GR)

ifeq ($(cfg), release)
    CPLUS_FLAGS = $(MS_CRT_KEY) /O2 /Zi $(EH_FLAGS) /Zc:forScope /Zc:wchar_t /D__TBB_LIB_NAME=$(TBB.LIB)
    ASM_FLAGS =
endif
ifeq ($(cfg), debug)
    CPLUS_FLAGS = $(MS_CRT_KEY) /Od /Ob0 /Zi $(EH_FLAGS) /Zc:forScope /Zc:wchar_t /DTBB_USE_DEBUG /D__TBB_LIB_NAME=$(TBB.LIB)
    ASM_FLAGS = /DUSE_FRAME_POINTER
endif
CPLUS_FLAGS += /GS

COMPILE_ONLY = /c /QMMD
# PREPROC_ONLY should really use /TP which applies to all files in the command line.
# But with /TP, ICL does not preprocess *.def files.
PREPROC_ONLY = /EP /Tp
INCLUDE_KEY = /I
DEFINE_KEY = /D
OUTPUT_KEY = /Fe
OUTPUTOBJ_KEY = /Fo
WARNING_AS_ERROR_KEY = /WX
WARNING_KEY = /W3
WARNING_SUPPRESS = $(if $(no_exceptions),/wd583)
DYLIB_KEY = /DLL
EXPORT_KEY = /DEF:
NODEFAULTLIB_KEY = /Zl
NOINTRINSIC_KEY = /Oi-
BIGOBJ_KEY = /bigobj
INCLUDE_TEST_HEADERS = /FI$(tbb_root)/src/test/harness_preload.h


ifneq (,$(codecov))
    CPLUS_FLAGS += /Qprof-genx
else
    CPLUS_FLAGS += /DDO_ITT_NOTIFY
endif

OPENMP_FLAG = /Qopenmp
CPLUS_FLAGS += /DUSE_WINTHREAD /D_CRT_SECURE_NO_DEPRECATE \
               /D_WIN32_WINNT=$(_WIN32_WINNT)

ifeq ($(runtime),vc8)
    CPLUS_FLAGS += /D_USE_RTM_VERSION
endif


C_FLAGS = $(subst $(EH_FLAGS),,$(CPLUS_FLAGS))

VCVERSION:=$(runtime)
VCCOMPAT_FLAG ?= $(if $(findstring vc7.1, $(VCVERSION)),/Qvc7.1)
ifeq ($(VCCOMPAT_FLAG),)
        VCCOMPAT_FLAG := $(if $(findstring vc8, $(VCVERSION)),/Qvc8)
endif
ifeq ($(VCCOMPAT_FLAG),)
        VCCOMPAT_FLAG := $(if $(findstring vc_mt, $(VCVERSION)),/Qvc10)
endif
ifeq ($(VCCOMPAT_FLAG),)
        VCCOMPAT_FLAG := $(if $(findstring vc9, $(VCVERSION)),/Qvc9)
endif
ifeq ($(VCCOMPAT_FLAG),)
        VCCOMPAT_FLAG := $(if $(findstring vc10, $(VCVERSION)),/Qvc10)
endif
ifeq ($(VCCOMPAT_FLAG),)
        VCCOMPAT_FLAG := $(if $(findstring vc11, $(VCVERSION)),/Qvc11)
endif
ifeq ($(VCCOMPAT_FLAG),)
        VCCOMPAT_FLAG := $(if $(findstring vc12, $(VCVERSION)),/Qvc12)
endif
ifeq ($(VCCOMPAT_FLAG),)
    VCCOMPAT_FLAG := $(if $(findstring vc14, $(VCVERSION)),/Qvc14)
    ifeq ($(VS2017_SUPPORT),yes)
        ifneq (,$(findstring vc14.1, $(VCVERSION)))
            VCCOMPAT_FLAG := /Qvc14.1
        endif
    endif
endif
ifeq ($(VCCOMPAT_FLAG),)
        $(error VC version not detected correctly: $(VCVERSION) )
endif
export VCCOMPAT_FLAG

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
    ASM=ml /nologo
    ASM_FLAGS += /c /coff /Zi /safeseh
    TBB_ASM.OBJ = atomic_support.obj lock_byte.obj itsx.obj
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
