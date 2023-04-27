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

ifneq ($(arch),$(filter $(arch),ia32 intel64 armv7 armv7s arm64))
  $(error $(arch) is unknown architecture. Known arhitechtures are ia32 intel64 armv7 armv7s arm64)
endif

# If target is ios but arch is ia32/intel64 then build for 32/64 simulator!
ifeq (,$(SDKROOT))
  ifeq ($(arch),$(filter $(arch),ia32 intel64))
    export SDKROOT:=$(shell xcodebuild -sdk -version | grep -o -E '/.*SDKs/iPhoneSimulator.*' 2>/dev/null)
  else
    export SDKROOT:=$(shell xcodebuild -sdk -version | grep -o -E '/.*SDKs/iPhoneOS.*' 2>/dev/null)
  endif
endif
ifeq (,$(SDKROOT))
  $(error iOS* SDK not found)
endif

ios_version:=$(shell echo $(SDKROOT) | sed -e "s/.*[a-z,A-Z]\(.*\).sdk/\1/")
runtime:=cc$(clang_version)_ios$(ios_version)
