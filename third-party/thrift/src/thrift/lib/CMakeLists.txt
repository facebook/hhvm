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

include(FindFolly)
include(FindPackageHandleStandardArgs)
find_package(Boost MODULE REQUIRED)

set(LIB_HOME ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(thrift)
add_subdirectory(cpp)
add_subdirectory(cpp2)
if(thriftpy)
  add_subdirectory(py)
endif()

if(thriftpy3)
  set(_cybld "${CMAKE_CURRENT_BINARY_DIR}/cybld")
  file(MAKE_DIRECTORY "${_cybld}/thrift/python/")

  #####
  # Compile types.pyx first, as several other cython modules depend on types_api.h
  #####

  add_custom_target(create_binding_symlink_python_types ALL)
  foreach(
    _src
      "${CMAKE_CURRENT_SOURCE_DIR}/python/types.pxd"
      "${CMAKE_CURRENT_SOURCE_DIR}/python/types.pyx"
  )
    get_filename_component(_target_file "${_src}" NAME)

    message(
      STATUS
      "Linking ${_src} to ${_cybld}/thrift/python/_${_target_file}"
    )

    add_custom_command(
      TARGET
      create_binding_symlink_python_types
      PRE_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_cybld}/thrift/python/_${_target_file}"
    )
  endforeach()

  set(inc_dirs "$<TARGET_PROPERTY:thrift,INCLUDE_DIRECTORIES>")
  get_target_property(fmt_include_dirs fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(wangle_incs wangle::wangle INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(thrift_include_dirs thrift INCLUDE_DIRECTORIES)
  list(JOIN wangle_incs ":" wangle_include_dirs)
  set(incs "-I$<JOIN:${inc_dirs},:>:${FOLLY_INCLUDE_DIR}:${Boost_INCLUDE_DIRS}:${fmt_include_dirs}:${wangle_include_dirs}:${CMAKE_BINARY_DIR}/thrift/lib/cybld")
  get_filename_component(folly_lib ${FOLLY_LIBRARY} DIRECTORY)
  set(libs "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${folly_lib}")
  set(py_install_dir ${CMAKE_INSTALL_PREFIX})
  if (NOT ${PYTHON_PACKAGE_INSTALL_DIR} STREQUAL "")
    set(py_install_dir ${PYTHON_PACKAGE_INSTALL_DIR})
  endif()

  add_custom_target(
    thrift_python_types_bindings ALL
    DEPENDS
      create_binding_symlink_python_types
      thriftcpp2
    WORKING_DIRECTORY ${_cybld}
  )
  add_custom_command(TARGET thrift_python_types_bindings
    COMMAND
      python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
      build_ext --types-only -f ${incs} ${libs}
    BYPRODUCTS ${_cybld}/thrift/python/types_api.h
    WORKING_DIRECTORY ${_cybld}
  )

  # After cython runs, link _types_api.h to types_api.h
  add_custom_target(
    create_cython_types_api_symlink ALL
    DEPENDS thrift_python_types_bindings
    WORKING_DIRECTORY ${_cybld})
  file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/")
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink "${_cybld}/thrift/python/_types_api.h" "${_cybld}/thrift/lib/python/types_api.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink "${_cybld}/thrift/python/_types.h" "${_cybld}/thrift/lib/python/_types.h"
  )

  add_library(
    thrift_python_cpp
      python/types.cpp
      python/Serializer.cpp
  )
  add_dependencies(thrift_python_cpp create_cython_types_api_symlink)
  set_property(TARGET thrift_python_cpp PROPERTY VERSION ${PACKAGE_VERSION})
  target_compile_definitions(thrift_python_cpp PRIVATE BOOST_NO_AUTO_PTR)
  target_include_directories(thrift_python_cpp PRIVATE "${_cybld}")
  target_link_libraries(
    thrift_python_cpp
    PUBLIC
      thriftcpp2
      Folly::folly
      Folly::folly_python_cpp
    )
  install(
    TARGETS thrift_python_cpp
    EXPORT thrift
    DESTINATION ${CMAKE_INSTALL_PREFIX}
  )
  install(
    TARGETS thrift_python_cpp
    DESTINATION ${py_install_dir}
  )

  #####
  # Now build everything else in lib/python, including all remaining Cython
  # modules that depend on types_api.h and thrift_python_cpp.
  #####
  add_custom_target(create_binding_symlink_python ALL)
  file(GLOB BindingFiles
    "${CMAKE_CURRENT_SOURCE_DIR}/python/abstract_types.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/metadata.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/common.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/converter.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/exceptions.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/flags.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_containers.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_exceptions.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_serializer.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_typeinfos.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_types.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/protocol.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/serializer.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/std_libcpp.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/types.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/adapter.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/common.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/converter.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/exceptions.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/flags.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_containers.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_exceptions.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_serializer.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_typeinfos.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/mutable_types.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/protocol.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/serializer.pyx"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/flags.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/types.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/python/Serializer.h"
  )

  foreach(_src ${BindingFiles})
    get_filename_component(_target_file "${_src}" NAME)

    message(
      STATUS
      "Linking ${_src} to ${_cybld}/thrift/python/${_target_file}"
    )

    add_custom_command(
      TARGET
      create_binding_symlink_python
      PRE_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_cybld}/thrift/python/${_target_file}"
    )
  endforeach()

  add_custom_target(create_binding_symlink_py3 ALL)
  file(GLOB BindingFiles
    "${CMAKE_CURRENT_SOURCE_DIR}/py3/*.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/py3/*.pyx"
  )
  file(MAKE_DIRECTORY "${_cybld}/thrift/py3/")

  foreach(_src ${BindingFiles})
    get_filename_component(_target_file "${_src}" NAME)

    message(
      STATUS
      "Linking ${_src} to ${_cybld}/thrift/py3/${_target_file}"
    )

    add_custom_command(
      TARGET
      create_binding_symlink_py3
      PRE_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_cybld}/thrift/py3/${_target_file}"
    )
  endforeach()

  add_custom_target(
    thrift_python_and_py3_bindings ALL
    DEPENDS
      create_binding_symlink_python
      create_binding_symlink_py3
      thriftcpp2
      thrift_python_cpp
    WORKING_DIRECTORY ${_cybld}
  )
  add_custom_command(TARGET thrift_python_and_py3_bindings
    COMMAND
      python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
      build_ext -f ${incs} ${libs}
    WORKING_DIRECTORY ${_cybld}
  )
  add_custom_command(TARGET thrift_python_and_py3_bindings
    COMMAND
      python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
      bdist_wheel
    WORKING_DIRECTORY ${_cybld}
  )

  add_custom_command(TARGET thrift_python_and_py3_bindings POST_BUILD
    COMMAND
      python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
      install --prefix ${CMAKE_INSTALL_PREFIX}
    WORKING_DIRECTORY ${_cybld}
  )

  # getdeps.py does not allow sufficient control of CMAKE_INSTALL_PREFIX,
  # so PYTHON_PACKAGE_INSTALL_DIR can be used to control the installation
  # location of Python packages.
  install(CODE "
    execute_process(
      COMMAND
        python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py install
        --prefix ${py_install_dir}
      COMMAND_ECHO STDOUT
      WORKING_DIRECTORY ${_cybld}
    )"
  )
endif()

set(LIB_DIRS
  thrift
  cpp
  cpp2
  python
  py3
)
foreach(dir ${LIB_DIRS})
  install(DIRECTORY ${dir} "${CMAKE_CURRENT_BINARY_DIR}/${dir}"
        DESTINATION include/thrift/lib
        FILES_MATCHING
          PATTERN "*.h"
          PATTERN "*.tcc"
          PATTERN CMakeFiles EXCLUDE)
endforeach()
