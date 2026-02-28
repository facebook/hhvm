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

ifeq ($(tbb_os),mic)
  $(error MIC supports only cross-compilation.  Specify "target=mic" instead.)
endif

ifneq ($(BUILDING_PHASE),1)
  # The same build prefix should be used in offload.inc
  ifeq (,$(tbb_build_prefix))
    tbb_build_prefix=mic_icc$(CPF_SUFFIX)
  endif
  # For examples
  mic_tbb_build_prefix=$(tbb_build_prefix)
endif

MAKE_VERSIONS=sh $(tbb_root)/build/version_info_linux.sh $(VERSION_FLAGS) >version_string.ver
MAKE_TBBVARS=sh $(tbb_root)/build/generate_tbbvars.sh MIC_ MIC_
def_prefix=lin64

TEST_LAUNCHER=
run_cmd ?= bash $(tbb_root)/build/mic.linux.launcher.sh $(largs)

# detects whether examples are being built.
ifeq ($(BUILDING_PHASE),0)
 export UI = con
 export x64 = 64
endif # examples
