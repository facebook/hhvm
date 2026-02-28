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
if(thrift_python)
  add_subdirectory(python)
endif()
# DEPRECATED: thriftpy is deprecated. On Linux, use thrift_python instead.
if(thriftpy)
  add_subdirectory(py)
endif()

# DEPRECATED: thriftpy3 is deprecated. On Linux, use thrift_python instead.
if(thriftpy3)
  set(_cybld "${CMAKE_CURRENT_BINARY_DIR}/cybld")
  file(MAKE_DIRECTORY "${_cybld}/thrift/python/")
  file(MAKE_DIRECTORY "${_cybld}/thrift/python/client/")
  file(MAKE_DIRECTORY "${_cybld}/thrift/python/server/")
  file(MAKE_DIRECTORY "${_cybld}/thrift/python/server/flagged/")
  file(MAKE_DIRECTORY "${_cybld}/thrift/py3/")

  # So that cython includes work correctly
  file(TOUCH "${_cybld}/thrift/__init__.pxd")
  file(TOUCH "${_cybld}/thrift/python/__init__.pxd")
  file(TOUCH "${_cybld}/thrift/py3/__init__.pxd")

  # Run Thrift compiler on metadata.thrift. thrift-python runtime depends on
  # apache.thrift.metadata package, created and installed later in setup.py.
  add_custom_target(
    generate_apache_thrift_metadata_python
    COMMAND
      thrift1 -I ${CMAKE_SOURCE_DIR} --out . --gen mstch_python
      ${CMAKE_CURRENT_SOURCE_DIR}/thrift/metadata.thrift
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

  # Create symlinks in the cybld directory for cython extensions
  add_custom_target(create_binding_symlink_python_types ALL)
  foreach(
    _filerelpath
      "python/types.pxd"
      "python/types.pyx"
      "python/util.pxd"
      "python/util.pyx"
      "py3/stream.pxd"
      "py3/stream.pyx"
  )
    set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${_filerelpath}")
    get_filename_component(_src_dir "${_src}" DIRECTORY)
    file(RELATIVE_PATH _filereldir ${CMAKE_CURRENT_SOURCE_DIR} ${_src_dir})
    get_filename_component(_target_file "${_src}" NAME)


    message(
      STATUS
      "Linking ${_src} to ${_cybld}/thrift/${_filereldir}/_${_target_file}"
    )

    add_custom_command(
      TARGET
      create_binding_symlink_python_types
      PRE_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        "${_src}" "${_cybld}/thrift/${_filereldir}/_${_target_file}"
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

  add_custom_target(create_binding_symlink_python ALL)
  file(GLOB BindingFiles
    "python/abstract_types.py"
    "python/client/client_wrapper.py"
    "python/client/__init__.py"
    "python/metadata.py"
    "python/client/async_client_factory.pxd"
    "python/client/async_client.pxd"
    "python/client/omni_client.pxd"
    "python/client/request_channel.pxd"
    "python/client/ssl.pxd"
    "python/client/sync_channel_factory.pxd"
    "python/client/sync_client.pxd"
    "python/common.pxd"
    "python/converter.pxd"
    "python/exceptions.pxd"
    "python/flags.pxd"
    "python/mutable_containers.pxd"
    "python/mutable_exceptions.pxd"
    "python/mutable_serializer.pxd"
    "python/mutable_typeinfos.pxd"
    "python/mutable_types.pxd"
    "python/protocol.pxd"
    "python/serializer.pxd"
    "python/server/server.pxd"
    "python/std_libcpp.pxd"
    "python/stream.pxd"
    "python/types.pxd"
    "python/util.pxd"
    "python/adapter.pyx"
    "python/client/async_client_factory.pyx"
    "python/client/async_client.pyx"
    "python/client/omni_client.pyx"
    "python/client/request_channel.pyx"
    "python/client/ssl.pyx"
    "python/client/sync_channel_factory.pyx"
    "python/client/sync_client_factory.pyx"
    "python/client/sync_client.pyx"
    "python/common.pyx"
    "python/converter.pyx"
    "python/exceptions.pyx"
    "python/flags.pyx"
    "python/mutable_containers.pyx"
    "python/mutable_exceptions.pyx"
    "python/mutable_serializer.pyx"
    "python/mutable_typeinfos.pyx"
    "python/mutable_types.pyx"
    "python/protocol.pyx"
    "python/serializer.pyx"
    "python/stream.pyx"
    "python/server/server.pyx"
    "python/util.pyx"
    "python/flags.h"
    "python/types.h"
    "python/Serializer.h"
    "python/server/PythonAsyncProcessor.cpp"
    "python/server/PythonAsyncProcessorFactory.cpp"
    "python/server/flagged/EnableResourcePoolsForPython.cpp"
  )

  foreach(_src ${BindingFiles})
    file(RELATIVE_PATH _filerelpath ${CMAKE_CURRENT_SOURCE_DIR} ${_src})
    get_filename_component(_target_file "${_src}" NAME)

    message(
      STATUS
      "Linking ${_src} to ${_cybld}/thrift/${_filerelpath}"
    )

    add_custom_command(
      TARGET
      create_binding_symlink_python
      PRE_BUILD
        COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        "${_src}" "${_cybld}/thrift/${_filerelpath}"
    )
  endforeach()

  add_custom_target(create_binding_symlink_py3 ALL)
  file(GLOB BindingFiles
    "${CMAKE_CURRENT_SOURCE_DIR}/py3/*.pxd"
    "${CMAKE_CURRENT_SOURCE_DIR}/py3/*.pyx"
  )

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
        ${CMAKE_COMMAND} -E create_symlink
        "${_src}" "${_cybld}/thrift/py3/${_target_file}"
    )
  endforeach()

  #####
  # Compile a few *_api.h headers first.
  #####
  add_custom_target(
    thrift_python_types_bindings ALL
    DEPENDS
      create_binding_symlink_python_types
      create_binding_symlink_python
      create_binding_symlink_py3
      thriftcpp2
    WORKING_DIRECTORY ${_cybld}
  )
  add_custom_command(TARGET thrift_python_types_bindings
    COMMAND ${CMAKE_COMMAND} -E env
      "CFLAGS=${CMAKE_C_FLAGS}"
      "CXXFLAGS=${CMAKE_CXX_FLAGS}"
      "CXX=${CMAKE_CXX_COMPILER}"
      python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py -v --api-only
    BYPRODUCTS
      ${_cybld}/thrift/python/types_api.h
      ${_cybld}/thrift/python/server/server_api.h
      ${_cybld}/thrift/python/util_api.h
    WORKING_DIRECTORY ${_cybld}
  )

  # After cython runs, link _types_api.h to types_api.h
  add_custom_target(
    create_cython_types_api_symlink ALL
    DEPENDS thrift_python_types_bindings
    WORKING_DIRECTORY ${_cybld})
  file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/")
  file(MAKE_DIRECTORY "${_cybld}/thrift/lib/python/server")
  file(MAKE_DIRECTORY "${_cybld}/thrift/lib/py3/")

  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/python/_types_api.h"
      "${_cybld}/thrift/lib/python/types_api.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/python/_types.h"
      "${_cybld}/thrift/lib/python/_types.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/python/server_api.h"
      "${_cybld}/thrift/lib/python/server/server_api.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/python/_util_api.h"
      "${_cybld}/thrift/lib/python/util_api.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/python/_util.h"
      "${_cybld}/thrift/lib/python/_util.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/py3/_stream_api.h"
      "${_cybld}/thrift/lib/py3/stream_api.h"
  )
  add_custom_command(
    TARGET
    create_cython_types_api_symlink
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      "${_cybld}/thrift/py3/_stream.h"
      "${_cybld}/thrift/lib/py3/_stream.h"
  )
  add_library(
    thrift_python_cpp
      python/client/OmniClient.cpp
      python/client/RequestChannel.cpp
      python/client/ssl.cpp
      python/types.cpp
      python/util.cpp
      python/Serializer.cpp
      py3/stream.cpp
  )
  add_dependencies(thrift_python_cpp create_cython_types_api_symlink)
  set_property(TARGET thrift_python_cpp PROPERTY VERSION ${PACKAGE_VERSION})
  target_compile_definitions(thrift_python_cpp PRIVATE BOOST_NO_AUTO_PTR)
  target_compile_definitions(
    thrift_python_cpp PRIVATE THRIFT_NO_HTTP_CLIENT_CHANNEL)
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
  )
  install(
    TARGETS thrift_python_cpp
    DESTINATION ${py_install_dir}
  )

  #####
  # Now build everything else in lib/python, including all remaining Cython
  # modules that depend on types_api.h and thrift_python_cpp.
  #####
  add_custom_target(
    thrift_python_and_py3_bindings ALL
    DEPENDS
      generate_apache_thrift_metadata_python
      create_binding_symlink_python
      create_binding_symlink_py3
      thriftcpp2
      thrift_python_cpp
    WORKING_DIRECTORY ${_cybld}
  )

  set(inc_dirs "$<TARGET_PROPERTY:thrift,INCLUDE_DIRECTORIES>")
  get_target_property(fmt_include_dirs fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(wangle_incs wangle::wangle INTERFACE_INCLUDE_DIRECTORIES)
  get_target_property(thrift_include_dirs thrift INCLUDE_DIRECTORIES)
  list(JOIN wangle_incs ":" wangle_include_dirs)
  set(incs
    "-I$<JOIN:${inc_dirs},:>:${FOLLY_INCLUDE_DIR}:${Boost_INCLUDE_DIRS}"
    ":${fmt_include_dirs}:${wangle_include_dirs}"
    ":${CMAKE_BINARY_DIR}/thrift/lib/cybld")
  get_target_property(fmt_lib fmt::fmt LOCATION)
  get_filename_component(fmt_lib_dir ${fmt_lib} DIRECTORY)
  get_filename_component(folly_lib ${FOLLY_LIBRARY} DIRECTORY)
  get_filename_component(glog_lib ${GLOG_LIBRARIES} DIRECTORY)
  get_filename_component(python_lib ${Python3_LIBRARIES} DIRECTORY)
  get_filename_component(python_libname ${Python3_LIBRARIES} NAME)
  set(libs
    "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${fmt_lib_dir}:${folly_lib}"
    ":${glog_lib}:${python_lib}")
  set(py_install_dir ${CMAKE_INSTALL_PREFIX})
  if (NOT ${PYTHON_PACKAGE_INSTALL_DIR} STREQUAL "")
    set(py_install_dir ${PYTHON_PACKAGE_INSTALL_DIR})
  endif()

  add_custom_command(TARGET thrift_python_and_py3_bindings
    COMMAND ${CMAKE_COMMAND} -E env
      "CFLAGS=${CMAKE_C_FLAGS}"
      "CXXFLAGS=${CMAKE_CXX_FLAGS}"
      "CXX=${CMAKE_CXX_COMPILER}"
      python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py -v
      build_ext -f ${incs} ${libs} --libpython ${python_libname}
    WORKING_DIRECTORY ${_cybld}
  )
  add_custom_command(TARGET thrift_python_and_py3_bindings
    COMMAND ${CMAKE_COMMAND} -E env
      "CFLAGS=${CMAKE_C_FLAGS}"
      "CXXFLAGS=${CMAKE_CXX_FLAGS}"
      "CXX=${CMAKE_CXX_COMPILER}"
      python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py -v
      bdist_wheel --libpython ${python_libname}
    WORKING_DIRECTORY ${_cybld}
  )
  add_custom_command(TARGET thrift_python_and_py3_bindings POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env
      "CFLAGS=${CMAKE_C_FLAGS}"
      "CXXFLAGS=${CMAKE_CXX_FLAGS}"
      "CXX=${CMAKE_CXX_COMPILER}"
      python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py -v
      install --prefix ${CMAKE_INSTALL_PREFIX} --libpython ${python_libname}
    WORKING_DIRECTORY ${_cybld}
  )

  # getdeps.py does not allow sufficient control of CMAKE_INSTALL_PREFIX,
  # so PYTHON_PACKAGE_INSTALL_DIR can be used to control the installation
  # location of Python packages.
  set(ENV{CXX} ${CMAKE_CXX_COMPILER})
  install(CODE "
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E env
        \"CFLAGS=${CMAKE_C_FLAGS}\"
        \"CXXFLAGS=${CMAKE_CXX_FLAGS}\"
        python3 ${CMAKE_CURRENT_SOURCE_DIR}/setup.py install -v
        --prefix ${py_install_dir} --libpython ${python_libname}
      COMMAND_ECHO STDOUT
      WORKING_DIRECTORY ${_cybld}
    )
  ")
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
