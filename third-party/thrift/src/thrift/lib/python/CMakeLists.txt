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

set(_cybld "${CMAKE_CURRENT_BINARY_DIR}/cybld")

add_custom_target(create_binding_symlink ALL)
file(GLOB BindingFiles
  "${CMAKE_CURRENT_SOURCE_DIR}/*.pyx"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.pxd"
)
file(MAKE_DIRECTORY "${_cybld}/thrift/python/")

foreach(_src ${BindingFiles})
  get_filename_component(_target_file "${_src}" NAME)

  message(
    STATUS
    "Linking ${_src} to ${_cybld}/thrift/python/${_target_file}"
  )

  add_custom_command(
    TARGET
    create_binding_symlink
    PRE_BUILD
      COMMAND
      ${CMAKE_COMMAND} -E create_symlink "${_src}" "${_cybld}/thrift/python/${_target_file}"
  )
endforeach()

set(prop "$<TARGET_PROPERTY:thrift,INCLUDE_DIRECTORIES>")
set(incs "$<$<BOOL:${prop}>:-I$<JOIN:${prop},:>>")
set(libs "-L${CMAKE_BINARY_DIR}")

add_custom_target(
  thrift_python_bindings ALL
  DEPENDS create_binding_symlink thriftcpp2
  WORKING_DIRECTORY ${_cybld})

add_custom_command(TARGET thrift_python_bindings POST_BUILD
  COMMAND
    python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
    build_ext -f ${incs} ${libs}
  WORKING_DIRECTORY ${_cybld}
)
