# Copyright (c) 2010, 2018, Oracle and/or its affiliates. All rights reserved.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License, version 2.0,
#  as published by the Free Software Foundation.
#
#  This program is also distributed with certain software (including
#  but not limited to OpenSSL) that is licensed under separate terms,
#  as designated in a particular file or component or in included license
#  documentation.  The authors of MySQL hereby grant you an additional
#  permission to link the program and your derivative works with the
#  separately licensed software that they have included with MySQL.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License, version 2.0, for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

IF(NOT WIN32)
  RETURN()
ENDIF()

INCLUDE(${CMAKE_SOURCE_DIR}/packaging/WiX/wix_setup.cmake)

IF(NOT WIX_DIR)
  RETURN()
ENDIF()

SET(CPACK_WIX_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/CPackWixConfig.cmake)
SET(CPACK_MSI_SETUP ${CMAKE_SOURCE_DIR}/packaging/WiX/cpack_msi_setup.cmake)

SET(LICENSE_FILE "${CMAKE_SOURCE_DIR}/router/LICENSE.router")
CREATE_WIX_LICENCE_AND_RTF(${LICENSE_FILE})

SET(CPACK_WIX_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/mysql_router_extra.wxs;${CMAKE_CURRENT_SOURCE_DIR}/custom_ui.wxs")

ADD_CUSTOM_TARGET(
  configure_mysql_router_extra ALL
  COMMAND ${CMAKE_COMMAND}
  -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/mysqlrouter.conf.in
  -DOUTPUT_FILE=${PROJECT_BINARY_DIR}/stage/${CMAKE_CFG_INTDIR}/etc/mysqlrouter.conf.sample
  -P ${CMAKE_CURRENT_SOURCE_DIR}/../configure_file.cmake
)

INSTALL(FILES "${PROJECT_BINARY_DIR}/stage/\${CMAKE_INSTALL_CONFIG_NAME}/etc/mysqlrouter.conf.sample"
  DESTINATION "etc" COMPONENT Router) 

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mysql_router_extra.wxs.in
  ${CMAKE_CURRENT_BINARY_DIR}/mysql_router_extra.wxs)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/create_msi.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/create_msi.cmake
  @ONLY)

IF(CMAKE_GENERATOR MATCHES "Visual Studio")
 SET(CONFIG_PARAM "-DCMAKE_INSTALL_CONFIG_NAME=${CMAKE_CFG_INTDIR}")
ENDIF()

ADD_CUSTOM_TARGET(
  MSI-Router
  COMMAND set VS_UNICODE_OUTPUT=
  COMMAND ${CMAKE_COMMAND}
  ${CONFIG_PARAM}
  -P  ${CMAKE_CURRENT_BINARY_DIR}/create_msi.cmake
)
ADD_DEPENDENCIES(MSI-Router configure_mysql_router_extra)
