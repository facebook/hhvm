# Copyright (c) 2010, 2018, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

IF(NOT WIN32)
  RETURN()
ENDIF()

include(wix_setup.cmake)

IF(NOT WIX_DIR)
  RETURN()
ENDIF()

# WiX wants the license text as rtf; if there is no rtf license, 
# we create a fake one from the plain text LICENSE file.
CREATE_WIX_LICENCE_AND_RTF("${CMAKE_SOURCE_DIR}/LICENSE")

SET(CPACK_WIX_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/CPackWixConfig.cmake)
SET(CPACK_WIX_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/mysql_server_extra.wxs;${CMAKE_CURRENT_SOURCE_DIR}/custom_ui.wxs")
SET(CPACK_MSI_SETUP ${CMAKE_CURRENT_SOURCE_DIR}/cpack_msi_setup.cmake)

#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mysql_server_extra.wxs.in
#  ${CMAKE_CURRENT_BINARY_DIR}/mysql_server_extra.wxs)

ADD_CUSTOM_TARGET(
  configure_mysql_server_extra
  COMMAND ${CMAKE_COMMAND}
  -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/mysql_server_extra.wxs.in
  -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/mysql_server_extra.wxs
  -DMAJOR_VERSION=${MAJOR_VERSION}
  -DMINOR_VERSION=${MINOR_VERSION}
  -P ${CMAKE_CURRENT_SOURCE_DIR}/configure_file.cmake
)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/create_msi.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/create_msi.cmake
  @ONLY)

IF(CMAKE_GENERATOR MATCHES "Visual Studio")
 SET(CONFIG_PARAM "-DCMAKE_INSTALL_CONFIG_NAME=${CMAKE_CFG_INTDIR}")
ENDIF()

ADD_CUSTOM_TARGET(
  MSI
  COMMAND set VS_UNICODE_OUTPUT=
  COMMAND ${CMAKE_COMMAND} 
  ${CONFIG_PARAM} 
  -P  ${CMAKE_CURRENT_BINARY_DIR}/create_msi.cmake
)
ADD_DEPENDENCIES(MSI configure_mysql_server_extra)
