# Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.
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

DISABLE_MISSING_PROFILE_WARNING()

ADD_SUBDIRECTORY(sys_schema)

MYSQL_ADD_EXECUTABLE(comp_sql comp_sql.cc SKIP_INSTALL)

# Add compiler information, for easier debugging of output from mysql_config.
SET(COMPILER_ID_AND_VERSION
  "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# Build mysql_fix_privilege_tables.sql (concatenate 2 sql scripts)
IF(NOT WIN32 OR CMAKE_CROSSCOMPILING)
  FIND_PROGRAM(CAT_EXECUTABLE cat DOC "path to the executable")
  MARK_AS_ADVANCED(CAT_EXECUTABLE)
ENDIF()

IF(CAT_EXECUTABLE)
  SET(CAT_COMMAND COMMAND
    ${CMAKE_COMMAND} -E chdir  ${CMAKE_CURRENT_SOURCE_DIR}
    ${CAT_EXECUTABLE} mysql_system_tables.sql mysql_system_tables_fix.sql >
    ${CMAKE_CURRENT_BINARY_DIR}/mysql_fix_privilege_tables.sql
  )
ELSEIF(WIN32)
  FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/mysql_fix_privilege_tables.sql
    native_outfile )
  SET(CAT_COMMAND
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}
    cmd /c copy /b mysql_system_tables.sql + mysql_system_tables_fix.sql
    ${native_outfile} )
ELSE()
  MESSAGE(FATAL_ERROR "Cannot concatenate files")
ENDIF()

# Build mysql_fix_privilege_tables.h
ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mysql_fix_privilege_tables_sql.h
  ${CAT_COMMAND}
  COMMAND comp_sql
  mysql_fix_privilege_tables
  mysql_fix_privilege_tables.sql
  mysql_fix_privilege_tables_sql.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables.sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables_fix.sql
)

# Add target for the above to be built
ADD_CUSTOM_TARGET(GenFixPrivs
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mysql_fix_privilege_tables_sql.h
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_sys_schema.h
  COMMAND comp_sql
  mysql_sys_schema
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_sys_schema.sql
  sql_commands_sys_schema.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_sys_schema.sql
  )

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_data.h
  COMMAND comp_sql
  mysql_system_data
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables_data.sql
  sql_commands_system_data.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables_data.sql
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_tables_data_fix.h
  COMMAND comp_sql
  mysql_system_tables_data_fix
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables_data_fix.sql
  sql_commands_system_tables_data_fix.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables_data_fix.sql
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_tables.h
  COMMAND comp_sql
  mysql_system_tables
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables.sql
  sql_commands_system_tables.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_tables.sql
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_users.h
  COMMAND comp_sql
  mysql_system_users
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_users.sql
  sql_commands_system_users.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/mysql_system_users.sql
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_help_data.h
  COMMAND comp_sql
  fill_help_tables
  ${CMAKE_CURRENT_SOURCE_DIR}/fill_help_tables.sql
  sql_commands_help_data.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS comp_sql
  ${CMAKE_CURRENT_SOURCE_DIR}/fill_help_tables.sql
)

# Add target for the above to be built
IF(NOT WITHOUT_SERVER)
  ADD_CUSTOM_TARGET(GenBootstrapPriv
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_data.h
    ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_tables.h
    ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_users.h
    ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_help_data.h
    ${CMAKE_CURRENT_BINARY_DIR}/sql_commands_system_tables_data_fix.h
  )
ENDIF()

IF(UNIX)
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_binary_distribution
    "cd ${CMAKE_BINARY_DIR} && '${CMAKE_CPACK_COMMAND}' -G TGZ --config CPackConfig.cmake\n" )
  EXECUTE_PROCESS(
    COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/make_binary_distribution
)
ENDIF()

# TCMalloc hacks
IF(MALLOC_LIB)
  MESSAGE("Using tcmalloc '${MALLOC_LIB}'")
  INSTALL(FILES ${MALLOC_LIB} DESTINATION ${INSTALL_LIBDIR} OPTIONAL)
ENDIF()

SET(COMPILE_DEFINITIONS_WHITELIST
)

# In practice: only in use on Solaris
SET(CMAKE_C_FLAGS_WHITELIST
  "-m32"
  "-m64"
)

SET(CFLAGS "")
SET(CXXFLAGS "")

GET_PROPERTY(cwd_definitions DIRECTORY PROPERTY COMPILE_DEFINITIONS)
FOREACH(white_flag ${COMPILE_DEFINITIONS_WHITELIST})
  FOREACH(flag ${cwd_definitions})
    IF (${flag} MATCHES ${white_flag})
      SET(CFLAGS "-D${white_flag} ${CFLAGS}")
      SET(CXXFLAGS "-D${white_flag} ${CXXFLAGS}")
    ENDIF()
  ENDFOREACH()
ENDFOREACH()

IF(CMAKE_BUILD_TYPE AND BUILD_IS_SINGLE_CONFIG)
  SET(CMAKEBT_FLAGS
    "${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} ${CMAKE_C_FLAGS}")
  SEPARATE_ARGUMENTS(CMAKEBT_FLAGS)
ENDIF()

FOREACH(white_flag ${CMAKE_C_FLAGS_WHITELIST})
  FOREACH(flag ${CMAKEBT_FLAGS})
    IF (${flag} STREQUAL ${white_flag})
      SET(CFLAGS "${white_flag} ${CFLAGS}")
      SET(CXXFLAGS "${white_flag} ${CXXFLAGS}")
    ENDIF()
  ENDFOREACH()
ENDFOREACH()

# Do not remove -std=c++14 for Oracle Studio as it is required to get
# the correct libraries. Only remove for GCC/Clang
IF(MY_COMPILER_IS_GNU_OR_CLANG)
  STRING(REGEX REPLACE "-std=c..[0-9]+" "" CXXFLAGS "${CXXFLAGS}")
ENDIF()

IF(FREEBSD)
  SET (PERL_PATH "/usr/local/bin/perl")
ELSE()
  SET (PERL_PATH "/usr/bin/perl")
ENDIF()

IF(UNIX)
  # SHELL_PATH, FIND_PROC, CHECK_PID are used by mysqld_safe
  IF(SOLARIS)
    SET (SHELL_PATH "/bin/bash")
  ELSE()
    SET (SHELL_PATH "/bin/sh")
  ENDIF()

  IF(LINUX)
    SET (FIND_PROC
      "ps wwwp $PID | grep -v mysqld_safe | grep -- $MYSQLD > /dev/null")
  ENDIF()
  IF(SOLARIS)
    SET (FIND_PROC
      "ps -p $PID | grep -v mysqld_safe | grep -- $MYSQLD > /dev/null")
  ENDIF()

  IF(NOT FIND_PROC)
    # BSD style
    EXECUTE_PROCESS(COMMAND ps -uaxww
      OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE result)
    IF(result MATCHES 0)
      SET( FIND_PROC
        "ps -uaxww | grep -v mysqld_safe | grep -- $MYSQLD | grep $PID > /dev/null")
    ENDIF()
  ENDIF()

  IF(NOT FIND_PROC)
    # SysV style
    EXECUTE_PROCESS(COMMAND ps -ef
      OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE result)
    IF(result MATCHES 0)
      SET( FIND_PROC "ps -ef | grep -v mysqld_safe | grep -- $MYSQLD | grep $PID > /dev/null")
    ENDIF()
  ENDIF()

  EXECUTE_PROCESS(COMMAND sh -c "kill -0 $$"
    OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE result)
  IF(result MATCHES 0)
    SET(CHECK_PID "kill -0 $PID > /dev/null 2> /dev/null")
  ELSE()
    SET(CHECK_PID "kill -s SIGCONT $PID  > /dev/null 2> /dev/null")
  ENDIF()

  SET(MYSQLD_USER "mysql")

ENDIF(UNIX)

SET(prefix "${CMAKE_INSTALL_PREFIX}")
SET(sysconfdir ${prefix})
SET(bindir ${prefix}/${INSTALL_BINDIR})
SET(libexecdir ${prefix}/${INSTALL_SBINDIR})
SET(datadir ${prefix}/${INSTALL_MYSQLSHAREDIR})
SET(libsubdir  ${INSTALL_LIBDIR})
SET(pkgincludedir ${prefix}/${INSTALL_INCLUDEDIR})
SET(pkglibdir ${prefix}/${INSTALL_LIBDIR})
SET(pkgplugindir ${prefix}/${INSTALL_PLUGINDIR})
SET(localstatedir ${MYSQL_DATADIR})

IF(SOLARIS)
  SET(RPATH_OPTION " -R$pkglibdir")
ENDIF()

IF(LINUX_INSTALL_RPATH_ORIGIN)
  SET(RPATH_OPTION " -Wl,-R,$pkglibdir")    # for mysql_config
  SET(RPATH_OPTION_PC " -Wl,-R,\${libdir}") # for pkg_config
  # Do not use STRING_APPEND, it may eval the $.
  SET(RPATH_OPTION "${RPATH_OPTION} -L$pkglibdir/private")
  SET(RPATH_OPTION_PC "${RPATH_OPTION_PC} -L\${libdir}/private")
  SET(RPATH_OPTION "${RPATH_OPTION} -Wl,-R,$pkglibdir/private")
  SET(RPATH_OPTION_PC "${RPATH_OPTION_PC} -Wl,-R,\${libdir}/private")
ENDIF()

# some scripts use  @TARGET_LINUX@
IF(LINUX)
  SET(TARGET_LINUX "true")
ELSE()
  SET(TARGET_LINUX "false")
ENDIF()

# Use cmake variables to inspect dependencies for
# mysqlclient library (add -l stuff)
SET(CLIENT_LIBS "")
SET(LIBS "")

# Extract dependencies using CMake's internal ${target}_LIB_DEPENDS variable
# returned string in ${var} is can be passed to linker's command line
MACRO(EXTRACT_LINK_LIBRARIES target var)
  IF(${target}_LIB_DEPENDS)
    LIST(REMOVE_ITEM ${target}_LIB_DEPENDS "")
    LIST(REMOVE_DUPLICATES ${target}_LIB_DEPENDS)
    FOREACH(lib ${${target}_LIB_DEPENDS})
      # Filter out "general", it is not a library, just CMake hint
      IF(NOT lib STREQUAL "general" AND NOT ${var}  MATCHES "-l${lib} ")
        IF (lib MATCHES "^\\-l")
          SET(${var} "${${var}} ${lib} ")
        ELSEIF (lib MATCHES "^\\-L")
          SET(${var} "${${var}} ${lib} ")
        ELSEIF(lib MATCHES "^/")
          # Full path, convert to just filename, strip "lib" prefix and extension
          GET_FILENAME_COMPONENT(lib "${lib}" NAME_WE)
          STRING(REGEX REPLACE "^lib" "" lib "${lib}")
          SET(${var} "${${var}}-l${lib} " )
        ELSE()
          SET(${var} "${${var}}-l${lib} " )
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()
  IF(MSVC)
    STRING(REPLACE "-l" "" ${var} "${${var}}")
  ENDIF()
  STRING(REGEX REPLACE "^[ ]+" "" ${var} "${${var}}")
  STRING(REGEX REPLACE "[ ]+$" "" ${var} "${${var}}")
ENDMACRO()

EXTRACT_LINK_LIBRARIES(mysqlclient CLIENT_LIBS)

IF(MSVC)
  GET_TARGET_PROPERTY(LIBMYSQL_OS_SHLIB_VERSION mysqlclient VERSION)
  GET_TARGET_PROPERTY(LIBMYSQL_OS_OUTPUT_NAME mysqlclient OUTPUT_NAME)
ELSE()
  GET_TARGET_PROPERTY(LIBMYSQL_OS_SHLIB_VERSION libmysql VERSION)
  GET_TARGET_PROPERTY(LIBMYSQL_OS_OUTPUT_NAME libmysql OUTPUT_NAME)
ENDIF()

SET(CONFIG_CLIENT_LIBS "${CLIENT_LIBS}")
SET(CONFIG_LIBS_PRIVATE "${CLIENT_LIBS}")
SET(CONFIG_REQUIRES_PRIVATE "")
# ssl.cmake should have used pkg-config rather than cmake to locate ssl
IF (WITH_SSL STREQUAL "system")
  # We have implemented "system" for other platforms as well, but those
  # are non-native packages.
  IF(LINUX OR SOLARIS OR FREEBSD)
    SET(CONFIG_REQUIRES_PRIVATE "openssl")
    STRING(REPLACE "-lssl" "" CONFIG_LIBS_PRIVATE "${CONFIG_LIBS_PRIVATE}")
    STRING(REPLACE "-lcrypto" "" CONFIG_LIBS_PRIVATE "${CONFIG_LIBS_PRIVATE}")
    STRING(REGEX REPLACE "[ ]+" " " CONFIG_LIBS_PRIVATE
      "${CONFIG_LIBS_PRIVATE}")
  ENDIF()
ENDIF()

MESSAGE(STATUS "CONFIG_CLIENT_LIBS ${CONFIG_CLIENT_LIBS}")
MESSAGE(STATUS "CONFIG_LIBS_PRIVATE ${CONFIG_LIBS_PRIVATE}")

IF(WIN32)

  SET(BIN_SCRIPTS
    mysqld_multi
    mysqldumpslow
    )

  FOREACH(file ${BIN_SCRIPTS})
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file}.pl.in
      ${CMAKE_CURRENT_BINARY_DIR}/${file}.pl ESCAPE_QUOTES @ONLY
      )
    INSTALL_SCRIPT(${CMAKE_CURRENT_BINARY_DIR}/${file}.pl
      COMPONENT Server_Scripts
      )
  ENDFOREACH()

ELSE()

  SET(PKGCONFIG_FILE ${LIBMYSQL_OS_OUTPUT_NAME}.pc)
  STRING(REGEX REPLACE "/mysql$" "" PKGCONFIG_DIR "${INSTALL_LIBDIR}")
  SET(PKGCONFIG_DIR "${PKGCONFIG_DIR}/pkgconfig")
  # Update the cache, to make it visible in cmake-gui.
  SET(INSTALL_PKGCONFIGDIR ${INSTALL_PKGCONFIGDIR} CACHE PATH
    "Where to install ${PKGCONFIG_FILE}, defaults to ${PKGCONFIG_DIR}")
  IF(NOT INSTALL_PKGCONFIGDIR)
    SET(INSTALL_PKGCONFIGDIR "${PKGCONFIG_DIR}")
  ENDIF()

  CONFIGURE_FILE(mysqlclient.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PKGCONFIG_FILE}
    ESCAPE_QUOTES @ONLY)

  IF(INSTALL_PKGCONFIGDIR)
    MESSAGE(STATUS "INSTALL ${PKGCONFIG_FILE} ${INSTALL_PKGCONFIGDIR}")
    INSTALL(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${PKGCONFIG_FILE}
      DESTINATION ${INSTALL_PKGCONFIGDIR}
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      COMPONENT Development
      )
  ENDIF()

  # mysqld_safe used in mtr even for systemd platforms
  IF(WITH_SYSTEMD)
    SET(BIN_SCRIPTS
      mysql_config
      mysqldumpslow
      mysqld_safe
      )
  ELSE()
    SET(BIN_SCRIPTS
      mysql_config
      mysqldumpslow
      mysqld_multi
      mysqld_safe
      )
  ENDIF()

  FOREACH(file ${BIN_SCRIPTS})
    IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.sh)
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file}.sh
        ${CMAKE_CURRENT_BINARY_DIR}/${file} ESCAPE_QUOTES @ONLY
        )
    ELSEIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${file}.pl.in)
      CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file}.pl.in
        ${CMAKE_CURRENT_BINARY_DIR}/${file} ESCAPE_QUOTES @ONLY
        )
    ELSE()
      MESSAGE(FATAL_ERROR "Can not find ${file}.sh or ${file}.pl.in in "
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )
    ENDIF()
    EXECUTE_PROCESS(COMMAND chmod +x ${CMAKE_CURRENT_BINARY_DIR}/${file})

    IF(${file} STREQUAL "mysql_config")
      SET(file_COMPONENT Development)
    ELSE()
      SET(file_COMPONENT Server)
    ENDIF()

    INSTALL_SCRIPT(
      ${CMAKE_CURRENT_BINARY_DIR}/${file}
      DESTINATION ${INSTALL_BINDIR}
      COMPONENT ${file_COMPONENT}
     )
  ENDFOREACH()

  # Systemd files
  IF(WITH_SYSTEMD)
    CONFIGURE_FILE(systemd/mysqld_pre_systemd.in
      ${CMAKE_CURRENT_BINARY_DIR}/mysqld_pre_systemd ESCAPE_QUOTES @ONLY)

    CONFIGURE_FILE(systemd/mysqld.service.in
      ${CMAKE_CURRENT_BINARY_DIR}/${SYSTEMD_SERVICE_NAME}.service
      ESCAPE_QUOTES @ONLY)

    CONFIGURE_FILE(systemd/mysqld@.service.in
      ${CMAKE_CURRENT_BINARY_DIR}/${SYSTEMD_SERVICE_NAME}@.service
      ESCAPE_QUOTES @ONLY)

    CONFIGURE_FILE(systemd/mysqld.tmpfiles.d.in
      ${CMAKE_CURRENT_BINARY_DIR}/mysql.conf ESCAPE_QUOTES @ONLY)

    INSTALL_SCRIPT(
      ${CMAKE_CURRENT_BINARY_DIR}/mysqld_pre_systemd
      DESTINATION ${INSTALL_BINDIR}
      COMPONENT Server
    )

    IF(INSTALL_LAYOUT MATCHES "STANDALONE")
      SET(SYSTEMD_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/")
    ELSE()
      SET(SYSTEMD_INSTALL_PREFIX "")
    ENDIF()

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SYSTEMD_SERVICE_NAME}.service
      DESTINATION ${SYSTEMD_INSTALL_PREFIX}${SYSTEMD_SERVICES_DIR}
      COMPONENT Server
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    )
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SYSTEMD_SERVICE_NAME}@.service
      DESTINATION ${SYSTEMD_INSTALL_PREFIX}${SYSTEMD_SERVICES_DIR}
      COMPONENT Server
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    )
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mysql.conf
      DESTINATION ${SYSTEMD_INSTALL_PREFIX}${SYSTEMD_TMPFILES_DIR}
      COMPONENT Server
      PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    )
  ENDIF()
ENDIF()
