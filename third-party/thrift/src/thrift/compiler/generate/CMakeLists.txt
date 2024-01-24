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

add_executable(compiler_generate_build_templates build_templates.cc)
target_link_libraries(compiler_generate_build_templates Boost::filesystem)

file(
  GLOB_RECURSE
  TEMPLATE_FILES
  CONFIGURE_DEPENDS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/templates/**/*.mustache
)
add_custom_command(
  OUTPUT templates.cc
  COMMAND
    compiler_generate_build_templates
    ${CMAKE_CURRENT_SOURCE_DIR}/templates
    > templates.cc
  DEPENDS
    compiler_generate_build_templates
    ${TEMPLATE_FILES}
  VERBATIM
)

file(GLOB GENERATOR_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cc)
list(REMOVE_ITEM GENERATOR_FILES build_templates.cc)
list(REMOVE_ITEM GENERATOR_FILES t_ast_generator.cc)

add_library(compiler_generators STATIC ${GENERATOR_FILES}
            ${CMAKE_CURRENT_BINARY_DIR}/templates.cc)
set_target_properties(compiler_generators PROPERTIES
                      POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
target_link_libraries(
  compiler_generators
  compiler_ast
  compiler_base
  compiler
  mustache
  Boost::filesystem
  ${OPENSSL_LIBRARIES}
)
