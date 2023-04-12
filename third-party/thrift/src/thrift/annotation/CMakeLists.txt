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

thrift_generate(
  "api" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "cpp" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "hack" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "java" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "meta" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "python" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "scope" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

thrift_generate(
  "thrift" #file_name
  "" #services
  "cpp2" #language
  "templates,no_metadata" #options
  "${CMAKE_CURRENT_SOURCE_DIR}" #file_path
  "${CMAKE_CURRENT_BINARY_DIR}" #output_path
  "thrift/annotation" #include_prefix
  THRIFT_INCLUDE_DIRECTORIES ${THRIFT_SOURCE_DIR}
)

set_property(GLOBAL PROPERTY api-cpp2-SOURCES ${api-cpp2-SOURCES})
set_property(GLOBAL PROPERTY cpp-cpp2-SOURCES ${cpp-cpp2-SOURCES})
set_property(GLOBAL PROPERTY hack-cpp2-SOURCES ${hack-cpp2-SOURCES})
set_property(GLOBAL PROPERTY java-cpp2-SOURCES ${java-cpp2-SOURCES})
set_property(GLOBAL PROPERTY meta-cpp2-SOURCES ${meta-cpp2-SOURCES})
set_property(GLOBAL PROPERTY python-cpp2-SOURCES ${python-cpp2-SOURCES})
set_property(GLOBAL PROPERTY scope-cpp2-SOURCES ${scope-cpp2-SOURCES})
set_property(GLOBAL PROPERTY thrift-cpp2-SOURCES ${thrift-cpp2-SOURCES})
