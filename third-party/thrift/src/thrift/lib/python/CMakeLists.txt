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

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# FOLLY_INCLUDE_DIR is exported by folly-config.cmake (set by find_package)
# Used by setup.py to locate folly .pxd files during Cython compilation
message(STATUS "FOLLY_INCLUDE_DIR: ${FOLLY_INCLUDE_DIR}")

# Set FOLLY_PYTHON_SITE_PACKAGES for Cython .pxd files
# (folly.pxd, iobuf.pxd, etc.)
# These are separate from C++ headers and live in Python site-packages
#
# find_file locates a known file, then extract the parent directory.
# HINTS cover both install locations:
#   - lib/pythonX.Y/site-packages/ (setup.py install)
#   - local/lib/pythonX.Y/dist-packages/ (pip --prefix on Debian/Ubuntu)
set(_folly_py_version "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
find_file(_folly_pxd_file
  NAMES folly/__init__.pxd
  HINTS
    "${FOLLY_PREFIX_DIR}/lib/${_folly_py_version}/site-packages"
    "${FOLLY_PREFIX_DIR}/local/lib/${_folly_py_version}/dist-packages"
  NO_DEFAULT_PATH
  REQUIRED
)

# Extract site-packages directory (two levels up from folly/__init__.pxd)
get_filename_component(_folly_pkg_dir "${_folly_pxd_file}" DIRECTORY)
get_filename_component(FOLLY_PYTHON_SITE_PACKAGES "${_folly_pkg_dir}" DIRECTORY)
message(STATUS "FOLLY_PYTHON_SITE_PACKAGES: ${FOLLY_PYTHON_SITE_PACKAGES}")


find_package(Cython 0.28 REQUIRED)
find_package(proxygen CONFIG REQUIRED)


# thrift-python's server and streaming modules depend architecturally on py3's
# infrastructure. The py3 code provides:
#   - Streaming support (stream)
#   - Core server infrastructure (server)
# thrift-python builds on top of these py3 components rather than duplicating them.
# They cannot be separated into independent add_subdirectory() calls.

# Reference to parent lib directory for sibling directories (py3/) and setup.py
get_filename_component(_lib_dir "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
# Debug option for build troubleshooting
option(DEBUG_BUILD "Enable debug messages" OFF)

# =============================================================================
# Explicit Dependencies for Self-Contained Wheel
# =============================================================================
#
# Goal: Build a self-contained Python wheel that installs with just
# `pip install thrift-*.whl` - no additional dependencies required.
#
# Challenge: A standard shared library build would require wheel consumers to:
#   - Install 20+ shared libraries (folly, fizz, wangle, etc.) separately
#   - Manage LD_LIBRARY_PATH or install to system paths
#   - Handle version compatibility across all dependencies
#
# Approach: Create a consolidated library (libthrift_python_cpp.so) that embeds
# all C++ dependencies into one shared object. auditwheel bundles this single
# .so into the wheel.
#
# Why --whole-archive?
#   Without it, each Python extension (.so) would embed the full static library:
#     types.so (~600MB) + client.so (~600MB) + server.so (~600MB) + ...
#     = 27+ extensions × 600MB = 16-32GB of duplicated code
#
#   With --whole-archive, one shared library holds everything:
#     libthrift_python_cpp.so (~600MB) + tiny extensions (~50KB each)
#     = ~600MB total
#
# Trade-off: --whole-archive is a linker flag, not a CMake concept. CMake
# propagates transitive dependencies via INTERFACE_LINK_LIBRARIES on targets.
# But linker flags bypass this mechanism. When CMake sees:
#
#   target_link_libraries(thrift_python_cpp "-Wl,--whole-archive" Folly::folly ...)
#
# it passes the flag to the linker, but the linker does not understand CMake's
# target properties. Transitive deps like libiberty (from Folly::folly) do not
# propagate. Without --whole-archive, CMake would automatically add libiberty
# to the link line. With --whole-archive, the find_package calls below must add
# them explicitly.
# =============================================================================

# Libevent: Force static link to embed symbols
set(LIBEVENT_STATIC_LINK TRUE)
find_package(Libevent REQUIRED CONFIG)

# Libiberty: Provides cplus_demangle_v3_callback for folly stack traces
find_package(Libiberty REQUIRED)

# gflags warning:
# This section adds explicit dependencies that --whole-archive bypasses.
# A natural assumption: if libevent and libiberty are here, gflags should be too.
# gflags is different. It's a transitive dependency (glog depends on gflags).
# setup.py links glog and gflags as shared libraries. A find_package(gflags) call
# here would embed static gflags symbols into thrift_python_cpp.so. At runtime,
# glog loads shared gflags, and duplicate symbols conflict. Do not add gflags.

function(log_debug msg)
  if(DEBUG_BUILD)
    message(STATUS "[DEBUG] ${msg}")
  endif()
endfunction()

# Use parent binary dir so _cybld stays at <build>/thrift/lib/cybld
# (consistent with include paths like thrift/lib/python/server/*.h)
#
# Why symlinks?
# Buck has a "base_module" concept that changes import paths. Buck handles this
# automatically. This build achieves the same explicitly via symlinks from source
# files to the build directory with the desired import path structure.
# Example: thrift/lib/python with base_module="thrift.python" becomes import
# thrift.python. Symlinks map thrift/lib/python -> thrift/python under _cybld.
get_filename_component(_cybld "${CMAKE_CURRENT_BINARY_DIR}/../cybld" ABSOLUTE)

# Create build directory structure
set(_cybld_dirs
  "thrift/python"
  "thrift/python/client"
  "thrift/python/server"
  "thrift/python/server/flagged"
  "thrift/python/server/interceptor"
  "thrift/python/any"
  "thrift/python/conformance"
  "thrift/python/streaming"
  "thrift/py3"
)
foreach(_dir ${_cybld_dirs})
  file(MAKE_DIRECTORY "${_cybld}/${_dir}")
endforeach()

# So that cython includes work correctly
file(TOUCH "${_cybld}/thrift/__init__.pxd")
file(TOUCH "${_cybld}/thrift/python/__init__.pxd")
file(TOUCH "${_cybld}/thrift/py3/__init__.pxd")

# Helper function: symlink all source files from a directory to build dir
# Non-recursive - only files directly in the directory
# (subdirs handled separately)
function(symlink_dir_contents src_dir dest_dir)
  file(MAKE_DIRECTORY "${dest_dir}")
  file(GLOB _all_items "${src_dir}/*")

  foreach(_src ${_all_items})
    if(IS_DIRECTORY "${_src}")
      continue()
    endif()

    get_filename_component(_filename "${_src}" NAME)
    set(_dest "${dest_dir}/${_filename}")

    message(STATUS "Symlinking ${_src} -> ${_dest}")
    file(CREATE_LINK "${_src}" "${_dest}" SYMBOLIC)
  endforeach()
endfunction()

# Helper function: recursively symlink all files from a source tree
# to build directory. Calls symlink_dir_contents for each directory.
function(symlink_dir_recursive src_dir dest_base_dir)
  # Handle the root directory
  symlink_dir_contents("${src_dir}" "${dest_base_dir}")

  # Find all subdirectories and call symlink_dir_contents for each
  file(GLOB_RECURSE _all_subdirs LIST_DIRECTORIES true
    "${src_dir}/*")

  foreach(_src_dir ${_all_subdirs})
    if(IS_DIRECTORY "${_src_dir}")
      file(RELATIVE_PATH _rel "${src_dir}" "${_src_dir}")
      symlink_dir_contents(
        "${src_dir}/${_rel}" "${dest_base_dir}/${_rel}")
    endif()
  endforeach()
endfunction()

# Symlink all python/ and py3/ source files recursively
symlink_dir_recursive("${CMAKE_CURRENT_SOURCE_DIR}" "${_cybld}/thrift/python")
symlink_dir_recursive("${_lib_dir}/py3" "${_cybld}/thrift/py3")

# Placeholder targets for build dependency tracking
# Symlinks are created at configure time by symlink_dir_recursive above,
# but other parts of the build depend on these targets for ordering.
add_custom_target(create_binding_symlink_python ALL)
add_custom_target(create_binding_symlink_py3 ALL)

#####################################################################
# Include Path Setup (must be done early, before API header generation)
#####################################################################
get_target_property(thrift_include_dirs thrift INCLUDE_DIRECTORIES)
get_target_property(fmt_include_dirs fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(wangle_incs wangle::wangle INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(
  glog_include_dirs glog::glog INTERFACE_INCLUDE_DIRECTORIES)

get_filename_component(python_libname ${Python3_LIBRARIES} NAME)

# Create colon-separated include path for CYTHON_INCLUDE_PATH env var
# Proxygen and mvfst include directories (for http2_helper in client tests)
# Derive from cmake targets (found via find_package above) - not hardcoded paths
get_target_property(
  proxygen_incs proxygen::proxygen INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(
  mvfst_incs mvfst::mvfst_transport INTERFACE_INCLUDE_DIRECTORIES)

set(_all_cython_includes
  # Thrift includes
  ${thrift_include_dirs}
  ${CMAKE_BINARY_DIR}/thrift/lib
  ${CMAKE_BINARY_DIR}/thrift/lib/cybld
  # Folly includes (including .pxd files)
  ${FOLLY_INCLUDE_DIR}
  ${FOLLY_PYTHON_SITE_PACKAGES}
  # Boost
  ${Boost_INCLUDE_DIRS}
  # Other dependencies
  ${fmt_include_dirs}
  ${wangle_incs}
  ${glog_include_dirs}
  # Proxygen and mvfst (for http2_helper.pyx client tests)
  ${proxygen_incs}
  ${mvfst_incs}
)
list(JOIN _all_cython_includes ":" cython_include_path)
log_debug("cython_include_path='${cython_include_path}'")

#####################################################################
# Library Path Setup
#####################################################################
# Auto-discover all library directories from getdeps installed packages
# This ensures setup.py's find_static_lib() can find ALL libraries
# in static_lib_names. Use GETDEPS_INSTALL_DIR env var if set (getdeps
# always sets this), otherwise fall back to deriving from
# CMAKE_INSTALL_PREFIX (works when CMAKE_INSTALL_PREFIX is in the
# getdeps installed directory, but NOT when --project-install-prefix
# overrides it)
if(DEFINED ENV{GETDEPS_INSTALL_DIR})
  set(GETDEPS_INSTALL_ROOT "$ENV{GETDEPS_INSTALL_DIR}")
else()
  get_filename_component(
    GETDEPS_INSTALL_ROOT "${CMAKE_INSTALL_PREFIX}/.." ABSOLUTE)
endif()
file(GLOB installed_packages "${GETDEPS_INSTALL_ROOT}/*")
set(all_lib_dirs "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
foreach(pkg ${installed_packages})
  if(EXISTS "${pkg}/lib")
    list(APPEND all_lib_dirs "${pkg}/lib")
  endif()
  if(EXISTS "${pkg}/lib64")
    list(APPEND all_lib_dirs "${pkg}/lib64")
  endif()
endforeach()
list(APPEND all_lib_dirs "/usr/lib64")
get_filename_component(python_lib ${Python3_LIBRARIES} DIRECTORY)
list(APPEND all_lib_dirs "${python_lib}")

# Join all library directories with colons for -L flags and LIBRARY_DIRS
list(JOIN all_lib_dirs ":" library_dirs_str)
set(libs "-L${library_dirs_str}")

#####################################################################
# Install Directory
#####################################################################
set(py_install_dir ${CMAKE_INSTALL_PREFIX})
if (NOT ${PYTHON_PACKAGE_INSTALL_DIR} STREQUAL "")
  set(py_install_dir ${PYTHON_PACKAGE_INSTALL_DIR})
endif()

#####################################################################
# Linker Flags
#####################################################################
# Set default LDFLAGS for setup.py to fix undefined symbol issues
# --no-as-needed ensures libfolly.so is in NEEDED list even if linker
# thinks it's not needed
set(DEFAULT_SETUP_LDFLAGS "-Wl,--no-as-needed")

# Allow users to override via environment variable (e.g., for custom RPATH)
if(DEFINED ENV{LDFLAGS})
  set(SETUP_LDFLAGS "$ENV{LDFLAGS}")
  message(STATUS "Using user-provided LDFLAGS: ${SETUP_LDFLAGS}")
else()
  set(SETUP_LDFLAGS "${DEFAULT_SETUP_LDFLAGS}")
  message(STATUS "Using default LDFLAGS: ${SETUP_LDFLAGS}")
endif()

###
# First, run Thrift compiler on metadata.thrift. thrift-python runtime
# depends on apache.thrift.metadata package, which is created and
# installed later in setup.py.
###
add_custom_target(
  generate_apache_thrift_metadata_python
  COMMAND
    thrift1 -I ${CMAKE_SOURCE_DIR} --out .
    --gen "mstch_python:include_prefix=thrift"
    ${_lib_dir}/thrift/metadata.thrift
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_cybld}/apache/thrift/metadata
  WORKING_DIRECTORY ${_cybld}
  BYPRODUCTS
    ${_cybld}/apache/thrift/metadata/thrift_abstract_types.py
    ${_cybld}/apache/thrift/metadata/thrift_clients.py
    ${_cybld}/apache/thrift/metadata/thrift_enums.py
    ${_cybld}/apache/thrift/metadata/thrift_metadata.py
    ${_cybld}/apache/thrift/metadata/thrift_mutable_clients.py
    ${_cybld}/apache/thrift/metadata/thrift_mutable_services.py
    ${_cybld}/apache/thrift/metadata/thrift_mutable_types.py
    ${_cybld}/apache/thrift/metadata/thrift_mutable_types.pyi
    ${_cybld}/apache/thrift/metadata/thrift_services.py
    ${_cybld}/apache/thrift/metadata/thrift_types.py
    ${_cybld}/apache/thrift/metadata/thrift_types.pyi
)

###
# Also generate py3 version for apache.thrift.metadata
# (required by py3/metadata.pyx)
###
add_custom_target(
  generate_apache_thrift_metadata_py3
  COMMAND
    thrift1 -I ${CMAKE_SOURCE_DIR} --out .
    --gen "mstch_py3:include_prefix=thrift"
    ${_lib_dir}/thrift/metadata.thrift
  WORKING_DIRECTORY ${_cybld}
  BYPRODUCTS
    ${_cybld}/apache/thrift/metadata/cbindings.pxd
    ${_cybld}/apache/thrift/metadata/cbindings.pyx
    ${_cybld}/apache/thrift/metadata/metadata.pxd
    ${_cybld}/apache/thrift/metadata/metadata.pyx
)

####
# Next, prepare for building cython extensions by creating the necessary
# symlinks in the cybld directory
####
add_custom_target(create_binding_symlink_python_types ALL)
# Python types files (local to this directory)
foreach(_file "types.pxd" "types.pyx")
  set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${_file}")
  get_filename_component(_target_file "${_src}" NAME)
  message(STATUS "Linking ${_src} to ${_cybld}/thrift/python/_${_target_file}")
  add_custom_command(
    TARGET create_binding_symlink_python_types PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      "${_src}" "${_cybld}/thrift/python/_${_target_file}"
  )
endforeach()
# py3 stream files (in sibling py3 directory)
foreach(_file "stream.pxd" "stream.pyx")
  set(_src "${_lib_dir}/py3/${_file}")
  get_filename_component(_target_file "${_src}" NAME)
  message(STATUS "Linking ${_src} to ${_cybld}/thrift/py3/_${_target_file}")
  add_custom_command(
    TARGET create_binding_symlink_python_types PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      "${_src}" "${_cybld}/thrift/py3/_${_target_file}"
  )
endforeach()
# Below symlinks simplify cythonize logic in setup.py
add_custom_command(
  TARGET
  create_binding_symlink_python_types
  PRE_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    "${_cybld}/thrift/python/server/server.pxd"
    "${_cybld}/thrift/python/server.pxd"
)
add_custom_command(
  TARGET
  create_binding_symlink_python_types
  PRE_BUILD
    COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    "${_cybld}/thrift/python/server/server.pyx"
    "${_cybld}/thrift/python/server.pyx"
)

#####
# Compile a few *_api.h headers first.
#####
# List all headers generated by setup.py --api-only
set(CYTHON_GENERATED_API_HEADERS
  ${_cybld}/thrift/python/_types_api.h
  ${_cybld}/thrift/python/server/python_async_processor_api.h
  ${_cybld}/thrift/python/server/request_context_api.h
  ${_cybld}/thrift/python/server/interceptor/service_interceptor_api.h
  ${_cybld}/thrift/python/server_impl/python_async_processor_api.h
  ${_cybld}/thrift/python/server_impl/request_context_api.h
  ${_cybld}/thrift/python/server_impl/interceptor/service_interceptor_api.h
  ${_cybld}/thrift/py3/_stream_api.h
  ${_cybld}/thrift/python/streaming/sink_api.h
  ${_cybld}/thrift/python/streaming/bidistream_api.h
  ${_cybld}/thrift/python/streaming/py_promise_api.h
)

log_debug(
  "At API header generation: cython_include_path='${cython_include_path}'")

add_custom_command(
  OUTPUT ${CYTHON_GENERATED_API_HEADERS}
  COMMAND ${CMAKE_COMMAND} -E env
    "CFLAGS=${CMAKE_C_FLAGS}"
    "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    "CXX=${CMAKE_CXX_COMPILER}"
    "CYTHON_INCLUDE_PATH=${cython_include_path}"
    python3 ${_lib_dir}/setup.py -v --api-only
  DEPENDS
    create_binding_symlink_python_types
    create_binding_symlink_python
    create_binding_symlink_py3
    create_binding_symlink_server_impl
    thriftcpp2
  WORKING_DIRECTORY ${_cybld}
  COMMENT "Generating Cython API headers"
)

add_custom_target(
  thrift_python_types_bindings ALL
  DEPENDS ${CYTHON_GENERATED_API_HEADERS}
)

#####################################################################
# API Header Symlinks
#####################################################################
# WHY: C++ code includes headers as "thrift/lib/python/types_api.h"
#      but setup.py generates them at "thrift/python/_types_api.h"
#      Symlinks bridge this path difference.
#
# NOTE: server_impl in source paths (not server) because:
#       - C++ code expects
#         import_thrift__python__server_impl__python_async_processor
#       - Buck uses package = "thrift.python.server_impl"
#       - Must match the import function names in generated API headers

# Define all API header symlink mappings (source|destination)
set(CYTHON_API_SYMLINK_MAPPINGS
  "${_cybld}/thrift/python/_types_api.h|\
${_cybld}/thrift/lib/python/types_api.h"
  "${_cybld}/thrift/python/_types.h|${_cybld}/thrift/lib/python/_types.h"
  "${_cybld}/thrift/python/server_impl/python_async_processor_api.h|\
${_cybld}/thrift/lib/python/server/python_async_processor_api.h"
  "${_cybld}/thrift/python/server_impl/request_context_api.h|\
${_cybld}/thrift/lib/python/server/request_context_api.h"
  "${_cybld}/thrift/python/server_impl/interceptor/service_interceptor_api.h|\
${_cybld}/thrift/lib/python/server/interceptor/service_interceptor_api.h"
  "${_cybld}/thrift/py3/_stream_api.h|${_cybld}/thrift/lib/py3/stream_api.h"
  "${_cybld}/thrift/py3/_stream.h|${_cybld}/thrift/lib/py3/_stream.h"
  "${_cybld}/thrift/python/streaming/sink_api.h|\
${_cybld}/thrift/lib/python/streaming/sink_api.h"
  "${_cybld}/thrift/python/streaming/bidistream_api.h|\
${_cybld}/thrift/lib/python/streaming/bidistream_api.h"
  "${_cybld}/thrift/python/streaming/py_promise_api.h|\
${_cybld}/thrift/lib/python/streaming/py_promise_api.h"
)

# Generate symlink commands from mappings
set(CYTHON_API_SYMLINKS "")
set(SYMLINK_COMMANDS "")
foreach(mapping IN LISTS CYTHON_API_SYMLINK_MAPPINGS)
  string(REPLACE "|" ";" pair ${mapping})
  list(GET pair 0 source)
  list(GET pair 1 dest)
  list(APPEND CYTHON_API_SYMLINKS ${dest})
  list(APPEND SYMLINK_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${dest})
endforeach()

file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/")
file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/server")
file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/server/interceptor")
file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/streaming")
file(MAKE_DIRECTORY "${_cybld}/thrift/lib/py3/")

# Create all symlinks
# (OUTPUT ensures files exist before dependent targets build)
add_custom_command(
  OUTPUT ${CYTHON_API_SYMLINKS}
  ${SYMLINK_COMMANDS}
  DEPENDS ${CYTHON_GENERATED_API_HEADERS}
  COMMENT "Creating symlinks to Cython API headers"
)

# Target for dependency tracking
add_custom_target(
  create_cython_types_api_symlink ALL
  DEPENDS ${CYTHON_API_SYMLINKS}
)
# Explicit target-level dependency: symlinks wait for API headers
add_dependencies(create_cython_types_api_symlink thrift_python_types_bindings)

# SHARED to reduce build time, disk space, and runtime memory
add_library(
  thrift_python_cpp SHARED
    client/OmniClient.cpp
    client/RequestChannel.cpp
    client/ssl.cpp
    Serializer.cpp
    server/event_handler.cpp
    server/flagged/RcAwareTaskPatch.cpp
    server/interceptor/PythonServiceInterceptor.cpp
    server/PythonAsyncProcessor.cpp
    server/PythonAsyncProcessorFactory.cpp
    server/RequestContextExtractor.cpp
    std_libcpp.cpp
    streaming/bidi_stream.cpp
    streaming/PythonUserException.cpp
    streaming/Sink.cpp
    streaming/StreamElementEncoder.cpp
    types.cpp
    ${_lib_dir}/py3/stream.cpp
)
add_dependencies(thrift_python_cpp create_cython_types_api_symlink)
set_property(TARGET thrift_python_cpp PROPERTY VERSION ${PACKAGE_VERSION})
target_compile_definitions(thrift_python_cpp PRIVATE BOOST_NO_AUTO_PTR)
target_compile_definitions(
  thrift_python_cpp PRIVATE THRIFT_NO_HTTP_CLIENT_CHANNEL)
target_include_directories(
  thrift_python_cpp PRIVATE "${_cybld}" ${Python3_INCLUDE_DIRS})
# Link strategy:
# - STATIC libs (thrift + deps): embed with --whole-archive to consolidate
#   into single .so
# - SHARED libs (folly, fmt, glog): link normally, become runtime deps
# --allow-multiple-definition handles duplicate symbols across static libs
# (e.g., librpcmetadata.a and libthriftprotocol.a both compile
# TJSONProtocol.cpp)
# libevent/libiberty: needed for undefined symbols,
# --allow-multiple-definition handles duplicates
target_link_libraries(
  thrift_python_cpp
  PUBLIC
    # Static libs - embed all symbols
    "-Wl,--whole-archive"
    "-Wl,--allow-multiple-definition"
    thriftcpp2
    rpcmetadata
    thriftannotation
    thriftmetadata
    thriftfrozen2
    thriftprotocol
    thrifttyperep
    thrifttype
    thriftanyrep
    serverdbginfo
    libevent::core
    libevent::extra
    ${LIBIBERTY_LIBRARIES}
    "-Wl,--no-whole-archive"
    # Shared libs - runtime deps (see manifests/folly, manifests/fmt)
    Folly::folly
    Folly::folly_python_cpp
    fmt::fmt
    glog::glog
  )
install(
  TARGETS thrift_python_cpp
  EXPORT thrift
)
install(
  TARGETS thrift_python_cpp
  DESTINATION ${py_install_dir}
)

#####
# Create server_impl alias for OSS builds
# Buck internal builds use package = "thrift.python.server_impl" remapping.
# OSS CMake builds need this alias created manually.
#####
add_custom_target(create_binding_symlink_server_impl ALL)

file(MAKE_DIRECTORY "${_cybld}/thrift/python/server_impl")
file(MAKE_DIRECTORY "${_cybld}/thrift/python/server_impl/interceptor")

# For cimport during Cython compilation
file(GLOB ServerImplFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/server/*.pxd"
  "${CMAKE_CURRENT_SOURCE_DIR}/server/*.pyx"
)

foreach(_src ${ServerImplFiles})
  get_filename_component(_target_file "${_src}" NAME)
  message(
    STATUS
    "Creating server_impl symlink: \
${_cybld}/thrift/python/server_impl/${_target_file} -> ${_src}"
  )
  add_custom_command(
    TARGET
    create_binding_symlink_server_impl
    PRE_BUILD
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_src}" "${_cybld}/thrift/python/server_impl/${_target_file}"
  )
endforeach()

# Create symlinks for server_impl/interceptor/*.pxd
file(GLOB ServerImplInterceptorFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/server/interceptor/*.pxd"
  "${CMAKE_CURRENT_SOURCE_DIR}/server/interceptor/*.pyx"
)

foreach(_src ${ServerImplInterceptorFiles})
  get_filename_component(_target_file "${_src}" NAME)
  message(
    STATUS
    "Creating server_impl/interceptor symlink: \
${_cybld}/thrift/python/server_impl/interceptor/${_target_file} -> ${_src}"
  )
  add_custom_command(
    TARGET
    create_binding_symlink_server_impl
    PRE_BUILD
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_src}"
      "${_cybld}/thrift/python/server_impl/interceptor/${_target_file}"
  )
endforeach()

#####
# Generate thrift-python test types
#####
add_subdirectory(test)

#####
# Now build everything else in lib/python, including all remaining Cython
# modules that depend on types_api.h and thrift_python_cpp.
#####
add_custom_target(
  thrift_python_and_py3_bindings ALL
  DEPENDS
    generate_apache_thrift_metadata_python
    generate_apache_thrift_metadata_py3
    create_binding_symlink_python
    create_binding_symlink_py3
    create_binding_symlink_server_impl
    create_binding_symlink_lib_test
    thriftcpp2
    thrift_python_cpp
    thrift_python_types_bindings
  WORKING_DIRECTORY ${_cybld}
)

# Copy folly Python bindings into the build directory before wheel creation.
# Folly doesn't build a wheel yet, so we bundle folly.iobuf, folly.executor,
# etc. into the thrift wheel so users don't need to install folly separately.
add_custom_command(TARGET thrift_python_and_py3_bindings
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${FOLLY_PYTHON_SITE_PACKAGES}/folly"
    "${_cybld}/folly"
  COMMENT "Copying folly Python bindings into wheel build directory"
)
add_custom_command(TARGET thrift_python_and_py3_bindings
  COMMAND ${CMAKE_COMMAND} -E env
    "CFLAGS=${CMAKE_C_FLAGS}"
    "CXXFLAGS=${CMAKE_CXX_FLAGS}"
    "CXX=${CMAKE_CXX_COMPILER}"
    "LDFLAGS=${SETUP_LDFLAGS}"
    "CYTHON_INCLUDE_PATH=${cython_include_path}"
    "LIBRARY_DIRS=${library_dirs_str}"
    "THRIFT_BUILD_TESTS=${THRIFT_BUILD_TESTS}"
    python3 ${_lib_dir}/setup.py -v
    bdist_wheel --libpython ${python_libname}
  WORKING_DIRECTORY ${_cybld}
)

# Run auditwheel repair to make the wheel self-contained
# This bundles .so dependencies and rewrites RPATH to $ORIGIN
# LD_LIBRARY_PATH tells auditwheel where to find the libraries to bundle
add_custom_command(TARGET thrift_python_and_py3_bindings
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_cybld}/dist_self_contained"
  COMMAND ${CMAKE_COMMAND} -E env
    "LD_LIBRARY_PATH=${library_dirs_str}"
    python3 -m auditwheel repair
    --wheel-dir "${_cybld}/dist_self_contained"
    "${_cybld}/dist/*.whl"
  WORKING_DIRECTORY ${_cybld}
)

# Install the self-contained wheel to the install prefix
# for users to pip install
# The wheel is processed by auditwheel to ${_cybld}/dist_self_contained/
install(
  DIRECTORY "${_cybld}/dist_self_contained/"
  DESTINATION share/thrift/wheels
  FILES_MATCHING PATTERN "*.whl"
)
