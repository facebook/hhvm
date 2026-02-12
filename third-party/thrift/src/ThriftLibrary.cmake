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

#
# Requirements:
# Please provide the following two variables before using these macros:
#   ${THRIFT1} - path/to/bin/thrift1
#   ${THRIFTCPP2} - path/to/lib/thriftcpp2
#

#
# thrift_object
# This creates a object that will contain the source files and all the proper
# dependencies to generate and compile thrift generated files
#
# Params:
#   @file_name - The name of the thrift file
#   @services  - A list of services that are declared in the thrift file
#   @language  - The generator to use (cpp, cpp2, or python)
#   @options   - Extra options to pass to the generator
#   @file_path - The directory where the thrift file lives
#   @output_path - The directory where the thrift objects will be built
#   @include_prefix - The last part of output_path, relative include prefix
#
# Output:
#  A object file named `${file-name}-${language}-obj` to include into your
#  project's library
#
# Notes:
# If any of the fields is empty, it is still required to provide an empty string
#
# Usage:
#   thrift_object(
#     #file_name
#     #services
#     #language
#     #options
#     #file_path
#     #output_path
#     #include_prefix
#   )
#   add_library(somelib $<TARGET_OBJECTS:${file_name}-${language}-obj> ...)
#

macro(thrift_object
  file_name
  services
  language
  options
  file_path
  output_path
  include_prefix
)
  thrift_generate(
    "${file_name}"
    "${services}"
    "${language}"
    "${options}"
    "${file_path}"
    "${output_path}"
    "${include_prefix}"
    ${ARGN}
  )
  if("${language}" STREQUAL "python")
    # Python: generated .py files are the artifacts
    message("Thrift will generate Python files for : ${file_name}-${language}")
  else()
    bypass_source_check(${${file_name}-${language}-SOURCES})
    add_library(
      "${file_name}-${language}-obj"
      OBJECT
      ${${file_name}-${language}-SOURCES}
    )
    add_dependencies(
      "${file_name}-${language}-obj"
      "${file_name}-${language}-target"
    )
    message("Thrift will create the Object file : ${file_name}-${language}-obj")
  endif()
endmacro()

# thrift_library
# Same as thrift object in terms of usage but creates the library instead of
# object so that you can use to link against your library instead of including
# all symbols into your library
#
# Params:
#   @file_name - The name of the thrift file
#   @services  - A list of services that are declared in the thrift file
#   @language  - The generator to use (cpp, cpp2, or python)
#   @options   - Extra options to pass to the generator
#   @file_path - The directory where the thrift file lives
#   @output_path - The directory where the thrift objects will be built
#   @include_prefix - The last part of output_path, relative include prefix
#
# Output:
#  A library file named `${file-name}-${language}` to link against your
#  project's library
#
# Notes:
# If any of the fields is empty, it is still required to provide an empty string
#
# Usage:
#   thrift_library(
#     #file_name
#     #services
#     #language
#     #options
#     #file_path
#     #output_path
#     #include_prefix
#   )
#   add_library(somelib ...)
#   target_link_libraries(somelib ${file_name}-${language} ...)
#

macro(thrift_library
  file_name
  services
  language
  options
  file_path
  output_path
  include_prefix
)
  thrift_object(
    "${file_name}"
    "${services}"
    "${language}"
    "${options}"
    "${file_path}"
    "${output_path}"
    "${include_prefix}"
    ${ARGN}
  )
  if("${language}" STREQUAL "python")
    # Python: create an alias target so users can depend on <name>-python
    add_custom_target("${file_name}-${language}" ALL)
    add_dependencies("${file_name}-${language}" "${file_name}-${language}-target")
    message("Thrift will create the Python library : ${file_name}-${language}")
  else()
    add_library(
      "${file_name}-${language}"
      $<TARGET_OBJECTS:${file_name}-${language}-obj>
    )
    target_link_libraries("${file_name}-${language}" ${THRIFTCPP2})
    message("Thrift will create the Library file : ${file_name}-${language}")
  endif()
endmacro()

#
# bypass_source_check
# This tells cmake to ignore if it doesn't see the following sources in
# the library that will be installed. Thrift files are generated at compile
# time so they do not exist at source check time
#
# Params:
#   @sources - The list of files to ignore in source check
#

macro(bypass_source_check sources)
  set_source_files_properties(
    ${sources}
    PROPERTIES GENERATED TRUE
  )
endmacro()

#
# thrift_generate
# This is used to codegen thrift files using the thrift compiler
# Supports library names that differ from the file name (to handle two libraries
# with the same filename on disk (in different folders))
# Params:
#   @file_name - Input file name. Will be used for naming the CMake
#       target if TARGET_NAME_BASE is not specified.
#   @services  - A list of services that are declared in the thrift file
#   @language  - The generator to use (cpp, cpp2, py3, or python)
#   @options   - Extra options to pass to the generator
#   @output_path - The directory where the thrift file lives
#   @include_prefix - Prefix to use for thrift includes in generated sources
#   @TARGET_NAME_BASE (optional) - name used for target instead of real filename
#   @THRIFT_INCLUDE_DIRECTORIES (optional) path to thrift include directories
#   @NAMESPACE (optional) - Must match the "namespace py3" directive in the
#       .thrift file. Dot-separated (e.g. "example.sum") maps to output
#       directory structure (example/sum/<file_name>/).
#
# Output:
#  file-language-target     - A custom target to add a dependency
#  ${file-language-HEADERS} - The generated Header Files.
#  ${file-language-SOURCES} - The generated Source Files.
#
# Notes:
# If any of the fields is empty, it is still required to provide an empty string
#
# When using file_language-SOURCES it should always call:
#   bypass_source_check(${file_language-SOURCES})
# This will prevent cmake from complaining about missing source files
#
macro(thrift_generate
  file_name
  services
  language
  options
  file_path
  output_path
  include_prefix
)
  cmake_parse_arguments(THRIFT_GENERATE   # Prefix
    "" # Options
    "TARGET_NAME_BASE;NAMESPACE" # One Value args
    "THRIFT_INCLUDE_DIRECTORIES" # Multi-value args
    "${ARGN}")

  set(source_file_name ${file_name})
  set(target_file_name ${file_name})
  set(thrift_include_directories)
  foreach(dir ${THRIFT_GENERATE_THRIFT_INCLUDE_DIRECTORIES})
    list(APPEND thrift_include_directories "-I" "${dir}")
  endforeach()
  if(DEFINED THRIFT_GENERATE_TARGET_NAME_BASE
     AND NOT THRIFT_GENERATE_TARGET_NAME_BASE STREQUAL "")
    set(target_file_name ${THRIFT_GENERATE_TARGET_NAME_BASE})
  endif()

  set("${target_file_name}-${language}-HEADERS"
    ${output_path}/gen-${language}/${source_file_name}_constants.h
    ${output_path}/gen-${language}/${source_file_name}_data.h
    ${output_path}/gen-${language}/${source_file_name}_metadata.h
    ${output_path}/gen-${language}/${source_file_name}_types.h
    ${output_path}/gen-${language}/${source_file_name}_types.tcch
    ${output_path}/gen-${language}/${source_file_name}_types_custom_protocol.h
  )
  set("${target_file_name}-${language}-SOURCES"
    ${output_path}/gen-${language}/${source_file_name}_constants.cpp
    ${output_path}/gen-${language}/${source_file_name}_data.cpp
    ${output_path}/gen-${language}/${source_file_name}_types.cpp
    ${output_path}/gen-${language}/${source_file_name}_types_binary.cpp
    ${output_path}/gen-${language}/${source_file_name}_types_compact.cpp
    ${output_path}/gen-${language}/${source_file_name}_types_serialization.cpp
  )
  if("${options}" MATCHES "layouts")
    set("${target_file_name}-${language}-SOURCES"
      ${${target_file_name}-${language}-SOURCES}
      ${output_path}/gen-${language}/${source_file_name}_layouts.cpp
    )
  endif()
  if(NOT "${options}" MATCHES "no_metadata")
    set("${target_file_name}-${language}-SOURCES"
      ${${target_file_name}-${language}-SOURCES}
      ${output_path}/gen-${language}/${source_file_name}_metadata.cpp
    )
  endif()
  foreach(service ${services})
    set("${target_file_name}-${language}-HEADERS"
      ${${source_file_name}-${language}-HEADERS}
      ${output_path}/gen-${language}/${service}.h
      ${output_path}/gen-${language}/${service}.tcc
      ${output_path}/gen-${language}/${service}AsyncClient.h
      ${output_path}/gen-${language}/${service}_custom_protocol.h
    )
    set("${target_file_name}-${language}-SOURCES"
      ${${source_file_name}-${language}-SOURCES}
      ${output_path}/gen-${language}/${service}.cpp
      ${output_path}/gen-${language}/${service}AsyncClient.cpp
    )
  endforeach()
  if("${include_prefix}" STREQUAL "")
    set(include_prefix_text "")
  else()
    set(include_prefix_text "include_prefix=${include_prefix}")
    if(NOT "${options}" STREQUAL "")
      set(include_prefix_text ",${include_prefix_text}")
    endif()
  endif()
  set(gen_language ${language})
  if("${language}" STREQUAL "cpp2")
    set(gen_language "mstch_cpp2")
  elseif("${language}" STREQUAL "py3")
    set(gen_language "mstch_py3")
    file(WRITE "${output_path}/gen-${language}/${source_file_name}/__init__.py")
  elseif("${language}" STREQUAL "python")
    set(gen_language "mstch_python")
    # thrift-python uses PEP 420 namespace packages — no __init__.py.
    # NAMESPACE must match "namespace py3" in the .thrift file.
    # Dot-separated value maps to directory: "example.sum" -> example/sum/<file_name>/
    if(DEFINED THRIFT_GENERATE_NAMESPACE
        AND NOT THRIFT_GENERATE_NAMESPACE STREQUAL "")
      string(REPLACE "." "/" _namespace_dir "${THRIFT_GENERATE_NAMESPACE}")
      set(_python_output_subdir "${_namespace_dir}/${source_file_name}")
    else()
      set(_python_output_subdir "${source_file_name}")
    endif()
    # Override C++ HEADERS/SOURCES with Python output files
    set("${target_file_name}-${language}-HEADERS" "")
    set("${target_file_name}-${language}-SOURCES"
      ${output_path}/gen-python/${_python_output_subdir}/thrift_types.py
      ${output_path}/gen-python/${_python_output_subdir}/thrift_types.pyi
      ${output_path}/gen-python/${_python_output_subdir}/thrift_metadata.py
      ${output_path}/gen-python/${_python_output_subdir}/thrift_enums.py
      ${output_path}/gen-python/${_python_output_subdir}/thrift_abstract_types.py
      ${output_path}/gen-python/${_python_output_subdir}/thrift_mutable_types.py
      ${output_path}/gen-python/${_python_output_subdir}/thrift_mutable_types.pyi
    )
    if(NOT "${services}" STREQUAL "")
      list(APPEND "${target_file_name}-${language}-SOURCES"
        ${output_path}/gen-python/${_python_output_subdir}/thrift_services.py
        ${output_path}/gen-python/${_python_output_subdir}/thrift_clients.py
        ${output_path}/gen-python/${_python_output_subdir}/thrift_mutable_services.py
        ${output_path}/gen-python/${_python_output_subdir}/thrift_mutable_clients.py
      )
    endif()
    # Python options format differs from C++: no include_prefix
    if(NOT "${options}" STREQUAL "")
      set(_python_gen_options ":${options}")
    else()
      set(_python_gen_options "")
    endif()
    add_custom_command(
      OUTPUT ${${target_file_name}-${language}-SOURCES}
      COMMAND ${THRIFT1}
        --gen "${gen_language}${_python_gen_options}"
        -o ${output_path}
        ${thrift_include_directories}
        "${file_path}/${source_file_name}.thrift"
      DEPENDS
        ${THRIFT1}
        "${file_path}/${source_file_name}.thrift"
      COMMENT "Generating ${target_file_name} thrift-python files. Output: ${output_path}"
    )
    add_custom_target(
      ${target_file_name}-${language}-target ALL
      DEPENDS ${${target_file_name}-${language}-SOURCES}
    )
  endif()
  if(NOT "${language}" STREQUAL "python")
    add_custom_command(
      OUTPUT ${${target_file_name}-${language}-HEADERS}
        ${${target_file_name}-${language}-SOURCES}
      COMMAND ${THRIFT1}
        --gen "${gen_language}:${options}${include_prefix_text}"
        -o ${output_path}
        ${thrift_include_directories}
        "${file_path}/${source_file_name}.thrift"
      DEPENDS
        ${THRIFT1}
        "${file_path}/${source_file_name}.thrift"
      COMMENT "Generating ${target_file_name} files. Output: ${output_path}"
    )
    add_custom_target(
      ${target_file_name}-${language}-target ALL
      DEPENDS ${${language}-${language}-HEADERS}
        ${${target_file_name}-${language}-SOURCES}
    )
    install(
      DIRECTORY gen-${language}
      DESTINATION include/${include_prefix}
      FILES_MATCHING PATTERN "*.h")
    install(
      DIRECTORY gen-${language}
      DESTINATION include/${include_prefix}
      FILES_MATCHING PATTERN "*.tcc")
  endif()
endmacro()
