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

# Generate thrift-python test types

# Signal to setup.py that test extensions should be built
set(THRIFT_BUILD_TESTS "1" PARENT_SCOPE)

# Include ThriftLibrary macros
# (for standalone builds, CMAKE_MODULE_PATH must be set)
include(ThriftLibrary OPTIONAL)

#
# thrift_python_library_for_test
# Generate thrift-python code for tests.
#
# Parameters:
#   file_name, services, options, file_path, output_path, include_prefix
#   THRIFT_INCLUDE_DIRECTORIES (optional)
#   NAMESPACE (optional)
#
function(thrift_python_library_for_test
  file_name
  services
  options
  file_path
  output_path
  include_prefix
)
  cmake_parse_arguments(THRIFT_GENERATE
    ""
    "TARGET_NAME_BASE;NAMESPACE"
    "THRIFT_INCLUDE_DIRECTORIES"
    ${ARGN})

  set(source_file_name ${file_name})
  set(target_file_name ${file_name})
  if(DEFINED THRIFT_GENERATE_TARGET_NAME_BASE
      AND NOT THRIFT_GENERATE_TARGET_NAME_BASE STREQUAL "")
    set(target_file_name ${THRIFT_GENERATE_TARGET_NAME_BASE})
  endif()

  set(thrift_include_directories)
  foreach(dir ${THRIFT_GENERATE_THRIFT_INCLUDE_DIRECTORIES})
    list(APPEND thrift_include_directories "-I" "${dir}")
  endforeach()

  # thrift-python (mstch_python generator)
  set(gen_language "mstch_python")
  set(gen_dir "gen-python")

  if(DEFINED THRIFT_GENERATE_NAMESPACE
      AND NOT THRIFT_GENERATE_NAMESPACE STREQUAL "")
    string(REPLACE "." "/" _namespace_dir "${THRIFT_GENERATE_NAMESPACE}")
    set(_output_subdir "${_namespace_dir}/${source_file_name}")
  else()
    set(_output_subdir "${source_file_name}")
  endif()

  set(_sources
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_types.py
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_types.pyi
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_metadata.py
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_enums.py
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_abstract_types.py
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_mutable_types.py
    ${output_path}/${gen_dir}/${_output_subdir}/thrift_mutable_types.pyi
  )
  if(NOT "${services}" STREQUAL "")
    list(APPEND _sources
      ${output_path}/${gen_dir}/${_output_subdir}/thrift_services.py
      ${output_path}/${gen_dir}/${_output_subdir}/thrift_clients.py
      ${output_path}/${gen_dir}/${_output_subdir}/thrift_mutable_services.py
      ${output_path}/${gen_dir}/${_output_subdir}/thrift_mutable_clients.py
    )
  endif()

  if(NOT "${options}" STREQUAL "")
    set(_gen_options ":${options}")
  else()
    set(_gen_options "")
  endif()

  add_custom_command(
    OUTPUT ${_sources}
    COMMAND ${THRIFT1}
      --gen "${gen_language}${_gen_options}"
      -o ${output_path}
      ${thrift_include_directories}
      "${file_path}/${source_file_name}.thrift"
    DEPENDS
      ${THRIFT1}
      "${file_path}/${source_file_name}.thrift"
    COMMENT "Generating ${target_file_name} thrift-python files"
  )

  add_custom_target(
    ${target_file_name}-python-target ALL
    DEPENDS ${_sources}
  )

  message("thrift_python_library_for_test: ${file_name}")
endfunction()

#####
# Create test package symlinks for thrift.python.test and
# thrift.python.test.adapters
#####
add_custom_target(create_binding_symlink_test ALL)

# Parent directory (thrift/lib/python/) for accessing sibling directories
get_filename_component(_python_src_dir "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

# Helper function: create multiple directories from a list
function(make_directories)
  foreach(_dir ${ARGN})
    file(MAKE_DIRECTORY "${_dir}")
  endforeach()
endfunction()

# Helper function: create symlinks for files to a target directory
function(symlink_files_to_target target_name dest_dir)
  foreach(_src ${ARGN})
    get_filename_component(_target_file "${_src}" NAME)
    add_custom_command(
      TARGET ${target_name}
      PRE_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        "${_src}" "${dest_dir}/${_target_file}"
    )
  endforeach()
endfunction()

# Helper function: register Python unittest tests
function(register_python_tests test_prefix module_prefix test_env working_dir)
  foreach(test_file ${ARGN})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_test(
      NAME ${test_prefix}_${test_name}
      COMMAND ${Python3_EXECUTABLE} -m unittest ${module_prefix}.${test_name} -v
      WORKING_DIRECTORY ${working_dir}
    )
    set_tests_properties(${test_prefix}_${test_name} PROPERTIES
      ENVIRONMENT "${test_env}"
    )
  endforeach()
endfunction()

make_directories(
  "${_cybld}/thrift/python/test"
  "${_cybld}/thrift/python/test/adapters"
)

file(GLOB TestFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/*.py"
)

symlink_files_to_target(create_binding_symlink_test
  "${_cybld}/thrift/python/test" ${TestFiles})

file(GLOB TestAdapterFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/adapters/*.py"
)

symlink_files_to_target(create_binding_symlink_test
  "${_cybld}/thrift/python/test/adapters" ${TestAdapterFiles})

# Create request_context_extractor subpackage symlinks
file(MAKE_DIRECTORY "${_cybld}/thrift/python/test/request_context_extractor")

file(GLOB TestRequestContextExtractorFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/request_context_extractor/*.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/request_context_extractor/*.pyx"
  "${CMAKE_CURRENT_SOURCE_DIR}/request_context_extractor/*.pxd"
  "${CMAKE_CURRENT_SOURCE_DIR}/request_context_extractor/*.pyi"
)

symlink_files_to_target(create_binding_symlink_test
  "${_cybld}/thrift/python/test/request_context_extractor"
  ${TestRequestContextExtractorFiles}
)

# Create server/test Cython module symlinks in thrift/python/test/
# Note: The Cython module needs to be at
# thrift.python.test.python_async_processor_factory_test
# because thrift.python.server is an extension module (.so), not a package
file(GLOB ServerTestCythonFiles
  "${_python_src_dir}/server/test/*.pyx"
  "${_python_src_dir}/server/test/*.pxd"
  "${_python_src_dir}/server/test/*.pyi"
)

symlink_files_to_target(create_binding_symlink_test
  "${_cybld}/thrift/python/test" ${ServerTestCythonFiles})

# Create client/test Cython module symlinks in thrift/python/client/test/
file(MAKE_DIRECTORY "${_cybld}/thrift/python/client/test")
file(GLOB ClientTestCythonFiles
  "${_python_src_dir}/client/test/*.pyx"
  "${_python_src_dir}/client/test/*.pxd"
  "${_python_src_dir}/client/test/*.pyi"
  "${_python_src_dir}/client/test/*.h"
)

symlink_files_to_target(create_binding_symlink_test
  "${_cybld}/thrift/python/client/test" ${ClientTestCythonFiles})

#####
# Create thrift.lib.python.test package symlinks (fbcode import path shim)
# This mirrors thrift.python.test to thrift.lib.python.test for fbcode
# compatibility
#####
add_custom_target(create_binding_symlink_lib_test ALL)

make_directories(
  "${_cybld}/thrift/lib"
  "${_cybld}/thrift/lib/python"
  "${_cybld}/thrift/lib/python/test"
  "${_cybld}/thrift/lib/python/test/event_handlers"
)

symlink_files_to_target(create_binding_symlink_lib_test
  "${_cybld}/thrift/lib/python/test" ${TestFiles})

file(GLOB TestEventHandlerFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/event_handlers/*.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/event_handlers/*.pyx"
  "${CMAKE_CURRENT_SOURCE_DIR}/event_handlers/*.pxd"
)

symlink_files_to_target(create_binding_symlink_lib_test
  "${_cybld}/thrift/lib/python/test/event_handlers"
  ${TestEventHandlerFiles}
)

# Create thrift/lib/python/test/metadata_response/ symlinks
# for metadata_response Cython module
file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/test/metadata_response")

file(GLOB TestMetadataResponseFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/metadata_response/*.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/metadata_response/*.pyx"
  "${CMAKE_CURRENT_SOURCE_DIR}/metadata_response/*.pxd"
  "${CMAKE_CURRENT_SOURCE_DIR}/metadata_response/*.pyi"
  "${CMAKE_CURRENT_SOURCE_DIR}/metadata_response/*.h"
)

symlink_files_to_target(create_binding_symlink_lib_test
  "${_cybld}/thrift/lib/python/test/metadata_response"
  ${TestMetadataResponseFiles}
)

# Create thrift/lib/python/client/test/ symlinks
# for client test Cython modules
make_directories(
  "${_cybld}/thrift/lib/python/client"
  "${_cybld}/thrift/lib/python/client/test"
  "${_cybld}/thrift/lib/python/client/test/client_event_handler"
)

file(GLOB LibClientTestCythonFiles
  "${_python_src_dir}/client/test/*.pyx"
  "${_python_src_dir}/client/test/*.pxd"
  "${_python_src_dir}/client/test/*.pyi"
  "${_python_src_dir}/client/test/*.h"
  "${_python_src_dir}/client/test/*.py"
)

symlink_files_to_target(create_binding_symlink_lib_test
  "${_cybld}/thrift/lib/python/client/test"
  ${LibClientTestCythonFiles}
)

file(GLOB LibClientEventHandlerFiles
  "${_python_src_dir}/client/test/client_event_handlers/*.pyx"
  "${_python_src_dir}/client/test/client_event_handlers/*.pxd"
  "${_python_src_dir}/client/test/client_event_handlers/*.pyi"
  "${_python_src_dir}/client/test/client_event_handlers/*.h"
)

symlink_files_to_target(create_binding_symlink_lib_test
  "${_cybld}/thrift/lib/python/client/test/client_event_handler"
  ${LibClientEventHandlerFiles}
)

set(TEST_THRIFT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PY3_TEST_THRIFT_DIR ${CMAKE_SOURCE_DIR}/thrift/lib/py3/test)

# Files from py3/test needed by thrift-python tests
# (no namespace in thrift file)
# Output goes to gen-python/binary/, gen-python/iobuf/ etc.
thrift_python_library_for_test(
  "binary"
  "BinaryService"
  ""
  "${PY3_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE ""
)

thrift_python_library_for_test(
  "iobuf"
  "IobufTestService"
  ""
  "${PY3_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE ""
)

# derived.thrift - no py3 namespace, includes testing.thrift
thrift_python_library_for_test(
  "derived"
  "DerivedTestingService"
  ""
  "${PY3_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE ""
)

# convertible.thrift - no py3 namespace
thrift_python_library_for_test(
  "convertible"
  ""
  ""
  "${PY3_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE ""
)

# stack_args.thrift - no py3 namespace
thrift_python_library_for_test(
  "stack_args"
  "StackService"
  ""
  "${PY3_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE ""
)

set(THRIFT_TEST_DIR ${CMAKE_SOURCE_DIR}/thrift/test)
thrift_python_library_for_test(
  "schema_evolution_test"
  ""
  ""
  "${THRIFT_TEST_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE ""
)

# testing.thrift has namespace py3 "" (empty) - uses filename as package
# Build service list separately to avoid long lines
set(_testing_services
  "TestingService;TestingServiceChild;ExtendServiceWithNoTypes")
string(APPEND _testing_services ";ClientMetadataTestingService")
thrift_python_library_for_test(
  "testing"
  "${_testing_services}"
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
)

# Files with namespace py3 "testing" - output to testing/<filename>/
thrift_python_library_for_test(
  "base_service_only"
  "BaseService"
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

thrift_python_library_for_test(
  "dependency"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

thrift_python_library_for_test(
  "sub_dependency"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

thrift_python_library_for_test(
  "injected_field"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

thrift_python_library_for_test(
  "py_deprecated_asyncio_test"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

thrift_python_library_for_test(
  "py_deprecated_asyncio_fallback_test"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

thrift_python_library_for_test(
  "test_inject_metadata_fields"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "testing"
)

# Files with namespace py3 "python_test"
thrift_python_library_for_test(
  "adapter"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "containers"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "enums"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "enums_typedef_only"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "lists"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "maps"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "refs"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

thrift_python_library_for_test(
  "sets"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "python_test"
)

# Files with namespace py3 "thrift.python"
thrift_python_library_for_test(
  "bidi_service"
  "TestBidiService"
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python"
)

thrift_python_library_for_test(
  "sink_service"
  "TestSinkService"
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python"
)

# Files with namespace py3 "thrift.python.test"
thrift_python_library_for_test(
  "special_cases"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python.test"
)

# Files without namespace py3 - use empty namespace
# (output to gen-python/<file_name>/)
thrift_python_library_for_test(
  "abstract_types_flags"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python.test"
)

thrift_python_library_for_test(
  "annotations"
  ""
  ""
  "${TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python.test"
)

set(THRIFT_PYTHON_TEST_DIR "${CMAKE_SOURCE_DIR}/thrift/test/thrift-python")

thrift_python_library_for_test(
  "struct_test"
  ""
  ""
  "${THRIFT_PYTHON_TEST_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.test.thrift_python"
)

thrift_python_library_for_test(
  "union_test"
  ""
  ""
  "${THRIFT_PYTHON_TEST_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.test.thrift_python"
)

thrift_python_library_for_test(
  "enum_test"
  ""
  ""
  "${THRIFT_PYTHON_TEST_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.test.thrift_python"
)

thrift_python_library_for_test(
  "included"
  ""
  ""
  "${THRIFT_PYTHON_TEST_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.test.thrift_python"
)

# transitive_deps thrift files - from py3/test/transitive_deps/
# namespace py3 transitive_deps -> output to transitive_deps/a/, etc.
set(TRANSITIVE_DEPS_DIR ${CMAKE_SOURCE_DIR}/thrift/lib/py3/test/transitive_deps)

thrift_python_library_for_test(
  "a"
  ""
  ""
  "${TRANSITIVE_DEPS_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "transitive_deps"
)

thrift_python_library_for_test(
  "b"
  ""
  ""
  "${TRANSITIVE_DEPS_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "transitive_deps"
)

thrift_python_library_for_test(
  "c"
  ""
  ""
  "${TRANSITIVE_DEPS_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "transitive_deps"
)

thrift_python_library_for_test(
  "d"
  ""
  ""
  "${TRANSITIVE_DEPS_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "transitive_deps"
)

# terse_write.thrift - from thrift/test/terse_write/
# namespace py3 thrift.test
# -> output to thrift/test/terse_write/
set(TERSE_WRITE_DIR ${CMAKE_SOURCE_DIR}/thrift/test/terse_write)

thrift_python_library_for_test(
  "terse_write"
  ""
  ""
  "${TERSE_WRITE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.test"
)

# client/test thrift files (test.thrift, leaf.thrift)
# Namespace py3 "thrift.python" means output goes to
# thrift/python/test/ and thrift/python/leaf/
set(CLIENT_TEST_THRIFT_DIR "${CMAKE_SOURCE_DIR}/thrift/lib/python/client/test")

# test.thrift - defines TestService, EchoService
thrift_python_library_for_test(
  "test"
  "TestService;EchoService"
  ""
  "${CLIENT_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python"
)

# leaf.thrift - defines LeafService (extends EchoService from test.thrift)
thrift_python_library_for_test(
  "leaf"
  "LeafService"
  ""
  "${CLIENT_TEST_THRIFT_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  ""
  THRIFT_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
  NAMESPACE "thrift.python"
)

#####
# Create symlinks for test namespaces
# gen-python/<namespace>/ -> cybld/<namespace>/
#####
# Note: Cython/setuptools uses "lib.linux-<arch>-cpython-<version>" format
# (e.g., lib.linux-x86_64-cpython-310)
set(_cybld_base "${CMAKE_BINARY_DIR}/thrift/lib/cybld")
set(_py_ver "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
set(_cybld_suffix
  "build/lib.linux-${CMAKE_HOST_SYSTEM_PROCESSOR}-cpython-${_py_ver}")
set(_cybld "${_cybld_base}/${_cybld_suffix}")
set(_gen_python "${CMAKE_CURRENT_BINARY_DIR}/gen-python")

#####
# Python Test Definitions
#####

# ==========================================================================
# WHEEL-BASED TEST SETUP
# ==========================================================================
# Instead of complex PYTHONPATH + symlinks, we install wheels:
# 1. thrift wheel - provides thrift.python.*, thrift.py3.*, thrift.lib.python.*
# 2. folly wheel - assumed already installed by getdeps
#    (provides folly.iobuf, etc.)
#
# TEST_PYTHONPATH only needs:
# - _gen_python: generated test thrift code (testing.thrift_types, etc.)
# ==========================================================================

# Install thrift wheel before running tests
# The wheel is built by thrift_python_and_py3_bindings target
# in thrift/lib/CMakeLists.txt
# Use CMAKE_BINARY_DIR (top-level build dir) for absolute path
# to avoid fragile relative paths
# Use --target to install to a known directory that we add to TEST_PYTHONPATH
set(_thrift_wheel_dir
  "${CMAKE_BINARY_DIR}/thrift/lib/cybld/dist_self_contained")
set(_thrift_wheel_install_dir
  "${CMAKE_BINARY_DIR}/thrift/lib/thrift_wheel_pythonpath")
# Note: auditwheel may change the wheel platform tag, so we use a shell glob
string(CONCAT _pip_install_cmd
  "${Python3_EXECUTABLE} -m pip install --force-reinstall --no-deps"
  " --target '${_thrift_wheel_install_dir}'"
  " '${_thrift_wheel_dir}'/thrift-0.0.1-*.whl")
add_custom_target(install_thrift_wheel ALL
  DEPENDS thrift_python_and_py3_bindings
  COMMAND bash -c "${_pip_install_cmd}"
  COMMAND ${Python3_EXECUTABLE} -m pip install
    --target "${_thrift_wheel_install_dir}" parameterized
  COMMENT "Installing thrift wheel and test dependencies"
)

# Build PYTHONPATH for tests
# With wheel-based approach for thrift, we need paths for:
# - Thrift wheel install directory
#   (thrift.python.*, thrift.py3.*, folly.*, etc.)
# - Generated test thrift code (testing.thrift_types, etc.)
# Note: Folly Python bindings are now bundled into the thrift wheel
# Note: Test modules are NOT in the wheel (removed by commit 1d87cfcfed).
# - _cybld: Cython test extensions (.so files)
# - _cybld_base: Pure Python test modules (adapters, etc.) - symlinked sources
string(CONCAT TEST_PYTHONPATH
  "${_thrift_wheel_install_dir}"
  ":${_cybld}"
  ":${_cybld_base}"
  ":${_gen_python}")

#####
# Create fbthrift_test package to avoid module shadowing
#####
# Problem: Test files like binary.py, iobuf.py, transitive_deps.py have
# same names as gen-python packages (binary/, iobuf/, transitive_deps/).
# When running "python binary.py", Python adds the test directory to
# sys.path[0], causing "import binary" to find the test file instead
# of the package.
#
# Solution (same as Buck): Run tests as modules, not scripts.
# Create a fbthrift_test/ package with symlinks to test files, then run:
#   python -m unittest fbthrift_test.binary
# This sets sys.path[0] to '' (cwd), so "import binary" finds the package.
#####

set(_fbthrift_test_dir "${_gen_python}/fbthrift_test")

# Create fbthrift_test package directory
add_custom_command(
  OUTPUT "${_fbthrift_test_dir}/__init__.py"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_fbthrift_test_dir}"
  COMMAND ${CMAKE_COMMAND} -E touch "${_fbthrift_test_dir}/__init__.py"
  COMMENT "Creating fbthrift_test package directory"
)

# List of test files
# (excluding non-test files like testing_utils.py, test_server.py)
set(THRIFT_PYTHON_TEST_FILES
  # abstract_types_flags_test.py - SKIPPED: test expects abstract types
  # flag to be disabled, but OSS build enables it
  adapter.py
  binary.py
  client_server.py
  # converter.py - SKIPPED: requires convertible.types
  # from py3 codegen (not built)
  enums.py
  exceptions.py
  exceptions_ft.py
  iobuf.py
  # intercompatibility.py - SKIPPED: requires testing.types
  # from py3 codegen (not built)
  lists.py
  maps.py
  metadata.py
  mutable_exceptions.py
  mutable_list_test.py
  mutable_map_test.py
  mutable_set_test.py
  mutable_structs.py
  mutable_structs_ft.py
  mutable_unions.py
  python_annotations_test.py
  refs.py
  serializer.py
  sets.py
  special_cases_test.py
  structs.py
  structs_ft.py
  test_helpers.py
  transitive_deps.py
  typeinfo_test.py
  unions.py
  unions_ft.py
)

# Also symlink helper files that tests import
set(THRIFT_PYTHON_TEST_HELPERS
  testing_utils.py
  test_server.py
)

# Create symlinks for all test files
set(TEST_SYMLINK_OUTPUTS "")
foreach(test_file ${THRIFT_PYTHON_TEST_FILES} ${THRIFT_PYTHON_TEST_HELPERS})
  set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${test_file}")
  set(_dst "${_fbthrift_test_dir}/${test_file}")
  list(APPEND TEST_SYMLINK_OUTPUTS "${_dst}")
  add_custom_command(
    OUTPUT "${_dst}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_dst}"
    DEPENDS "${_src}" "${_fbthrift_test_dir}/__init__.py"
    COMMENT "Symlinking ${test_file} to fbthrift_test package"
  )
endforeach()

# Add server/test .py file to fbthrift_test package
# This file is in python/server/test/ but needs to be accessible
# via fbthrift_test
string(CONCAT _server_test_src
  "${_python_src_dir}/server/test"
  "/python_async_processor_factory_test.py")
set(_server_test_dst
  "${_fbthrift_test_dir}/python_async_processor_factory_test.py")
add_custom_command(
  OUTPUT "${_server_test_dst}"
  COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${_server_test_src}" "${_server_test_dst}"
  DEPENDS "${_server_test_src}" "${_fbthrift_test_dir}/__init__.py"
  COMMENT "Symlinking python_async_processor_factory_test.py"
)
list(APPEND TEST_SYMLINK_OUTPUTS "${_server_test_dst}")

add_custom_target(thrift_python_test_package ALL
  DEPENDS "${_fbthrift_test_dir}/__init__.py" ${TEST_SYMLINK_OUTPUTS}
)

# Add dependencies on thrift generation targets
add_dependencies(thrift_python_test_package
  # Core test types (no namespace or namespace "testing")
  testing-python-target
  dependency-python-target
  sub_dependency-python-target
  base_service_only-python-target
  injected_field-python-target
  test_inject_metadata_fields-python-target
  py_deprecated_asyncio_test-python-target
  py_deprecated_asyncio_fallback_test-python-target
  # py3/test types (no namespace)
  binary-python-target
  iobuf-python-target
  derived-python-target
  convertible-python-target
  stack_args-python-target
  schema_evolution_test-python-target
  # python_test namespace
  adapter-python-target
  containers-python-target
  enums-python-target
  refs-python-target
  maps-python-target
  lists-python-target
  sets-python-target
  terse_write-python-target
  enums_typedef_only-python-target
)

# Register each test file with ctest
# Run as module to avoid sys.path[0] = test_dir shadowing

# Build the test environment string once (reused by all tests)
# NOTE: We do NOT set LD_LIBRARY_PATH or LD_PRELOAD.
# The wheel must be self-contained: auditwheel bundles deps into .libs/,
# and RPATH $ORIGIN/.libs finds them. If tests fail, the wheel isn't portable.
set(_test_env "PYTHONPATH=${TEST_PYTHONPATH}")

register_python_tests(thrift_python fbthrift_test
  "${_test_env}" "${_gen_python}" ${THRIFT_PYTHON_TEST_FILES})

#####
# thrift/test/thrift-python/ tests
#####
# These tests are in a separate directory and use namespace
# thrift.test.thrift_python
# The thrift_library entries already exist above
# (struct_test, union_test, enum_test, included)

set(THRIFT_TEST_THRIFT_PYTHON_DIR
  "${CMAKE_SOURCE_DIR}/thrift/test/thrift-python")

# Create thrift_test_thrift_python_tests package for these tests
set(_thrift_test_tests_dir "${_gen_python}/thrift_test_thrift_python_tests")

add_custom_command(
  OUTPUT "${_thrift_test_tests_dir}/__init__.py"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_thrift_test_tests_dir}"
  COMMAND ${CMAKE_COMMAND} -E touch "${_thrift_test_tests_dir}/__init__.py"
  COMMENT "Creating thrift_test_thrift_python_tests package directory"
)

set(THRIFT_TEST_THRIFT_PYTHON_TEST_FILES
  abstract_types_test.py
  enum_test.py
  schema_evolution_test.py
  struct_test.py
  union_test.py
)

set(THRIFT_TEST_SYMLINK_OUTPUTS "")
foreach(test_file ${THRIFT_TEST_THRIFT_PYTHON_TEST_FILES})
  set(_src "${THRIFT_TEST_THRIFT_PYTHON_DIR}/${test_file}")
  set(_dst "${_thrift_test_tests_dir}/${test_file}")
  list(APPEND THRIFT_TEST_SYMLINK_OUTPUTS "${_dst}")
  add_custom_command(
    OUTPUT "${_dst}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_dst}"
    DEPENDS "${_src}" "${_thrift_test_tests_dir}/__init__.py"
    COMMENT "Symlinking ${test_file} to thrift_test_thrift_python_tests package"
  )
endforeach()

add_custom_target(thrift_test_thrift_python_test_package ALL
  DEPENDS "${_thrift_test_tests_dir}/__init__.py" ${THRIFT_TEST_SYMLINK_OUTPUTS}
  struct_test-python-target union_test-python-target
  enum_test-python-target included-python-target
)

register_python_tests(thrift_test_thrift_python thrift_test_thrift_python_tests
  "${_test_env}" "${_gen_python}" ${THRIFT_TEST_THRIFT_PYTHON_TEST_FILES})

#####
# thrift/lib/python/test/adapters/ tests
#####
set(ADAPTERS_TEST_FILES
  atoi_test.py
  datetime_test.py
)

register_python_tests(thrift_python_adapters thrift.python.test.adapters
  "${_test_env}" "${_gen_python}" ${ADAPTERS_TEST_FILES})

#####
# thrift/lib/python/test/request_context_extractor/ tests
#####
add_test(
  NAME thrift_python_request_context_extractor_test
  COMMAND ${Python3_EXECUTABLE} -m unittest
    thrift.python.test.request_context_extractor.request_context_extractor_test
    -v
  WORKING_DIRECTORY ${_gen_python}
)
set_tests_properties(thrift_python_request_context_extractor_test PROPERTIES
  ENVIRONMENT "${_test_env}"
)

#####
# thrift/lib/python/server/test/ tests
# Note: The .py test file imports from
#   thrift.python.test.python_async_processor_factory_test
# We symlink the .py to fbthrift_test/ and run via fbthrift_test module
#####
add_test(
  NAME thrift_python_server_python_async_processor_factory_test
  COMMAND ${Python3_EXECUTABLE} -m unittest
    fbthrift_test.python_async_processor_factory_test -v
  WORKING_DIRECTORY ${_gen_python}
)
set_tests_properties(
  thrift_python_server_python_async_processor_factory_test PROPERTIES
  ENVIRONMENT "${_test_env}"
)

#####
# thrift/lib/python/client/test/ tests
#
# These tests require proxygen library (for http2_helper.pyx).
# The test_server.py imports http2_helper which depends on
# proxygen::httpserver.
#
# Dependencies:
# - proxygen (HTTPServerOptions) - added as fbthrift manifest dependency
# - http2_helper.pyx - added to setup.py Cython extensions
# - thrift.py3.server - for get_context, SocketAddress
#   (built as part of py3 bindings)
#####

set(CLIENT_TEST_DIR "${CMAKE_SOURCE_DIR}/thrift/lib/python/client/test")

# Create fbthrift_client_test package for client tests
# (same approach as fbthrift_test)
set(_fbthrift_client_test_dir "${_gen_python}/fbthrift_client_test")

add_custom_command(
  OUTPUT "${_fbthrift_client_test_dir}/__init__.py"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_fbthrift_client_test_dir}"
  COMMAND ${CMAKE_COMMAND} -E touch "${_fbthrift_client_test_dir}/__init__.py"
  COMMENT "Creating fbthrift_client_test package directory"
)

# Client test files
# SKIPPED: async_client_test.py and sync_client_test.py require
# http2_helper.pyx which has undefined symbol:
# apache::thrift::HTTP2RoutingHandler
# The HTTP2RoutingHandler is not linked correctly in the OSS build.
# To fix: Need to link http2_helper extension against the library
# containing HTTP2RoutingHandler
set(CLIENT_TEST_FILES
  # async_client_test.py - SKIPPED: requires http2_helper
  #   (HTTP2RoutingHandler linking issue)
  # sync_client_test.py - SKIPPED: requires http2_helper
  #   (HTTP2RoutingHandler linking issue)
)

# Helper files needed by client tests
set(CLIENT_TEST_HELPERS
  test_server.py
)

set(CLIENT_TEST_SYMLINK_OUTPUTS "")
foreach(test_file ${CLIENT_TEST_FILES} ${CLIENT_TEST_HELPERS})
  set(_src "${CLIENT_TEST_DIR}/${test_file}")
  set(_dst "${_fbthrift_client_test_dir}/${test_file}")
  list(APPEND CLIENT_TEST_SYMLINK_OUTPUTS "${_dst}")
  add_custom_command(
    OUTPUT "${_dst}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_dst}"
    DEPENDS "${_src}" "${_fbthrift_client_test_dir}/__init__.py"
    COMMENT "Symlinking ${test_file} to fbthrift_client_test package"
  )
endforeach()

add_custom_target(thrift_python_client_test_package ALL
  DEPENDS "${_fbthrift_client_test_dir}/__init__.py"
    ${CLIENT_TEST_SYMLINK_OUTPUTS}
  test-python-target leaf-python-target
)

register_python_tests(thrift_python_client fbthrift_client_test
  "${_test_env}" "${_gen_python}" ${CLIENT_TEST_FILES})

#####
# thrift/lib/python/test/metadata_response/ tests
#
# Tests for metadata response handling.
# Requires:
# - metadata_response.pyx Cython extension (added to setup.py)
# - thrift.lib.python.test.test_server helper
# - testing.thrift_services and testing.thrift_types (generated)
# - apache.thrift.metadata.thrift_types (generated)
#####
add_test(
  NAME thrift_python_metadata_response_test
  COMMAND ${Python3_EXECUTABLE} -m unittest
    thrift.lib.python.test.metadata_response.metadata_response_test -v
  WORKING_DIRECTORY ${_gen_python}
)
set_tests_properties(thrift_python_metadata_response_test PROPERTIES
  ENVIRONMENT "${_test_env}"
)
