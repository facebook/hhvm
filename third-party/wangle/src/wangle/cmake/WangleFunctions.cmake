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

# Install header files preserving directory structure
# Similar to folly's auto_install_files()
function(wangle_install_headers rootName rootDir)
  file(TO_CMAKE_PATH "${rootDir}" rootDir)
  string(LENGTH "${rootDir}" rootDirLength)
  foreach(fil ${ARGN})
    file(TO_CMAKE_PATH "${fil}" filePath)
    string(FIND "${filePath}" "/" rIdx REVERSE)
    if(rIdx EQUAL -1)
      continue()
    endif()
    string(SUBSTRING "${filePath}" 0 ${rIdx} filePath)

    string(LENGTH "${filePath}" filePathLength)
    string(FIND "${filePath}" "${rootDir}" rIdx)
    if(rIdx EQUAL 0)
      math(EXPR filePathLength "${filePathLength} - ${rootDirLength}")
      string(SUBSTRING "${filePath}" ${rootDirLength} ${filePathLength} fileGroup)
      install(FILES ${fil}
              DESTINATION ${INCLUDE_INSTALL_DIR}/${rootName}${fileGroup})
    endif()
  endforeach()
endfunction()

# Initialize global properties for tracking targets and deferred dependencies
set_property(GLOBAL PROPERTY WANGLE_COMPONENT_TARGETS)
set_property(GLOBAL PROPERTY WANGLE_DEFERRED_DEPS)
set_property(GLOBAL PROPERTY WANGLE_GRANULAR_INTERFACE_TARGETS)

# Define a granular wangle library that:
# 1. Compiles sources ONCE via OBJECT library
# 2. Creates a STATIC library for individual linking (static builds)
# 3. Creates an INTERFACE library linking to monolithic wangle (shared builds)
# 4. Defers internal wangle deps to be resolved later
# 5. Tracks OBJECT target for monolithic aggregation
#
# Usage:
#   wangle_add_library(wangle_ssl_util
#     SRCS SSLUtil.cpp
#     DEPS wangle_ssl_ssl_stats             # Private dependencies
#     EXPORTED_DEPS wangle_ssl_ssl_context  # Public dependencies (propagated)
#   )
function(wangle_add_library _target_name)
  cmake_parse_arguments(
    WANGLE_LIB
    ""                              # Options (boolean flags)
    ""                              # Single-value args
    "SRCS;DEPS;EXPORTED_DEPS"       # Multi-value args
    ${ARGN}
  )

  set(_sources ${WANGLE_LIB_SRCS})
  if(NOT _sources)
    # Legacy support: if no SRCS keyword, treat remaining args as sources
    set(_sources ${WANGLE_LIB_UNPARSED_ARGUMENTS})
  endif()

  # Object library name - used for monolithic aggregation
  set(_obj_target "${_target_name}_obj")

  # Skip if no sources (header-only library)
  list(LENGTH _sources _src_count)
  if(_src_count EQUAL 0)
    # Header-only: create INTERFACE library
    add_library(${_target_name} INTERFACE)
    target_include_directories(${_target_name}
      INTERFACE
        $<BUILD_INTERFACE:${WANGLE_BASE_DIR}>
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
    )

    # Link exported deps for INTERFACE libraries
    if(WANGLE_LIB_EXPORTED_DEPS)
      target_link_libraries(${_target_name} INTERFACE ${WANGLE_LIB_EXPORTED_DEPS})
    endif()

    install(TARGETS ${_target_name} EXPORT wangle-exports)
    add_library(wangle::${_target_name} ALIAS ${_target_name})
    return()
  endif()

  # 1. Create OBJECT library (compiles sources once)
  add_library(${_obj_target} OBJECT ${_sources})

  if(BUILD_SHARED_LIBS)
    set_property(TARGET ${_obj_target} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  target_include_directories(${_obj_target}
    PUBLIC
      $<BUILD_INTERFACE:${WANGLE_BASE_DIR}>
      $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
      ${FOLLY_INCLUDE_DIR}
      ${FIZZ_INCLUDE_DIR}
      ${OPENSSL_INCLUDE_DIR}
      ${GLOG_INCLUDE_DIRS}
      ${GFLAGS_INCLUDE_DIRS}
      ${LIBEVENT_INCLUDE_DIR}
      ${DOUBLE_CONVERSION_INCLUDE_DIR}
  )

  target_compile_features(${_obj_target} PUBLIC cxx_std_17)

  # Link external dependencies on OBJECT library
  target_link_libraries(${_obj_target}
    PUBLIC
      ${OPENSSL_LIBRARIES}
      Threads::Threads
    PRIVATE
      ${GLOG_LIBRARIES}
      ${GFLAGS_LIBRARIES}
      ${LIBEVENT_LIB}
      ${DOUBLE_CONVERSION_LIBRARY}
      ${CMAKE_DL_LIBS}
      ${LIBRT_LIBRARIES}
  )

  # Separate wangle internal deps (defer) from external deps (link immediately)
  set(_immediate_deps "")
  set(_wangle_deps "")
  foreach(_dep IN LISTS WANGLE_LIB_EXPORTED_DEPS)
    if(_dep MATCHES "^wangle_")
      list(APPEND _wangle_deps ${_dep})
    else()
      # Folly::*, fizz::*, external libs, etc. - link immediately
      list(APPEND _immediate_deps ${_dep})
    endif()
  endforeach()

  # Link non-wangle deps immediately - they provide include paths needed at compile time
  if(_immediate_deps)
    target_link_libraries(${_obj_target} PUBLIC ${_immediate_deps})
  endif()

  # For shared builds: link Folly::folly and fizz::fizz to OBJECT libraries to get transitive
  # includes. We can't link wangle internal deps because they're INTERFACE libraries
  # linking to monolithic wangle, creating cycles.
  if(BUILD_SHARED_LIBS)
    target_link_libraries(${_obj_target} PUBLIC Folly::folly fizz::fizz)
  endif()

  # Defer internal wangle dependencies until all targets are created
  # Only for static builds - in shared builds, wangle internal deps are INTERFACE
  # libraries linking to monolithic wangle, which would create cycles
  if(NOT BUILD_SHARED_LIBS)
    if(_wangle_deps)
      list(JOIN _wangle_deps "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY WANGLE_DEFERRED_DEPS
        "${_obj_target}|PUBLIC|${_deps_str}"
      )
    endif()
    if(WANGLE_LIB_DEPS)
      list(JOIN WANGLE_LIB_DEPS "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY WANGLE_DEFERRED_DEPS
        "${_obj_target}|PRIVATE|${_deps_str}"
      )
    endif()
  endif()

  # Track OBJECT target for monolithic aggregation
  set_property(GLOBAL APPEND PROPERTY WANGLE_COMPONENT_TARGETS ${_obj_target})

  # 2. Create the granular library target
  if(BUILD_SHARED_LIBS)
    # For shared builds: create INTERFACE library that will link to monolithic wangle
    add_library(${_target_name} INTERFACE)

    target_include_directories(${_target_name}
      INTERFACE
        $<BUILD_INTERFACE:${WANGLE_BASE_DIR}>
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
    )

    # Track this target to link to wangle after monolithic library is created
    set_property(GLOBAL APPEND PROPERTY WANGLE_GRANULAR_INTERFACE_TARGETS ${_target_name})

    install(TARGETS ${_target_name} EXPORT wangle-exports)
  else()
    # For static builds: create STATIC library
    add_library(${_target_name} STATIC $<TARGET_OBJECTS:${_obj_target}>)

    target_include_directories(${_target_name}
      PUBLIC
        $<BUILD_INTERFACE:${WANGLE_BASE_DIR}>
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
        ${FOLLY_INCLUDE_DIR}
        ${FIZZ_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${GLOG_INCLUDE_DIRS}
        ${GFLAGS_INCLUDE_DIRS}
        ${LIBEVENT_INCLUDE_DIR}
        ${DOUBLE_CONVERSION_INCLUDE_DIR}
    )

    target_compile_features(${_target_name} PUBLIC cxx_std_17)

    # Link external dependencies on STATIC library
    target_link_libraries(${_target_name}
      PUBLIC
        ${OPENSSL_LIBRARIES}
        Threads::Threads
      PRIVATE
        ${GLOG_LIBRARIES}
        ${GFLAGS_LIBRARIES}
        ${LIBEVENT_LIB}
        ${DOUBLE_CONVERSION_LIBRARY}
        ${CMAKE_DL_LIBS}
        ${LIBRT_LIBRARIES}
    )

    # Link non-wangle deps immediately (reuse _immediate_deps computed above)
    if(_immediate_deps)
      target_link_libraries(${_target_name} PUBLIC ${_immediate_deps})
    endif()

    # Defer internal wangle dependencies for STATIC library too (reuse _wangle_deps)
    if(_wangle_deps)
      list(JOIN _wangle_deps "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY WANGLE_DEFERRED_DEPS
        "${_target_name}|PUBLIC|${_deps_str}"
      )
    endif()
    if(WANGLE_LIB_DEPS)
      list(JOIN WANGLE_LIB_DEPS "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY WANGLE_DEFERRED_DEPS
        "${_target_name}|PRIVATE|${_deps_str}"
      )
    endif()

    install(
      TARGETS ${_target_name}
      EXPORT wangle-exports
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    )
  endif()

  # Create alias for the library
  add_library(wangle::${_target_name} ALIAS ${_target_name})
endfunction()

# Create the monolithic wangle library from all component OBJECT libraries
# Call this after all add_subdirectory() calls, before wangle_resolve_deferred_dependencies()
function(wangle_create_monolithic_library)
  get_property(_component_targets GLOBAL PROPERTY WANGLE_COMPONENT_TARGETS)

  if(NOT _component_targets)
    message(STATUS "No component targets found, skipping monolithic library creation")
    return()
  endif()

  # Collect all object files from component targets
  set(_all_objects)
  foreach(_target IN LISTS _component_targets)
    list(APPEND _all_objects $<TARGET_OBJECTS:${_target}>)
  endforeach()

  # Create the monolithic library
  add_library(wangle ${_all_objects})

  if(BUILD_SHARED_LIBS)
    set_property(TARGET wangle PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET wangle PROPERTY VERSION ${PACKAGE_VERSION})
  endif()

  target_include_directories(wangle
    PUBLIC
      $<BUILD_INTERFACE:${WANGLE_BASE_DIR}>
      $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
      ${FOLLY_INCLUDE_DIR}
      ${FIZZ_INCLUDE_DIR}
      ${OPENSSL_INCLUDE_DIR}
      ${GLOG_INCLUDE_DIRS}
      ${GFLAGS_INCLUDE_DIRS}
      ${LIBEVENT_INCLUDE_DIR}
      ${DOUBLE_CONVERSION_INCLUDE_DIR}
  )

  target_compile_features(wangle PUBLIC cxx_std_17)

  # Link all dependencies
  target_link_libraries(wangle
    PUBLIC
      Folly::folly
      fizz::fizz
      ${OPENSSL_LIBRARIES}
      Threads::Threads
    PRIVATE
      ${GLOG_LIBRARIES}
      ${GFLAGS_LIBRARIES}
      ${LIBEVENT_LIB}
      ${DOUBLE_CONVERSION_LIBRARY}
      ${CMAKE_DL_LIBS}
      ${LIBRT_LIBRARIES}
  )

  # Create alias for consistency
  add_library(wangle::wangle ALIAS wangle)

  # For shared builds: link all granular INTERFACE targets to the monolithic library
  if(BUILD_SHARED_LIBS)
    cmake_policy(SET CMP0079 NEW)
    get_property(_interface_targets GLOBAL PROPERTY WANGLE_GRANULAR_INTERFACE_TARGETS)
    foreach(_target IN LISTS _interface_targets)
      target_link_libraries(${_target} INTERFACE wangle)
    endforeach()
  endif()
endfunction()

# Resolve all deferred dependencies after all targets have been created
# Call this after all add_subdirectory() calls and wangle_create_monolithic_library()
function(wangle_resolve_deferred_dependencies)
  # Allow linking targets defined in other directories
  cmake_policy(SET CMP0079 NEW)

  get_property(_deferred_deps GLOBAL PROPERTY WANGLE_DEFERRED_DEPS)

  foreach(_spec IN LISTS _deferred_deps)
    # Parse the spec: "target|visibility|dep1,dep2,..."
    string(REPLACE "|" ";" _parts "${_spec}")
    list(LENGTH _parts _len)
    if(_len LESS 3)
      continue()
    endif()

    list(GET _parts 0 _target)
    list(GET _parts 1 _visibility)
    list(GET _parts 2 _deps_str)

    # Split deps by comma
    string(REPLACE "," ";" _deps "${_deps_str}")

    # Filter to only existing targets (skip deps that weren't generated)
    set(_valid_deps "")
    foreach(_dep IN LISTS _deps)
      if(TARGET ${_dep})
        list(APPEND _valid_deps ${_dep})
      endif()
    endforeach()

    if(_valid_deps)
      target_link_libraries(${_target} ${_visibility} ${_valid_deps})
    endif()
  endforeach()
endfunction()
