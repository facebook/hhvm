# Copyright 2015 Google Inc. All Rights Reserved.
# Distributed under MIT license.
#
# See file src/LICENSE for detail or copy at https://opensource.org/licenses/MIT

project(brotli C)

# Parse version information from common/version.h. Normally we would
# define these values here and write them out to configuration file(s)
# (i.e., config.h), but in this case we parse them from
# common/version.h to be less intrusive.
function(hex_to_dec HEXADECIMAL DECIMAL)
  string(TOUPPER "${HEXADECIMAL}" _tail)
  set(_decimal 0)
  string(LENGTH "${_tail}" _tail_length)
  while (_tail_length GREATER 0)
    math(EXPR _decimal "${_decimal} * 16")
    string(SUBSTRING "${_tail}" 0 1 _digit)
    string(SUBSTRING "${_tail}" 1 -1 _tail)
    if (_digit STREQUAL "A")
      math(EXPR _decimal "${_decimal} + 10")
    elseif (_digit STREQUAL "B")
      math(EXPR _decimal "${_decimal} + 11")
    elseif (_digit STREQUAL "C")
      math(EXPR _decimal "${_decimal} + 12")
    elseif (_digit STREQUAL "D")
      math(EXPR _decimal "${_decimal} + 13")
    elseif (_digit STREQUAL "E")
      math(EXPR _decimal "${_decimal} + 14")
    elseif (_digit STREQUAL "F")
      math(EXPR _decimal "${_decimal} + 15")
    else()
      math(EXPR _decimal "${_decimal} + ${_digit}")
    endif()
    string(LENGTH "${_tail}" _tail_length)
  endwhile()
  set(${DECIMAL} ${_decimal} PARENT_SCOPE)
endfunction(hex_to_dec)

# Version information
file(STRINGS "src/c/common/version.h" _brotli_version_line REGEX "^#define BROTLI_VERSION (0x[0-9a-fA-F]+)$")
string(REGEX REPLACE "^#define BROTLI_VERSION 0x([0-9a-fA-F]+)$" "\\1" _brotli_version_hex "${_brotli_version_line}")
hex_to_dec("${_brotli_version_hex}" _brotli_version)
math(EXPR BROTLI_VERSION_MAJOR "${_brotli_version} >> 24")
math(EXPR BROTLI_VERSION_MINOR "(${_brotli_version} >> 12) & 4095")
math(EXPR BROTLI_VERSION_PATCH "${_brotli_version} & 4095")
set(BROTLI_VERSION "${BROTLI_VERSION_MAJOR}.${BROTLI_VERSION_MINOR}.${BROTLI_VERSION_PATCH}")
mark_as_advanced(BROTLI_VERSION BROTLI_VERSION_MAJOR BROTLI_VERSION_MINOR BROTLI_VERSION_PATCH)

# ABI Version information
file(STRINGS "src/c/common/version.h" _brotli_abi_info_line REGEX "^#define BROTLI_ABI_VERSION (0x[0-9a-fA-F]+)$")
string(REGEX REPLACE "^#define BROTLI_ABI_VERSION 0x([0-9a-fA-F]+)$" "\\1" _brotli_abi_info_hex "${_brotli_abi_info_line}")
hex_to_dec("${_brotli_abi_info_hex}" _brotli_abi_info)
math(EXPR BROTLI_ABI_CURRENT "${_brotli_abi_info} >> 24")
math(EXPR BROTLI_ABI_REVISION "(${_brotli_abi_info} >> 12) & 4095")
math(EXPR BROTLI_ABI_AGE "${_brotli_abi_info} & 4095")
math(EXPR BROTLI_ABI_COMPATIBILITY "${BROTLI_ABI_CURRENT} - ${BROTLI_ABI_AGE}")
mark_as_advanced(BROTLI_ABI_CURRENT BROTLI_ABI_REVISION BROTLI_ABI_AGE BROTLI_ABI_COMPATIBILITY)

if (ENABLE_SANITIZER)
  set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -fsanitize=${ENABLE_SANITIZER}")
  set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fsanitize=${ENABLE_SANITIZER}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${ENABLE_SANITIZER}")

  # By default, brotli depends on undefined behavior, but setting
  # BROTLI_BUILD_PORTABLE should result in a build which does not.
  if(ENABLE_SANITIZER STREQUAL "undefined")
    add_definitions(-DBROTLI_BUILD_PORTABLE)
  endif()
endif ()

include(CheckFunctionExists)
set(LIBM_LIBRARY)
CHECK_FUNCTION_EXISTS(log2 LOG2_RES)
if(NOT LOG2_RES)
  set(orig_req_libs "${CMAKE_REQUIRED_LIBRARIES}")
  set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};m")
  CHECK_FUNCTION_EXISTS(log2 LOG2_LIBM_RES)
  if(LOG2_LIBM_RES)
    set(LIBM_LIBRARY "m")
  else()
    message(FATAL_ERROR "log2() not found")
  endif()

  set(CMAKE_REQUIRED_LIBRARIES "${orig_req_libs}")
  unset(LOG2_LIBM_RES)
  unset(orig_req_libs)
endif()
unset(LOG2_RES)

set(BROTLI_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/c/include")
mark_as_advanced(BROTLI_INCLUDE_DIRS)

set(BROTLI_LIBRARIES_CORE brotlienc brotlidec brotlicommon)
set(BROTLI_LIBRARIES ${BROTLI_LIBRARIES_CORE} ${LIBM_LIBRARY})
mark_as_advanced(BROTLI_LIBRARIES)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DOS_LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  add_definitions(-DOS_FREEBSD)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DOS_MACOSX)
endif()

function(transform_sources_list INPUT_FILE OUTPUT_FILE)
  file(READ ${INPUT_FILE} TEXT)
  string(REGEX REPLACE "\\\\\n" "~continuation~" TEXT ${TEXT})
  string(REGEX REPLACE "([a-zA-Z_][a-zA-Z0-9_]*)[\t ]*=[\t ]*([^\n]*)" "SET(\\1 \\2)" TEXT ${TEXT})
  string(REPLACE "~continuation~" "\n" TEXT ${TEXT})
  file(WRITE ${OUTPUT_FILE} ${TEXT})
endfunction()

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

transform_sources_list("src/scripts/sources.lst" "${CMAKE_CURRENT_BINARY_DIR}/sources.lst.cmake")
include("${CMAKE_CURRENT_BINARY_DIR}/sources.lst.cmake")

PREPEND(BROTLI_COMMON_C_SRCS "src/" ${BROTLI_COMMON_C})
PREPEND(BROTLI_ENC_C_SRCS "src/" ${BROTLI_ENC_C})
PREPEND(BROTLI_DEC_C_SRCS "src/" ${BROTLI_DEC_C})

set(BROTLI_STATIC_LIBS brotlicommon brotlidec brotlienc)
add_library(brotlicommon STATIC ${BROTLI_COMMON_C_SRCS})
add_library(brotlidec STATIC ${BROTLI_DEC_C_SRCS})
add_library(brotlienc STATIC ${BROTLI_ENC_C_SRCS})

# Older CMake versions does not understand INCLUDE_DIRECTORIES property.
target_include_directories(brotlicommon PUBLIC ${BROTLI_INCLUDE_DIRS})
target_include_directories(brotlidec PUBLIC ${BROTLI_INCLUDE_DIRS})
target_include_directories(brotlienc PUBLIC ${BROTLI_INCLUDE_DIRS})

foreach(lib IN LISTS BROTLI_STATIC_LIBS)
  target_link_libraries(${lib} ${LIBM_LIBRARY})
  set_property(TARGET ${lib} APPEND PROPERTY INCLUDE_DIRECTORIES ${BROTLI_INCLUDE_DIRS})
  set_target_properties(${lib} PROPERTIES
    VERSION "${BROTLI_ABI_COMPATIBILITY}.${BROTLI_ABI_AGE}.${BROTLI_ABI_REVISION}"
    SOVERSION "${BROTLI_ABI_COMPATIBILITY}")
  set_property(TARGET ${lib} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${BROTLI_INCLUDE_DIRS}")
endforeach()

target_link_libraries(brotlidec brotlicommon)
target_link_libraries(brotlienc brotlicommon)

# For projects stuck on older versions of CMake, this will set the
# BROTLI_INCLUDE_DIRS and BROTLI_LIBRARIES variables so they still
# have a relatively easy way to use Brotli:
#
#   include_directories(${BROTLI_INCLUDE_DIRS})
#   target_link_libraries(foo ${BROTLI_LIBRARIES})
if(BROTLI_PARENT_DIRECTORY)
  set(BROTLI_INCLUDE_DIRS "${BROTLI_INCLUDE_DIRS}" PARENT_SCOPE)
  set(BROTLI_LIBRARIES "${BROTLI_LIBRARIES}" PARENT_SCOPE)
endif()
