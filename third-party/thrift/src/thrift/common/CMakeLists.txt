# Copyright (c) Meta Platforms, Inc. and affiliates.
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

set(COMPILER_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_library(
  common

  detail/string.cc
  tree_printer.cc
  universal_name.cc
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${INCLUDE_INSTALL_DIR}/thrift/common
  FILES_MATCHING PATTERN "*.h" PATTERN "*.tcc"
)

target_link_libraries(
  common
  Boost::boost
  fmt::fmt
)
