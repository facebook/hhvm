# Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.
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

# systemd support files shall be installed on linux if the build host machine
# has systemd package installed.  make install will install the systemd
# related configuration files into the systemd service files directories. To
# use systemd the system needs to be booted with init as systemd.

MACRO(MYSQL_CHECK_SYSTEMD)
  FIND_PACKAGE(PkgConfig QUIET)
  IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(SYSTEMD "systemd")

    IF(SYSTEMD_FOUND)
      EXECUTE_PROCESS(COMMAND ${PKG_CONFIG_EXECUTABLE}
                      --variable=systemdsystemunitdir systemd
                      OUTPUT_VARIABLE SYSTEMD_SERVICES_DIR)
      STRING(REGEX REPLACE "[ \t\n]+" ""
             SYSTEMD_SERVICES_DIR "${SYSTEMD_SERVICES_DIR}")
      IF("${SYSTEMD_SERVICES_DIR}" STREQUAL "")
        SET(SYSTEMD_SERVICES_DIR "/usr/lib/systemd/system")
      ENDIF()
      MESSAGE(STATUS "SYSTEMD_SERVICES_DIR ${SYSTEMD_SERVICES_DIR}")
      EXECUTE_PROCESS(COMMAND ${PKG_CONFIG_EXECUTABLE}
                      --variable=tmpfilesdir systemd
                      OUTPUT_VARIABLE SYSTEMD_TMPFILES_DIR)
      STRING(REGEX REPLACE "[ \t\n]+" ""
             SYSTEMD_TMPFILES_DIR "${SYSTEMD_TMPFILES_DIR}")
      IF ("${SYSTEMD_TMPFILES_DIR}" STREQUAL "")
        SET(SYSTEMD_TMPFILES_DIR "/usr/lib/tmpfiles.d")
      ENDIF()
      MESSAGE(STATUS "SYSTEMD_TMPFILES_DIR ${SYSTEMD_TMPFILES_DIR}")
    ELSE()
      MESSAGE(FATAL_ERROR "Unable to detect systemd support on build machine,\
                           Aborting cmake build.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR, "Unable to detect pkg-config support on build machine,\
                          Aborting cmake build.")
  ENDIF()

  IF("${SYSTEMD_SERVICE_NAME}" STREQUAL "")
    SET(SYSTEMD_SERVICE_NAME "mysqld")
  ENDIF()
  MESSAGE(STATUS "SYSTEMD_SERVICE_NAME ${SYSTEMD_SERVICE_NAME}")

  IF("${SYSTEMD_PID_DIR}" STREQUAL "")
    SET(SYSTEMD_PID_DIR "/var/run/mysqld")
  ENDIF()
  MESSAGE(STATUS "SYSTEMD_PID_DIR ${SYSTEMD_PID_DIR}")
ENDMACRO()

MESSAGE(STATUS "Enabling installation of systemd support files...")
MYSQL_CHECK_SYSTEMD()
