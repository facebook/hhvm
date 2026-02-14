# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Initialize global properties for tracking targets and deferred dependencies
set_property(GLOBAL PROPERTY PROXYGEN_COMPONENT_TARGETS)
set_property(GLOBAL PROPERTY PROXYGEN_DEFERRED_DEPS)
set_property(GLOBAL PROPERTY PROXYGEN_GRANULAR_INTERFACE_TARGETS)

# Define a granular proxygen library that:
# 1. Compiles sources ONCE via OBJECT library
# 2. Creates a STATIC library for individual linking (static builds)
# 3. Creates an INTERFACE library linking to monolithic proxygen (shared builds)
# 4. Defers internal proxygen deps to be resolved later
# 5. Tracks OBJECT target for monolithic aggregation
# 6. Creates proxygen:: namespace alias
#
# Usage:
#   proxygen_add_library(proxygen_http_message
#     SRCS HTTPMessage.cpp
#     DEPS proxygen_http_headers         # Private dependencies
#     EXPORTED_DEPS Folly::folly_io_iobuf  # Public dependencies (propagated)
#     EXCLUDE_FROM_MONOLITH              # Don't include in monolithic proxygen library
#   )
function(proxygen_add_library _target_name)
  cmake_parse_arguments(
    PROXYGEN_LIB
    "EXCLUDE_FROM_MONOLITH"         # Options (boolean flags)
    ""                              # Single-value args
    "SRCS;DEPS;EXPORTED_DEPS"       # Multi-value args
    ${ARGN}
  )

  set(_sources ${PROXYGEN_LIB_SRCS})
  if(NOT _sources)
    # Legacy support: if no SRCS keyword, treat remaining args as sources
    set(_sources ${PROXYGEN_LIB_UNPARSED_ARGUMENTS})
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
        $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
        $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
        $<INSTALL_INTERFACE:include/>
    )

    # Link exported deps for INTERFACE libraries
    if(PROXYGEN_LIB_EXPORTED_DEPS)
      target_link_libraries(${_target_name} INTERFACE ${PROXYGEN_LIB_EXPORTED_DEPS})
    endif()

    install(TARGETS ${_target_name} EXPORT proxygen-exports)
    add_library(proxygen::${_target_name} ALIAS ${_target_name})
    return()
  endif()

  # 1. Create OBJECT library (compiles sources once)
  add_library(${_obj_target} OBJECT ${_sources})

  # Ensure generated headers are built before any proxygen sources
  # This is needed because transitive deps don't carry build-order dependencies
  if(TARGET proxygen-generated)
    add_dependencies(${_obj_target} proxygen-generated)
  endif()

  if(DEFINED PACKAGE_VERSION)
    set_property(TARGET ${_obj_target} PROPERTY VERSION ${PACKAGE_VERSION})
  endif()

  if(BUILD_SHARED_LIBS)
    set_property(TARGET ${_obj_target} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  target_include_directories(${_obj_target}
    PUBLIC
      $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
      $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
      $<INSTALL_INTERFACE:include/>
  )

  target_compile_options(${_obj_target}
    PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
  )

  target_compile_features(${_obj_target} PUBLIC cxx_std_20)

  # Separate proxygen internal deps (defer) from external deps (link immediately)
  # Also separate utility deps (like proxygen-generated) that need add_dependencies
  set(_immediate_deps "")
  set(_proxygen_deps "")
  set(_utility_deps "")
  foreach(_dep IN LISTS PROXYGEN_LIB_EXPORTED_DEPS)
    if(_dep STREQUAL "proxygen-generated")
      # Utility target - use add_dependencies, not target_link_libraries
      list(APPEND _utility_deps ${_dep})
    elseif(_dep MATCHES "^proxygen_")
      list(APPEND _proxygen_deps ${_dep})
    else()
      # Folly::*, fizz::*, mvfst::*, wangle::*, external libs - link immediately
      list(APPEND _immediate_deps ${_dep})
    endif()
  endforeach()

  # Add build-order dependencies on utility targets (like proxygen-generated)
  if(_utility_deps)
    add_dependencies(${_obj_target} ${_utility_deps})
  endif()

  # Link non-proxygen deps immediately - they provide include paths needed at compile time
  if(_immediate_deps)
    target_link_libraries(${_obj_target} PUBLIC ${_immediate_deps})
  endif()

  # For shared builds: link Folly::folly, fizz::fizz, wangle::wangle to OBJECT libraries
  # to get transitive includes
  if(BUILD_SHARED_LIBS)
    target_link_libraries(${_obj_target} PUBLIC Folly::folly fizz::fizz wangle::wangle)
  endif()

  # Defer internal proxygen dependencies until all targets are created
  # This is needed for both static and shared builds because:
  # - Static: avoid circular dependencies during library creation
  # - Shared: OBJECT libraries need include paths from deps that may not exist yet
  if(_proxygen_deps)
    list(JOIN _proxygen_deps "," _deps_str)
    set_property(GLOBAL APPEND PROPERTY PROXYGEN_DEFERRED_DEPS
      "${_obj_target}|PUBLIC|${_deps_str}"
    )
  endif()
  if(PROXYGEN_LIB_DEPS)
    # Separate proxygen internal deps (defer) from external deps (link immediately)
    set(_private_immediate_deps "")
    set(_private_proxygen_deps "")
    foreach(_dep IN LISTS PROXYGEN_LIB_DEPS)
      if(_dep MATCHES "^proxygen_")
        list(APPEND _private_proxygen_deps ${_dep})
      else()
        list(APPEND _private_immediate_deps ${_dep})
      endif()
    endforeach()

    if(_private_immediate_deps)
      target_link_libraries(${_obj_target} PRIVATE ${_private_immediate_deps})
    endif()

    if(_private_proxygen_deps)
      list(JOIN _private_proxygen_deps "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY PROXYGEN_DEFERRED_DEPS
        "${_obj_target}|PRIVATE|${_deps_str}"
      )
    endif()
  endif()

  # Track OBJECT target for monolithic aggregation (unless excluded)
  if(NOT PROXYGEN_LIB_EXCLUDE_FROM_MONOLITH)
    set_property(GLOBAL APPEND PROPERTY PROXYGEN_COMPONENT_TARGETS ${_obj_target})
  endif()

  # 2. Create the granular library target
  if(BUILD_SHARED_LIBS AND NOT PROXYGEN_LIB_EXCLUDE_FROM_MONOLITH)
    # For shared builds: create INTERFACE library that will link to monolithic proxygen
    add_library(${_target_name} INTERFACE)

    target_include_directories(${_target_name}
      INTERFACE
        $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
        $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
        $<INSTALL_INTERFACE:include/>
    )

    # Track this target to link to proxygen after monolithic library is created
    set_property(GLOBAL APPEND PROPERTY PROXYGEN_GRANULAR_INTERFACE_TARGETS ${_target_name})

    install(TARGETS ${_target_name} EXPORT proxygen-exports)
  elseif(BUILD_SHARED_LIBS AND PROXYGEN_LIB_EXCLUDE_FROM_MONOLITH)
    # For excluded targets in shared builds: create SHARED library with actual code
    # These are NOT in the monolithic proxygen, so they need their own implementation
    add_library(${_target_name} SHARED $<TARGET_OBJECTS:${_obj_target}>)

    if(DEFINED PACKAGE_VERSION)
      set_property(TARGET ${_target_name} PROPERTY VERSION ${PACKAGE_VERSION})
    endif()

    target_include_directories(${_target_name}
      PUBLIC
        $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
        $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
        $<INSTALL_INTERFACE:include/>
    )

    target_compile_features(${_target_name} PUBLIC cxx_std_20)

    # Add build-order dependencies on utility targets
    if(_utility_deps)
      add_dependencies(${_target_name} ${_utility_deps})
    endif()

    # Link non-proxygen deps immediately
    if(_immediate_deps)
      target_link_libraries(${_target_name} PUBLIC ${_immediate_deps})
    endif()

    # Defer internal proxygen dependencies
    if(_proxygen_deps)
      list(JOIN _proxygen_deps "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY PROXYGEN_DEFERRED_DEPS
        "${_target_name}|PUBLIC|${_deps_str}"
      )
    endif()
    if(PROXYGEN_LIB_DEPS)
      set(_priv_imm "")
      set(_priv_prox "")
      foreach(_dep IN LISTS PROXYGEN_LIB_DEPS)
        if(_dep MATCHES "^proxygen_")
          list(APPEND _priv_prox ${_dep})
        else()
          list(APPEND _priv_imm ${_dep})
        endif()
      endforeach()
      if(_priv_imm)
        target_link_libraries(${_target_name} PRIVATE ${_priv_imm})
      endif()
      if(_priv_prox)
        list(JOIN _priv_prox "," _deps_str)
        set_property(GLOBAL APPEND PROPERTY PROXYGEN_DEFERRED_DEPS
          "${_target_name}|PRIVATE|${_deps_str}"
        )
      endif()
    endif()

    install(
      TARGETS ${_target_name}
      EXPORT proxygen-exports
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    )
  else()
    # For static builds: create STATIC library
    add_library(${_target_name} STATIC $<TARGET_OBJECTS:${_obj_target}>)

    if(DEFINED PACKAGE_VERSION)
      set_property(TARGET ${_target_name} PROPERTY VERSION ${PACKAGE_VERSION})
    endif()

    target_include_directories(${_target_name}
      PUBLIC
        $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
        $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
        $<INSTALL_INTERFACE:include/>
    )

    target_compile_features(${_target_name} PUBLIC cxx_std_20)

    # Add build-order dependencies on utility targets (reuse _utility_deps from above)
    if(_utility_deps)
      add_dependencies(${_target_name} ${_utility_deps})
    endif()

    # Link non-proxygen deps immediately (reuse _immediate_deps computed above)
    if(_immediate_deps)
      target_link_libraries(${_target_name} PUBLIC ${_immediate_deps})
    endif()

    # Defer internal proxygen dependencies for STATIC library too (reuse _proxygen_deps)
    if(_proxygen_deps)
      list(JOIN _proxygen_deps "," _deps_str)
      set_property(GLOBAL APPEND PROPERTY PROXYGEN_DEFERRED_DEPS
        "${_target_name}|PUBLIC|${_deps_str}"
      )
    endif()
    if(PROXYGEN_LIB_DEPS)
      set(_priv_imm "")
      set(_priv_prox "")
      foreach(_dep IN LISTS PROXYGEN_LIB_DEPS)
        if(_dep MATCHES "^proxygen_")
          list(APPEND _priv_prox ${_dep})
        else()
          list(APPEND _priv_imm ${_dep})
        endif()
      endforeach()
      if(_priv_imm)
        target_link_libraries(${_target_name} PRIVATE ${_priv_imm})
      endif()
      if(_priv_prox)
        list(JOIN _priv_prox "," _deps_str)
        set_property(GLOBAL APPEND PROPERTY PROXYGEN_DEFERRED_DEPS
          "${_target_name}|PRIVATE|${_deps_str}"
        )
      endif()
    endif()

    install(
      TARGETS ${_target_name}
      EXPORT proxygen-exports
      LIBRARY DESTINATION ${LIB_INSTALL_DIR}
      ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    )
  endif()

  # Create alias for the library
  add_library(proxygen::${_target_name} ALIAS ${_target_name})
endfunction()

# Create a backwards-compatible alias target
# This creates an INTERFACE library with the old name that links to the new target
function(proxygen_add_compat_alias _old_name _new_name)
  if(NOT TARGET ${_new_name})
    message(WARNING "Cannot create compat alias ${_old_name}: target ${_new_name} does not exist")
    return()
  endif()

  add_library(${_old_name} INTERFACE)
  target_link_libraries(${_old_name} INTERFACE ${_new_name})
  install(TARGETS ${_old_name} EXPORT proxygen-exports)
  add_library(proxygen::${_old_name} ALIAS ${_old_name})
endfunction()

# Create the monolithic proxygen library from all component OBJECT libraries
# Call this after all add_subdirectory() calls, before proxygen_resolve_deferred_dependencies()
function(proxygen_create_monolithic_library)
  get_property(_component_targets GLOBAL PROPERTY PROXYGEN_COMPONENT_TARGETS)

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
  add_library(proxygen ${_all_objects})

  if(BUILD_SHARED_LIBS)
    set_property(TARGET proxygen PROPERTY POSITION_INDEPENDENT_CODE ON)
    if(DEFINED PACKAGE_VERSION)
      set_target_properties(proxygen PROPERTIES VERSION ${PACKAGE_VERSION})
    endif()
  endif()

  target_include_directories(proxygen
    PUBLIC
      $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
      $<BUILD_INTERFACE:${PROXYGEN_GENERATED_ROOT}>
      $<INSTALL_INTERFACE:include/>
  )

  target_compile_features(proxygen PUBLIC cxx_std_20)

  # Link all dependencies
  target_link_libraries(proxygen
    PUBLIC
      Folly::folly
      fizz::fizz
      wangle::wangle
      ${ZSTD_LIBRARIES}
      ZLIB::ZLIB
      ${OPENSSL_LIBRARIES}
      Threads::Threads
      c-ares::cares
    PRIVATE
      glog::glog
      ${GFLAG_DEPENDENCIES}
      ${CMAKE_DL_LIBS}
  )

  # Create alias for consistency
  add_library(proxygen::proxygen ALIAS proxygen)

  # For shared builds: link all granular INTERFACE targets to the monolithic library
  if(BUILD_SHARED_LIBS)
    cmake_policy(SET CMP0079 NEW)
    get_property(_interface_targets GLOBAL PROPERTY PROXYGEN_GRANULAR_INTERFACE_TARGETS)
    foreach(_target IN LISTS _interface_targets)
      target_link_libraries(${_target} INTERFACE proxygen)
    endforeach()
  endif()
endfunction()

# Resolve all deferred dependencies after all targets have been created
# Call this after all add_subdirectory() calls
function(proxygen_resolve_deferred_dependencies)
  # Allow linking targets defined in other directories
  cmake_policy(SET CMP0079 NEW)

  # For shared builds: link all granular INTERFACE targets to the monolithic library
  # This is needed because proxygen_add_library creates INTERFACE libraries for shared builds
  # that need to link to the monolithic proxygen target
  if(BUILD_SHARED_LIBS)
    get_property(_interface_targets GLOBAL PROPERTY PROXYGEN_GRANULAR_INTERFACE_TARGETS)
    foreach(_target IN LISTS _interface_targets)
      if(TARGET ${_target} AND TARGET proxygen)
        target_link_libraries(${_target} INTERFACE proxygen)
      endif()
    endforeach()
  endif()

  get_property(_deferred_deps GLOBAL PROPERTY PROXYGEN_DEFERRED_DEPS)

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
    # For shared builds and OBJECT targets, prefer linking to _obj version for include paths
    set(_valid_deps "")
    foreach(_dep IN LISTS _deps)
      # For shared builds: if target is an OBJECT library (*_obj) and dep has an _obj version,
      # link to _obj for include path propagation
      if(BUILD_SHARED_LIBS AND _target MATCHES "_obj$" AND TARGET ${_dep}_obj)
        list(APPEND _valid_deps ${_dep}_obj)
      elseif(TARGET ${_dep})
        list(APPEND _valid_deps ${_dep})
      endif()
    endforeach()

    if(_valid_deps)
      target_link_libraries(${_target} ${_visibility} ${_valid_deps})
    endif()
  endforeach()
endfunction()

# =============================================================================
# Header installation function
# =============================================================================
# Install headers preserving directory structure relative to rootDir
# Usage: proxygen_install_headers(proxygen ${CMAKE_CURRENT_SOURCE_DIR} ${HEADERS})
function(proxygen_install_headers rootName rootDir)
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
