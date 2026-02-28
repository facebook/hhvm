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


ifeq ($(tbb_strict),1)
  ifeq ($(WARNING_AS_ERROR_KEY),)
    $(error WARNING_AS_ERROR_KEY is empty)
  endif
  # Do not remove line below!
  WARNING_KEY += $(WARNING_AS_ERROR_KEY)
endif

ifneq (,$(findstring s,$(MAKEFLAGS)))
  override largs+=-q
endif
ifneq (,$(repeat))
  override largs+=-r $(repeat)
endif
ifneq (,$(largs)$(run_prefix))
  override run_cmd:=$(run_cmd) $(TEST_LAUNCHER)
  TEST_LAUNCHER=
  ifeq (,$(strip $(run_cmd)))
    $(warning Test launcher is not defined for the platform, ignoring launcher arguments)
  endif
endif

ifndef TEST_EXT
    TEST_EXT = exe
endif

INCLUDES += $(INCLUDE_KEY)$(tbb_root)/src $(INCLUDE_KEY)$(tbb_root)/src/rml/include $(INCLUDE_KEY)$(tbb_root)/include

CPLUS_FLAGS += $(WARNING_KEY) $(CXXFLAGS)
ifeq (1,$(tbb_cpf))
CPLUS_FLAGS += $(DEFINE_KEY)__TBB_CPF_BUILD=1
endif
ifeq (0,$(exceptions))
CPLUS_FLAGS += $(DEFINE_KEY)TBB_USE_EXCEPTIONS=0
endif
LINK_FLAGS += $(LDFLAGS)
LIB_LINK_FLAGS += $(LDFLAGS)

LIB_LINK_CMD ?= $(CPLUS) $(PIC_KEY)
ifeq ($(origin LIB_OUTPUT_KEY), undefined)
    LIB_OUTPUT_KEY = $(OUTPUT_KEY)
endif
ifeq ($(origin LIB_LINK_LIBS), undefined)
    LIB_LINK_LIBS = $(LIBDL) $(LIBS)
endif

# Define C & C++ compilers according to platform defaults or CXX & CC environment variables
ifneq (,$(findstring environment, $(origin CXX)))
CPLUS = $(CXX)
endif
CONLY ?= $(CPLUS)
ifneq (,$(findstring environment, $(origin CC)))
CONLY = $(CC)
endif

# The most generic rules
#$(1) - is the target pattern
define make-cxx-obj
$1: %.cpp
	$$(CPLUS) $$(OUTPUTOBJ_KEY)$$@ $$(COMPILE_ONLY) $$(CPLUS_FLAGS) $$(CXX_ONLY_FLAGS) $$(CXX_WARN_SUPPRESS) $$(INCLUDES) $$<
endef

TEST_AFFIXES_OBJS=$(addsuffix .$(OBJ),$(addprefix %_,$(TEST_SUFFIXES)) $(addsuffix _%,$(TEST_PREFIXES)))

# Make will not process the same recipe for each test pattern (since the dependency on the same %.cpp)
# thus the separated recipes should be provided
$(foreach t,%.$(OBJ) $(TEST_AFFIXES_OBJS),$(eval $(call make-cxx-obj,$(t))))

.PRECIOUS: %.$(OBJ) %.$(TEST_EXT) %.res $(TEST_AFFIXES_OBJS)

# Rules for generating a test DLL
%_dll.$(OBJ): %.cpp
	$(CPLUS) $(COMPILE_ONLY) $(OUTPUTOBJ_KEY)$@ $(CPLUS_FLAGS) $(PIC_KEY) $(DEFINE_KEY)_USRDLL $(INCLUDES) $<

#$(1) - is the binary name
#$(2) - is the input obj files and libraries
define make-test-binary
	$(CPLUS) $(OUTPUT_KEY)$(strip $1) $(CPLUS_FLAGS) $(2) $(LIBS) $(LINK_FLAGS)
endef 

# LINK_FILES the list of options to link test specific files (libraries and object files)
LINK_FILES+=$(TEST_LIBS)
# Rule for generating executable test
%.$(TEST_EXT): %.$(OBJ) $(TEST_LIBS) $(TEST_PREREQUISITE) $(if $(use_proxy),$(PROXY.LIB))
	$(call make-test-binary,$@,$< $(LINK_FILES) $(PIE_FLAG))

# Rules for generating a test DLL
%_dll.$(DLL): LINK_FLAGS += $(PIC_KEY) $(DYLIB_KEY)
%_dll.$(DLL): TEST_LIBS := $(subst %_dll.$(DLL),,$(TEST_LIBS))
%_dll.$(DLL): %_dll.$(OBJ)
	$(call make-test-binary,$@,$< $(LINK_FILES))
.PRECIOUS: %_dll.$(OBJ) %_dll.$(DLL)

%.$(OBJ): %.c
	$(CONLY) $(COMPILE_ONLY) $(OUTPUTOBJ_KEY)$@ $(C_FLAGS) $(INCLUDES) $<

%.$(OBJ): %.asm
	$(ASM) $(ASM_FLAGS) $<

%.$(OBJ): %.s
	cpp <$< | grep -v '^#' >$*.tmp
	$(ASM) $(ASM_FLAGS) -o $@ $*.tmp

# Rule for generating .E file if needed for visual inspection
# Note that ICL treats an argument after PREPROC_ONLY as a file to open,
# so all uses of PREPROC_ONLY should be immediately followed by a file name
%.E: %.cpp
	$(CPLUS) $(CPLUS_FLAGS) $(CXX_ONLY_FLAGS) $(INCLUDES) $(PREPROC_ONLY) $< >$@

# TODO Rule for generating .asm file if needed for visual inspection
%.asm: %.cpp
	$(CPLUS) /c /FAs /Fa $(CPLUS_FLAGS) $(CXX_ONLY_FLAGS) $(INCLUDES) $<

# TODO Rule for generating .s file if needed for visual inspection
%.s: %.cpp
	$(CPLUS) -S $(CPLUS_FLAGS) $(CXX_ONLY_FLAGS) $(INCLUDES) $<

# Customizations
$(KNOWN_WARNINGS): %.$(OBJ): %.cpp
	$(CPLUS) $(COMPILE_ONLY) $(subst $(WARNING_KEY),,$(CPLUS_FLAGS)) $(CXX_ONLY_FLAGS) $(CXX_WARN_SUPPRESS) $(INCLUDES) $<

tbb_misc.$(OBJ): version_string.ver
tbb_misc.$(OBJ): INCLUDES+=$(INCLUDE_KEY).

tbb_misc.E: tbb_misc.cpp version_string.ver
	$(CPLUS) $(CPLUS_FLAGS) $(CXX_ONLY_FLAGS) $(INCLUDE_KEY). $(INCLUDES) $(PREPROC_ONLY) $< >$@

%.res: %.rc version_string.ver $(TBB.MANIFEST)
	rc /Fo$@ $(INCLUDES) $(filter /D%,$(CPLUS_FLAGS)) $<

# TODO: add $(LIB_LINK_LIBS) $(LIB_LINK_FLAGS) (in a separate line?) and remove useless $(INCLUDES)
VERSION_FLAGS=$(CPLUS) $(CPLUS_FLAGS) $(CXX_ONLY_FLAGS) $(INCLUDES)

ifneq (,$(TBB.MANIFEST))
$(TBB.MANIFEST):
	cmd /C "echo #include ^<stdio.h^> >tbbmanifest.c"
	cmd /C "echo int main(){return 0;} >>tbbmanifest.c"
	cl /nologo $(C_FLAGS) tbbmanifest.c

version_string.ver: $(TBB.MANIFEST)
	$(MAKE_VERSIONS)
	cmd /C "echo #define TBB_MANIFEST 1 >> version_string.ver"
# TODO: fix parallel build by writing to a temporary file and rename it when complete
else
# TODO: make version strings directly representative for all the libraries
version_string.ver:
	$(MAKE_VERSIONS)
endif

test_% debug_%: test_%.$(TEST_EXT) $(TEST_PREREQUISITE)
	$(run_cmd) ./$< $(args)
ifneq (,$(codecov))
	profmerge
	codecov $(if $(findstring -,$(codecov)),$(codecov),) -demang -comp $(tbb_root)/build/codecov.txt
endif

