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

ADD_WSHADOW_WARNING()

# We are not interesting in profiling clients.
DISABLE_MISSING_PROFILE_WARNING()

## Subdirectory with common client code.
ADD_SUBDIRECTORY(base)
## Subdirectory for mysqlpump code.
ADD_SUBDIRECTORY(dump)

MYSQL_ADD_EXECUTABLE(mysql
  ${CMAKE_SOURCE_DIR}/sql-common/net_ns.cc
  completion_hash.cc
  mysql.cc
  pattern_matcher.cc
  readline.cc
  LINK_LIBRARIES mysqlclient client_base ${EDITLINE_LIBRARY}
  )

IF(NOT WITHOUT_SERVER)
  MYSQL_ADD_EXECUTABLE(mysql_upgrade
    upgrade/program.cc
    LINK_LIBRARIES mysqlclient client_base
    )
ENDIF()

# Enable inlining, to avoid stack overflow for some long regular expressions.
IF(SOLARIS_SPARC AND MY_COMPILER_IS_SUNPRO AND
    CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
  ADD_COMPILE_FLAGS(mysqltest/regular_expressions.cc
    COMPILE_FLAGS "-xO3"
    )
ENDIF()

MYSQL_ADD_EXECUTABLE(mysqltest
  mysqltest.cc
  mysqltest/error_names.cc
  mysqltest/expected_errors.cc
  mysqltest/expected_warnings.cc
  mysqltest/logfile.cc
  mysqltest/regular_expressions.cc
  mysqltest/secondary_engine.cc
  mysqltest/utils.cc

  COMPONENT Test
  DEPENDENCIES GenError GenClientError
  ENABLE_EXPORTS
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysqlcheck
  check/mysqlcheck.cc
  check/mysqlcheck_core.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysqldump
  mysqldump.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysqlimport
  mysqlimport.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysqlshow
  mysqlshow.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysqlbinlog
  mysqlbinlog.cc
  ${CMAKE_SOURCE_DIR}/sql/json_binary.cc
  ${CMAKE_SOURCE_DIR}/sql/json_dom.cc
  ${CMAKE_SOURCE_DIR}/sql/json_syntax_check.cc
  ${CMAKE_SOURCE_DIR}/sql/log_event.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_utility.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_gtid_sid_map.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_gtid_misc.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_gtid_set.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_gtid_specification.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_tblmap.cc
  ${CMAKE_SOURCE_DIR}/sql/basic_istream.cc
  ${CMAKE_SOURCE_DIR}/sql/binlog_istream.cc
  ${CMAKE_SOURCE_DIR}/sql/binlog_reader.cc
  ${CMAKE_SOURCE_DIR}/sql/stream_cipher.cc
  ${CMAKE_SOURCE_DIR}/sql/rpl_log_encryption.cc
  ${CMAKE_SOURCE_DIR}/libbinlogevents/src/trx_boundary_parser.cpp

  DEPENDENCIES GenError
  LINK_LIBRARIES mysqlclient client_base binlogevents_static
  )

TARGET_COMPILE_DEFINITIONS(mysqlbinlog PRIVATE DISABLE_PSI_MUTEX)
TARGET_INCLUDE_DIRECTORIES(mysqlbinlog PRIVATE ${CMAKE_SOURCE_DIR}/sql)

MYSQL_ADD_EXECUTABLE(mysqladmin
  mysqladmin.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysqlslap
  mysqlslap.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysql_config_editor
  mysql_config_editor.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysql_secure_installation
  mysql_secure_installation.cc
  LINK_LIBRARIES mysqlclient
  )
MYSQL_ADD_EXECUTABLE(mysql_ssl_rsa_setup
  logger.cc
  mysql_ssl_rsa_setup.cc
  path.cc
  LINK_LIBRARIES mysys
  )

# "WIN32" also covers 64 bit. "echo" is used in some files below "mysql-test/".
IF(WIN32)
  MYSQL_ADD_EXECUTABLE(echo echo.cc)
ENDIF(WIN32)
