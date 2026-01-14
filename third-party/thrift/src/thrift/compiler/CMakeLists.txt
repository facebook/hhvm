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

# Set the compiler directory.
set(COMPILER_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Override the default install path for `make install` step.
set(CMAKE_INSTALL_PREFIX ${THRIFT_SOURCE_DIR})

check_cxx_source_compiles("
  #include <type_traits>
  #if _GLIBCXX_RELEASE
  int main() {}
  #endif"
  FBTHRIFT_COMPILER_STDLIB_LIBSTDCXX
)
check_cxx_source_compiles("
  #include <type_traits>
  #if _GLIBCXX_RELEASE >= 9
  int main() {}
  #endif"
  FBTHRIFT_COMPILER_STDLIB_LIBSTDCXX_GE_9
)
check_cxx_source_compiles("
  #include <type_traits>
  #if _LIBCPP_VERSION
  int main() {}
  #endif"
  FBTHRIFT_COMPILER_STDLIB_LIBCXX
)
check_cxx_source_compiles("
  #include <type_traits>
  #if _LIBCPP_VERSION >= 9000
  int main() {}
  #endif"
  FBTHRIFT_COMPILER_STDLIB_LIBCXX_GE_9
)

if (FBTHRIFT_COMPILER_STDLIB_LIBSTDCXX AND NOT FBTHRIFT_COMPILER_STDLIB_LIBSTDCXX_GE_9)
  list (APPEND FBTHRIFT_COMPILER_LINK_CXXFS stdc++fs)
endif()
if (FBTHRIFT_COMPILER_STDLIB_LIBCXX AND NOT FBTHRIFT_COMPILER_STDLIB_LIBCXX_GE_9)
  list (APPEND FBTHRIFT_COMPILER_LINK_CXXFS c++fs)
endif ()

# The Thrift AST library.
add_library(
  compiler_ast

  ast/t_primitive_type.cc
  ast/t_const.cc
  ast/t_const_value.cc
  ast/t_container.cc
  ast/t_enum.cc
  ast/t_exception.cc
  ast/t_field.cc
  ast/t_function.cc
  ast/t_interaction.cc
  ast/t_interface.cc
  ast/t_list.cc
  ast/t_named.cc
  ast/t_node.cc
  ast/t_map.cc
  ast/t_package.cc
  ast/t_program.cc
  ast/t_global_scope.cc
  ast/t_set.cc
  ast/t_structured.cc
  ast/t_type.cc
  ast/t_typedef.cc
  ast/t_union.cc
  ast/scope_identifier.cc
)
target_link_libraries(
  compiler_ast
  ${OPENSSL_LIBRARIES}
  Boost::boost
  fmt::fmt
  Folly::folly
)

# The base compiler infrastructure library
add_library(
  compiler_base
  STATIC

  detail/system.cc
  detail/pluggable_functions.cc
  diagnostic.cc
  source_location.cc
)
target_link_libraries(
  compiler_base
  fmt::fmt
  Boost::boost
  Folly::folly
  common
  ${FBTHRIFT_COMPILER_LINK_CXXFS}
)

# The base Thrift compiler library.
add_library(
  compiler_lib
  STATIC

  parse/lexer.cc
  parse/lexer.h
  parse/parser.cc
  parse/parser.h
  parse/parse_ast.cc
  parse/parse_ast.h
  parse/token.cc
  parse/token.h
  sema/ast_uri_utils.cc
  sema/check_initializer.cc
  sema/check_map_keys.cc
  sema/explicit_include_validator.cc
  sema/scope_validator.cc
  sema/sema.cc
  sema/sema_context.cc
  sema/schematizer.cc
  sema/standard_validator.cc
  sema/sema_context.cc
  sema/resolution_mismatch.cc
)
target_compile_definitions(compiler_lib PRIVATE THRIFT_OSS)
target_link_libraries(
  compiler_lib
  compiler_base
  compiler_ast
  Boost::regex
  fmt::fmt
  Folly::folly
)

# The Thrift compiler library.
add_library(
  compiler

  generate/cpp/reference_type.cc
  generate/cpp/name_resolver.cc
  generate/cpp/util.cc
  generate/go/util.cc
  generate/java/util.cc
  generate/python/util.cc
  generate/rust/util.cc
)
target_link_libraries(
  compiler
  compiler_ast
  fmt::fmt
  Folly::folly
  ${FBTHRIFT_COMPILER_LINK_CXXFS}
)

add_library(
  whisker
  STATIC

  whisker/ast.cc
  whisker/dsl.cc
  whisker/detail/string.cc
  whisker/eval_context.cc
  whisker/lexer.cc
  whisker/mstch_compat.cc
  whisker/object.cc
  whisker/parser.cc
  whisker/print_ast.cc
  whisker/render.cc
  whisker/standard_library.cc
  whisker/token.cc
)
target_link_libraries(
  whisker
  fmt::fmt
  Folly::folly
  compiler_base
)

add_subdirectory(generate)

# Force compiler_generators linking (compiler will optimize it out otherwise).
if (MSVC)
  set(GENERATE_LINKED compiler_generators) # MSVC WHOLEARCHIVE is set after exe.
elseif (APPLE)
  set(GENERATE_LINKED -Wl,-force_load compiler_generators)
else ()
  set(GENERATE_LINKED
      -Wl,--whole-archive compiler_generators -Wl,--no-whole-archive)
endif ()

# Create the Thrift compiler binary.
add_executable(
  thrift1
  main.cc
  compiler.cc
)
target_link_libraries(
  thrift1
  compiler_ast
  compiler_lib
  ${FBTHRIFT_COMPILER_LINK_CXXFS}
  ${GENERATE_LINKED}
)
# Force compiler_generators linking (compiler will optimize it out otherwise).
if (MSVC)
  set_target_properties(
    thrift1
    PROPERTIES LINK_FLAGS "/WHOLEARCHIVE:compiler_generators"
  )
endif ()

if (BUILD_SHARED_LIBS)
  # All but thrift1 since it's a binary.
  set_target_properties(
    compiler_ast
    compiler_base
    compiler_lib
    compiler
    whisker
    compiler_generators
    PROPERTIES VERSION ${PACKAGE_VERSION}
  )
endif ()

install(
  TARGETS thrift1
    compiler_ast
    compiler_base
    compiler_lib
    compiler
    whisker
    common
  EXPORT fbthrift-exports
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)
