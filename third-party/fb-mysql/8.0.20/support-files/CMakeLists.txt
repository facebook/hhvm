# Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.
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

IF(NOT UNIX)
  RETURN()
ENDIF()

INSTALL(FILES mysql.m4
  DESTINATION ${INSTALL_SHAREDIR}/aclocal COMPONENT Development)

SET(localstatedir "${MYSQL_DATADIR}")

SET(prefix "${CMAKE_INSTALL_PREFIX}")
SET(bindir ${prefix}/${INSTALL_BINDIR})
SET(sbindir ${prefix}/${INSTALL_SBINDIR})

IF(LINUX)
  SET (PIDOF "pidof")
ELSE()
  SET (PIDOF "pgrep -d' ' -f")
ENDIF()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mysql.server.sh
  ${CMAKE_CURRENT_BINARY_DIR}/mysql.server @ONLY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mysql-log-rotate.in
  ${CMAKE_CURRENT_BINARY_DIR}/mysql-log-rotate @ONLY)

INSTALL_SCRIPT(${CMAKE_CURRENT_BINARY_DIR}/mysql.server
  COMPONENT Server_Scripts
  DESTINATION ${INSTALL_SUPPORTFILESDIR}
  )

INSTALL_SCRIPT(${CMAKE_CURRENT_SOURCE_DIR}/mysqld_multi.server
  COMPONENT Server_Scripts
  DESTINATION ${INSTALL_SUPPORTFILESDIR}
  )

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mysql-log-rotate
  COMPONENT Server_Scripts
  DESTINATION ${INSTALL_SUPPORTFILESDIR}
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ WORLD_READ
  )
