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

# Create tree of symbolic links in structure required for successful
# compliation by Cython.
#   - must be in path named same as extension


# Arguments:
#   name of the extension - e.g. thrift
#   sources...

include(CheckCXXSourceCompiles)

string(CONFIGURE [[
  #include <folly/Portability.h>

  #if !FOLLY_HAS_COROUTINES
  #error Support for coroutines unavailable.
  #endif

  int main() {
    return 0;
  }
  ]] _code)

check_cxx_source_compiles("${_code}" HAVE_STREAM_SUPPORT)

set(_py_srcs
  "__init__.py"
  "client.pxd"
  "client.pyx"
  "ssl.pxd"
  "ssl.pyx"
  "server.pxd"
  "server.pyx"
  "common.pxd"
  "common.pyx"
  "std_libcpp.pxd"
  "exceptions.pxd"
  "exceptions.pyx"
  "types.pxd"
  "types.pyx"
  "reflection.pxd"
  "reflection.pyx"
  "serializer.pxd"
  "serializer.pyx"
)

if(HAVE_STREAM_SUPPORT)
  list(APPEND _py_srcs
    "stream.pxd"
    "stream.pyx"
  )
endif()

set(_extname thrift)
set(_tre thrift/py3)
file(MAKE_DIRECTORY "cybld/${_tre}")
set(_linked_files)
foreach (_src ${_py_srcs})
  add_custom_command(OUTPUT "cybld/${_tre}/${_src}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR}/${_src}
      "cybld/${_tre}/${_src}"
    COMMENT "Generating symlink to ${_cxx}"
  )
  list(APPEND _linked_files "cybld/${_tre}/${_src}")
endforeach ()
add_custom_target(${_extname}-symlinks DEPENDS ${_linked_files})
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cybld/thrift/__init__.pxd)

# Shared library targets for each Cython module, to avoid conficts targets
# named <dir>-<dir>-<module>-py3

set(cpp_tgts
  "thrift/py3/common.cpp"
  "thrift/py3/types.cpp"
  "thrift/py3/exceptions.cpp"
  "thrift/py3/serializer.cpp"
  "thrift/py3/client.cpp"
  "thrift/py3/server.cpp"
  "thrift/py3/ssl.cpp"
  "thrift/py3/reflection.cpp"
)
if(HAVE_STREAM_SUPPORT)
  list(APPEND cpp_tgts "thrift/py3/stream.cpp")
endiF()
foreach(_mod ${cpp_tgts})
  get_filename_component(_f ${_mod} NAME_WE)
  get_filename_component(_d ${_mod} DIRECTORY)
  set(_pyx "cybld/${_d}/${_f}.pyx")
  set(_cxx "cybld/${_d}/${_f}.cpp")
  string(REPLACE "/" "-" _module_name "${_d}/${_f}-py3")
  message(STATUS "Create Cython module ${_module_name} from ${_f}.pyx")

  set_source_files_properties(${_cxx} PROPERTIES GENERATED TRUE)

  add_custom_command(OUTPUT ${_cxx}
    COMMAND ${CYTHON_EXE} --fast-fail -3 --cplus ${_pyx} -o ${_cxx}
    COMMENT "Generating ${_cxx} using Cython"
    DEPENDS thrift-symlinks
  )

  if(${_f} STREQUAL "types")
    python_add_module(${_module_name} "${_cxx}" "enums.cpp")
  else()
    python_add_module(${_module_name} "${_cxx}")
  endif()
  target_link_libraries(${_module_name} thriftcpp2)
  set_target_properties(${_module_name}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "cybld/${_d}"
    LIBRARY_OUTPUT_NAME ${_f}
  )
endforeach()

include_directories(${PYTHON_INCLUDE_DIRS})

# Build and Install Thrift Python Bindings using Python setuptools
if (UNIX)
  set(ROOT_ARG "--root \$DESTDIR/")
endif ()

install(CODE "
  execute_process(COMMAND
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py bdist_wheel
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cybld)
  execute_process(COMMAND
  python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py install
  --prefix ${CMAKE_INSTALL_PREFIX} ${ROOT_ARG}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cybld)"
)

if (enable_tests)
  add_subdirectory(test)
endif ()
