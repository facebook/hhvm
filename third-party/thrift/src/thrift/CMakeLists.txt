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

add_subdirectory(annotation)

if (THRIFT_COMPILER_ONLY OR build_all)
  add_subdirectory(compiler)
endif ()

if (THRIFT_LIB_ONLY OR build_all)
  add_subdirectory(lib)
endif ()

# Install CMake package configuration files for thrift.
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/FBThriftConfig.cmake.in
  FBThriftConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
  PATH_VARS
    CMAKE_INSTALL_DIR
    BIN_INSTALL_DIR
    INCLUDE_INSTALL_DIR
)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/FBThriftConfig.cmake
  DESTINATION ${CMAKE_INSTALL_DIR}
)
install(
  EXPORT fbthrift-exports
  FILE FBThriftTargets.cmake
  NAMESPACE FBThrift::
  DESTINATION ${CMAKE_INSTALL_DIR}
)

install(
  DIRECTORY annotation ${CMAKE_CURRENT_SOURCE_DIR}/annotation
  DESTINATION include/thrift
  FILES_MATCHING PATTERN *.thrift
)

install(
  DIRECTORY annotation ${CMAKE_CURRENT_BINARY_DIR}/annotation
  DESTINATION include/thrift
  FILES_MATCHING
    PATTERN *.h
    PATTERN *.tcc
    PATTERN CMakeFiles EXCLUDE
)
