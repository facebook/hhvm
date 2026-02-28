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

#
# Extra gmake command-line parameters for use with Android:
#
#    dlopen_workaround:  Some OS versions need workaround for dlopen to avoid recursive calls.
#

####### Detections and Commands ###############################################

ifeq (android,$(findstring android,$(tbb_os)))
    $(error TBB only supports cross-compilation for Android. Specify "target=android" instead.)
endif

ifndef BUILDING_PHASE
    ifneq ("command line","$(origin arch)")
        ifeq (icc,$(compiler))
            export COMPILER_VERSION := ICC: $(shell icc -V </dev/null 2>&1 | grep 'Version')
            ifneq (,$(findstring running on IA-32, $(COMPILER_VERSION)))
                export arch:=ia32
            else ifneq (,$(findstring running on Intel(R) 64, $(COMPILER_VERSION)))
                export arch:=intel64
            else
                $(error "No support for Android in $(COMPILER_VERSION)")
            endif

        else
            ifdef ANDROID_SERIAL
                uname_m:=$(shell adb shell uname -m)
                ifeq (i686,$(uname_m))
                    export arch:=ia32
                else
                    export arch:=$(uname_m)
                endif
            endif
        endif
    endif
endif

ifeq ("$(arch)","")
    $(error "No target architecture specified and \'ANDROID_SERIAL\' environment variable specifying target device not set")
endif

# Android platform only supported from TBB 4.1 forward
NO_LEGACY_TESTS = 1


