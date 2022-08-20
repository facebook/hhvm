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
#   @language  - The generator to use (cpp or cpp2)
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
    "${ARGN}"
  )
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
endmacro()

# thrift_library
# Same as thrift object in terms of usage but creates the library instead of
# object so that you can use to link against your library instead of including
# all symbols into your library
#
# Params:
#   @file_name - The name of the thrift file
#   @services  - A list of services that are declared in the thrift file
#   @language  - The generator to use (cpp or cpp2)
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
#   target_link_libraries(somelibe ${file_name}-${language} ...)
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
    "${ARGN}"
  )
  add_library(
    "${file_name}-${language}"
    $<TARGET_OBJECTS:${file_name}-${language}-obj>
  )
  target_link_libraries("${file_name}-${language}" ${THRIFTCPP2})
  message("Thrift will create the Library file : ${file_name}-${language}")
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
# Params:
#   @file_name - The name of tge thrift file
#   @services  - A list of services that are declared in the thrift file
#   @language  - The generator to use (cpp, cpp2 or py3)
#   @options   - Extra options to pass to the generator
#   @output_path - The directory where the thrift file lives
#
# Output:
#  file-language-target     - A custom target to add a dependenct
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
    "" # One Value args
    "THRIFT_INCLUDE_DIRECTORIES" # Multi-value args
    "${ARGN}")

  set(thrift_include_directories)
  foreach(dir ${THRIFT_GENERATE_THRIFT_INCLUDE_DIRECTORIES})
    list(APPEND thrift_include_directories "-I" "${dir}")
  endforeach()

  set("${file_name}-${language}-HEADERS"
    ${output_path}/gen-${language}/${file_name}_constants.h
    ${output_path}/gen-${language}/${file_name}_data.h
    ${output_path}/gen-${language}/${file_name}_metadata.h
    ${output_path}/gen-${language}/${file_name}_types.h
    ${output_path}/gen-${language}/${file_name}_types.tcc
  )
  set("${file_name}-${language}-SOURCES"
    ${output_path}/gen-${language}/${file_name}_constants.cpp
    ${output_path}/gen-${language}/${file_name}_data.cpp
    ${output_path}/gen-${language}/${file_name}_types.cpp
  )
  if(NOT "${options}" MATCHES "no_metadata")
    set("${file_name}-${language}-SOURCES"
      ${${file_name}-${language}-SOURCES}
      ${output_path}/gen-${language}/${file_name}_metadata.cpp
    )
  endif()
  foreach(service ${services})
    set("${file_name}-${language}-HEADERS"
      ${${file_name}-${language}-HEADERS}
      ${output_path}/gen-${language}/${service}.h
      ${output_path}/gen-${language}/${service}.tcc
      ${output_path}/gen-${language}/${service}AsyncClient.h
      ${output_path}/gen-${language}/${service}_custom_protocol.h
    )
    set("${file_name}-${language}-SOURCES"
      ${${file_name}-${language}-SOURCES}
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
    file(WRITE "${output_path}/gen-${language}/${file_name}/__init__.py")
  endif()
  add_custom_command(
    OUTPUT ${${file_name}-${language}-HEADERS}
      ${${file_name}-${language}-SOURCES}
    COMMAND ${THRIFT1}
      --gen "${gen_language}:${options}${include_prefix_text}"
      -o ${output_path}
      ${thrift_include_directories}
      "${file_path}/${file_name}.thrift"
    DEPENDS
      ${THRIFT1}
      "${file_path}/${file_name}.thrift"
    COMMENT "Generating ${file_name} files. Output: ${output_path}"
  )
  add_custom_target(
    ${file_name}-${language}-target ALL
    DEPENDS ${${language}-${language}-HEADERS}
      ${${file_name}-${language}-SOURCES}
  )
  install(
    DIRECTORY gen-${language}
    DESTINATION include/${include_prefix}
    FILES_MATCHING PATTERN "*.h")
  install(
    DIRECTORY gen-${language}
    DESTINATION include/${include_prefix}
    FILES_MATCHING PATTERN "*.tcc")
endmacro()
