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

include_directories("${CMAKE_CURRENT_BINARY_DIR}/..")

macro(thrift_py3_test filename services)
  thrift_library(
    ${filename}
    ${services}
    "cpp2"
    ""
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "test"
  )

  set_target_properties(${filename}-cpp2-obj
     PROPERTIES POSITION_INDEPENDENT_CODE True)
  target_include_directories(${filename}-cpp2-obj PUBLIC ${FOLLY_INCLUDE_DIR})

  set(_py3_thrift_flags "")
  if(NOT HAVE_STREAM_SUPPORT)
    message(WARNING "Support for Thrift 'stream' type disabled for lack of C++ coroutine support.")
    set(_py3_thrift_flags "no_stream")
  endif()

  thrift_generate(
    ${filename}
    ${services}
    "py3"
    "${_py3_thrift_flags}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "test"
  )

  foreach(_src
    "types"
    "clients"
    "services"
  )
    set(_pyx "gen-py3/${filename}/${_src}.pyx")
    set(_cxx "gen-py3/${filename}/${_src}.cpp")
    string(REPLACE "/" "-" _module_name "${filename}/${_src}-py3")
    message(STATUS "Create Cython module ${_module_name} from ${_pyx}")

    add_custom_command(OUTPUT ${_cxx}
      COMMAND ${CYTHON_EXE} --fast-fail -3 --cplus ${_pyx} -o ${_cxx}
        -I${THRIFT_BUILD}/thrift/lib/py3/cybld/
      COMMENT "Generating ${_cxx} using Cython"
      DEPENDS thriftcpp2 thrift-symlinks
    )

    if(${_src} STREQUAL "types")
      python_add_module(${_module_name} "${_cxx}")
    else()
      python_add_module(${_module_name} "${_cxx}" "gen-py3/${filename}/${_src}_wrapper.cpp")
      set_source_files_properties(
        "gen-py3/${filename}/${_src}_wrapper.cpp"
        PROPERTIES GENERATED TRUE
      )
    endif()
    target_link_libraries(${_module_name} thriftcpp2 "${filename}-cpp2")
    set_target_properties(${_module_name}
      PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "gen-py3/${filename}"
      LIBRARY_OUTPUT_NAME ${_src})
  endforeach()
endmacro()

include_directories(${PYTHON_INCLUDE_DIRS})
thrift_py3_test("testing" "TestingService")
thrift_py3_test("iobuf" "IobufTestService")
thrift_py3_test("stack_args" "StackService")
thrift_py3_test("binary" "BinaryService")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gen-py3/test")

set(_linked_files)
foreach(_src
  binary.py
  client_server.py
  clients.py
  custom.py
  enums.py
  exceptions.py
  exception_helper.pyx
  iobuf.py
  lists.py
  maps.py
  serializer.py
  server.py
  sets.py
  structs.py
  unions.py
)
  set(_link_dest "gen-py3/test/${_src}")
  add_custom_command(OUTPUT ${_link_dest}
    COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR}/${_src}
      "${_link_dest}"
    COMMENT "Generating symlink to ${_link_dest}"
  )
  list(APPEND _linked_files "${_link_dest}")
endforeach()
add_custom_target(py3-test-symlinks DEPENDS ${_linked_files})
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gen-py3/test/__init__.py)

foreach(_src
    "exception_helper"
)
  set(_pyx "gen-py3/test/${_src}.pyx")
  set(_cxx "gen-py3/test/${_src}.cpp")
  string(REPLACE "/" "-" _module_name "test/${_src}-py3")
  message(STATUS "Create Cython module ${_module_name} from ${_pyx}")

  add_custom_command(OUTPUT ${_cxx}
    COMMAND ${CYTHON_EXE} --fast-fail -3 --cplus ${_pyx} -o ${_cxx}
      -I${THRIFT_BUILD}/thrift/lib/py3/cybld/
    COMMENT "Generating ${_cxx} using Cython"
    DEPENDS thrift-symlinks py3-test-symlinks
  )

  python_add_module(${_module_name} "${_cxx}")

  set_target_properties(${_module_name}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY gen-py3/test
    LIBRARY_OUTPUT_NAME "${_src}")

  target_link_libraries(${_module_name} iobuf-cpp2 thriftcpp2)
endforeach()

