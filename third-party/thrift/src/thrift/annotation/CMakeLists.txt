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

add_library(thriftannotation)
target_link_libraries(thriftannotation PUBLIC Folly::folly)

foreach (name cpp hack java python scope thrift)
  thrift_generate(
    ${name} # file_name
    "" # services
    "cpp2" # language
    "templates,no_metadata" # options
    "${CMAKE_CURRENT_SOURCE_DIR}" # file_path
    "${CMAKE_CURRENT_BINARY_DIR}" # output_path
    "thrift/annotation" # include_prefix
    THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR})
  target_sources(thriftannotation PRIVATE ${${name}-cpp2-SOURCES})
endforeach ()

