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

ifndef tbb_os

  # Windows sets environment variable OS; for other systems, ask uname
  ifeq ($(OS),)
    OS:=$(shell uname)
    ifeq ($(OS),)
      $(error "Cannot detect operating system")
    endif
    export tbb_os=$(OS)
  endif

  ifeq ($(OS), Windows_NT)
    export tbb_os=windows
  endif
  ifeq ($(OS), Linux)
    export tbb_os=linux
  endif
  ifeq ($(OS), Darwin)
    export tbb_os=macos
  endif

endif # !tbb_os

ifeq (1,$(tbb_cpf))
  export CPF_SUFFIX ?=_preview
endif

ifeq (0,$(exceptions))
# Inverse the value, for simplicity of use
  export no_exceptions=1
endif

ifdef cpp0x
    $(warning "Warning: deprecated cpp0x=$(cpp0x) is used, stdver must be used instead. Building in stdver=c++0x mode.")
    export stdver?=c++0x
    override cpp0x=
endif

ifneq (,$(stdver))
    ifeq (,$(findstring ++, $(stdver)))
        $(warning "Warning: unexpected stdver=$(stdver) is used.")
    endif
    CXX_STD_FLAGS=-std=$(stdver)
endif

# The requested option is added unconditionally.
# If it is not supported, a compiler warning or error is expected.
# Note that CXX_STD_FLAGS can be changed in <os>.<compiler>.inc.
CXX_ONLY_FLAGS+=$(CXX_STD_FLAGS)

ifeq (,$(wildcard $(tbb_root)/build/$(tbb_os).inc))
  $(error "$(tbb_os)" is not supported. Add build/$(tbb_os).inc file with os-specific settings )
endif

# detect arch and runtime versions, provide common host-specific definitions
include $(tbb_root)/build/$(tbb_os).inc

ifeq ($(arch),)
 $(error Architecture not detected)
endif
ifeq ($(runtime),)
 $(error Runtime version not detected)
endif

# process target-dependent compilation and testing configurations
ifdef target
 # optionally process target-dependent options for compilation and testing
 ifneq (,$(wildcard $(tbb_root)/build/$(target).inc))
  include $(tbb_root)/build/$(target).inc
 endif

 # optionally process host-dependent environment for target-dependent compilation and testing
 ifneq (,$(wildcard $(tbb_root)/build/$(target).$(tbb_os).inc))
  include $(tbb_root)/build/$(target).$(tbb_os).inc
 endif

 # insure at least one target-dependent configuration file was found for compilation and testing
 ifeq (,$(wildcard $(tbb_root)/build/$(target).inc)$(wildcard $(tbb_root)/build/$(target).$(tbb_os).inc))
  $(error "$(target)" is not supported. Add build/$(target).inc or build/$(target).$(tbb_os).inc file)
 endif
endif  #target

# Support for running debug tests to release library and vice versa
flip_cfg=$(subst _flipcfg,_release,$(subst _release,_debug,$(subst _debug,_flipcfg,$(1))))
cross_cfg = $(if $(crosstest),$(call flip_cfg,$(1)),$(1))
# Setting default configuration to release
cfg?=release

compiler_name=$(notdir $(compiler))
ifdef BUILDING_PHASE
 ifndef target
  target:=$(tbb_os)
 endif
 # process host/target compiler-dependent build configuration
 ifeq (,$(wildcard $(tbb_root)/build/$(target).$(compiler_name).inc))
  $(error "$(compiler_name)" is not supported on $(target). Add build/$(target).$(compiler_name).inc file with compiler-specific settings. )
 endif
 include $(tbb_root)/build/$(target).$(compiler_name).inc
endif

ifneq ($(BUILDING_PHASE),1)
 # definitions for top-level Makefiles
 origin_build_dir:=$(origin tbb_build_dir)
 tbb_build_dir?=$(tbb_root)$(SLASH)build
 export tbb_build_prefix?=$(tbb_os)_$(arch)_$(compiler_name)_$(runtime)$(CPF_SUFFIX)
 work_dir=$(tbb_build_dir)$(SLASH)$(tbb_build_prefix)
endif  # BUILDING_PHASE != 1

ifdef offload
  extra_inc=$(offload).offload.inc
endif
ifdef extra_inc
 ifneq (,$(wildcard $(tbb_root)/build/$(extra_inc)))
  include $(tbb_root)/build/$(extra_inc)
 else
  $(error specified build file: "build/$(extra_inc)" is not found. )
 endif
endif

ifndef BUILDING_PHASE
  work_dir:=$(work_dir)
  # assign new value for tbb_root if path is not absolute (the filter keeps only /* paths)
  ifeq ($(filter /% $(SLASH)%, $(subst :, ,$(tbb_root)) ),)
   full_tbb_root:=$(CURDIR)/$(tbb_root)
   ifeq ($(origin_build_dir),undefined)
   #relative path are needed here as a workaround to support whitespaces in path
    override tbb_root:=../..
   else
    override tbb_root:=$(full_tbb_root)
   endif
  export tbb_root
  endif
 endif # !BUILDING_PHASE

.DELETE_ON_ERROR:    # Make will delete target if error occurred when building it.

# MAKEOVERRIDES contains the command line variable definitions. Reseting it to
# empty allows propogating all exported overridden variables to nested makes.
# NOTEs:
#   1. All variable set in command line are propagated to nested makes.
#   2. All variables declared with the "export" keyword are propagated to
#   nested makes.
#   3. "override" allows changing variables set in command line. But it doesn't
#   propagate new values to nested makes. For propagation, the "export" keyword
#   should be used.
#   4. gmake v3.80 doesn't support exporting of target-specific variables using
#   the "export" keyword
MAKEOVERRIDES =
