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

ifneq (mic,$(offload))
  $(error File mic.offload.inc should not be included directly. Use offload=mic instead.)
endif
ifneq (icc,$(compiler))
  $(error Only Intel(R) Compiler is supported for MIC offload compilation)
endif

# The same build prefix should be used in mic.linux.inc
mic_tbb_build_prefix=mic_icc$(CPF_SUFFIX)
MIC_OFFLOAD_NATIVE_PATH?=../$(mic_tbb_build_prefix)_$(cfg)

ifdef BUILDING_PHASE
  ifeq ($(BUILDING_PHASE),1)
    # Tests
    export MIC_OFFLOAD_NATIVE_PATH
    LINK_TBB_NATIVE.LIB=$(MIC_OFFLOAD_NATIVE_PATH)/$(TBB.LIB)
    LINK_TBB.LIB=-qoffload-option,mic,ld,"$(LINK_TBB_NATIVE.LIB)" $(TBB.LIB)
    LINK_MALLOC_NATIVE.LIB=$(MIC_OFFLOAD_NATIVE_PATH)/$(MALLOC.DLL)
    LINK_MALLOC.LIB=-qoffload-option,mic,ld,"$(LINK_MALLOC_NATIVE.LIB)" $(MALLOC.LIB)
    LINK_MALLOCPROXY_NATIVE.LIB=$(MIC_OFFLOAD_NATIVE_PATH)/$(MALLOCPROXY.DLL)
    LINK_MALLOCPROXY.LIB=-qoffload-option,mic,ld,"$(LINK_MALLOCPROXY_NATIVE.LIB)" $(MALLOCPROXY.LIB)

    # Export extensions for test_launcher
    export DLL
    export TEST_EXT=offload.exe
    OBJ=offload.o

    # Do not use -Werror because it is too strict for the early offload compiler.
    # Need to set anything because WARNING_AS_ERROR_KEY should not be empty.
    # Treat #2426 as a warning. Print errors only.
    tbb_strict=0
    WARNING_AS_ERROR_KEY = Warning as error
    WARNING_KEY = -diag-warning 2426 -w0

    CXX_MIC_STUFF = -qoffload-attribute-target=mic -D__TBB_MIC_OFFLOAD=1 -qoffload-option,mic,compiler,"-D__TBB_MIC_OFFLOAD=1 $(CXX_MIC_NATIVE_STUFF)"
    CXX_MIC_NATIVE_STUFF = -DHARNESS_INCOMPLETE_SOURCES=1 -D__TBB_MIC_NATIVE -DTBB_USE_EXCEPTIONS=0
    CPLUS_FLAGS += $(CXX_MIC_STUFF)

    # Some tests require that an executable exports its symbols.
    LINK_FLAGS += -qoffload-option,mic,ld,"--export-dynamic"

    # libcoi_device.so is needed for COIProcessProxyFlush used in Harness.
    LINK_FLAGS += -qoffload-option,mic,ld,"-lcoi_device"

    # DSO-linking semantics forces linking libpthread and librt to a test.
    LINK_FLAGS += -qoffload-option,mic,ld,"-lpthread -lrt"

    .PHONY: FORCE
    FORCE:

    $(MIC_OFFLOAD_NATIVE_PATH)/%_dll.$(DLL): FORCE
	@$(MAKE) --no-print-directory -C "$(MIC_OFFLOAD_NATIVE_PATH)" target=mic offload= -f$(tbb_root)/build/Makefile.$(TESTFILE) $*_dll.$(DLL)
    %_dll.$(DLL): $(MIC_OFFLOAD_NATIVE_PATH)/%_dll.$(DLL) FORCE
	@$(MAKE) --no-print-directory offload= -f$(tbb_root)/build/Makefile.$(TESTFILE) $*_dll.$(DLL)

    .PRECIOUS: $(MIC_OFFLOAD_NATIVE_PATH)/%_dll.$(DLL)

    %.$(TEST_EXT): LINK_FILES+=-qoffload-option,mic,ld,"$(addprefix $(MIC_OFFLOAD_NATIVE_PATH)/,$(TEST_LIBS))"

    TEST_LAUNCHER=sh $(tbb_root)/build/test_launcher.sh $(largs)

    ifdef MIC_LD_LIBRARY_PATH
      export MIC_LD_LIBRARY_PATH := $(MIC_OFFLOAD_NATIVE_PATH):$(MIC_LD_LIBRARY_PATH)
    else
      export MIC_LD_LIBRARY_PATH := $(MIC_OFFLOAD_NATIVE_PATH)
    endif
  else
    # Examples
    export UI = con
    export x64 = 64
  endif
else
  # Libraries
  LIB_TARGETS = tbb tbbmalloc tbbproxy rml
  addsuffixes = $(foreach suff,$(1),$(addsuffix $(suff),$(2)))

  .PHONY: $(call addsuffixes, _debug _release _debug_mic _release_mic,$(LIB_TARGETS))

  # The dependence on *_debug and *_release targets unifies the offload support
  # for top-level Makefile and src/Makefile
  $(LIB_TARGETS): %: %_release %_debug

  # "override offload=" suppresses the "offload" variable value for nested makes
  $(LIB_TARGETS) $(call addsuffixes, _debug _release,$(LIB_TARGETS)): override offload=
  # Apply overriding for library builds
  export offload
  export tbb_build_prefix
  # Add the dependency on target libraries
  $(call addsuffixes, _debug _release,$(LIB_TARGETS)): %: %_mic

  # tbb_build_prefix should be overridden since we want to restart make in "clear" environment
  $(call addsuffixes, _debug_mic _release_mic,$(LIB_TARGETS)): override tbb_build_prefix=
  $(call addsuffixes, _debug_mic _release_mic,$(LIB_TARGETS)): %_mic:
	@$(MAKE) --no-print-directory -C "$(full_tbb_root)/src" $* target=mic tbb_root=..

  mic_clean: override tbb_build_prefix=
  mic_clean:
	@$(MAKE) --no-print-directory -C "$(full_tbb_root)/src" clean offload= target=mic tbb_root=..
  clean: mic_clean
endif
