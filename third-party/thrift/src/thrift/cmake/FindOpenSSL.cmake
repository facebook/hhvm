# Copyright (c) Facebook, Inc. and its affiliates.
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

# A wrapper around the standard CMake FindOpenSSL module.

if (APPLE AND NOT DEFINED OPENSSL_ROOT_DIR)
  execute_process(COMMAND brew --prefix openssl
                  OUTPUT_VARIABLE OPENSSL_ROOT_DIR
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

set(saved_path ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_STD_MODULE_PATH})
find_package(OpenSSL ${ARGN})
set(CMAKE_MODULE_PATH ${saved_path})
