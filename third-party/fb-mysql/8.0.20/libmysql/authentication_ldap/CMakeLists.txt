# Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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

#
# Configuration for building LDAP SASL Authentication Plugin (client-side)
#

# The client authentication plugin is part of the community build.
# Build it if we have found the necessary libraries.
IF(NOT SASL_FOUND OR NOT LDAP_FOUND)
  MESSAGE(WARNING "Skipping the LDAP client authentication plugin")
  RETURN()
ENDIF()

IF(FREEBSD AND KERBEROS_SYSTEM_LIBRARY)
  INCLUDE_DIRECTORIES(SYSTEM /usr/local/include)
ENDIF()

IF(FREEBSD AND SASL_INCLUDE_DIR)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM ${SASL_INCLUDE_DIR})
ENDIF()

INCLUDE(CheckIncludeFiles)

# Several sasl_() functions are deprecated
IF(APPLE)
  STRING_APPEND(CMAKE_CXX_FLAGS " -Wno-deprecated-declarations")
ENDIF()

DISABLE_MISSING_PROFILE_WARNING()

IF(KERBEROS_FOUND)
  MESSAGE(STATUS
    "Building authentication_ldap_sasl_client with Kerberos")
  IF(KERBEROS_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(SYSTEM ${KERBEROS_INCLUDE_DIR})
  ENDIF()
  SET(AUTH_LDAP_KERBEROS_CC auth_ldap_kerberos.cc)
  # mysys is required to use my_free and my_strdup when getting
  # Kerberos user name.
  # Adding mysys only when build has Kerberos support
  SET(MY_SYS "mysys")
ELSE()
  MESSAGE(STATUS
    "Building authentication_ldap_sasl_client WITHOUT Kerberos")
ENDIF()

MYSQL_ADD_PLUGIN(authentication_ldap_sasl_client
  auth_ldap_sasl_client.cc
  auth_ldap_sasl_mechanism.cc
  ${AUTH_LDAP_KERBEROS_CC}
  log_client.cc
  LINK_LIBRARIES
  ${SASL_LIBRARY}
  # Uncomment to verify that all symbols are found.
  # ${LINK_FLAG_NO_UNDEFINED}
  ${KERBEROS_LIBRARIES}
  ${SSL_LIBRARIES}
  ${LIBDL}
  ${MY_SYS}
  CLIENT_ONLY
  MODULE_ONLY
  MODULE_OUTPUT_NAME "authentication_ldap_sasl_client")

IF(LINUX_STANDALONE AND SASL_CUSTOM_LIBRARY)
  TARGET_COMPILE_DEFINITIONS(authentication_ldap_sasl_client
    PRIVATE SASL_CUSTOM_LIBRARY)
  ADD_DEPENDENCIES(authentication_ldap_sasl_client ${sasl_target})
ENDIF()

IF(LINUX_STANDALONE AND KERBEROS_CUSTOM_LIBRARY)
  ADD_DEPENDENCIES(authentication_ldap_sasl_client ${kerberos_target})
ENDIF()

IF(WIN32)
  GET_FILENAME_COMPONENT(SASL_DLL_NAME ${SASL_LIBRARY_DLL} NAME)
  GET_FILENAME_COMPONENT(SASL_SCRAM_PLUGIN_NAME "${SASL_SCRAM_PLUGIN}" NAME)

  MESSAGE(STATUS "SASL_INCLUDE_DIR  = ${SASL_INCLUDE_DIR}")
  MESSAGE(STATUS "SASL_LIBRARY_DLL  = ${SASL_LIBRARY_DLL}")
  MESSAGE(STATUS "SASL_SCRAM_PLUGIN = ${SASL_SCRAM_PLUGIN}")

  # Note that "libsasl.dll" and "saslSCRAM.dll" go into the "bin" directory
  # where "mysql.exe" and other client executables are located.
  INSTALL(FILES "${SASL_LIBRARY_DLL}"
          DESTINATION ${INSTALL_BINDIR}
          COMPONENT SharedLibraries)
  INSTALL(FILES "${SASL_SCRAM_PLUGIN}"
          DESTINATION ${INSTALL_BINDIR}
          COMPONENT SharedLibraries)

  # To run client executables that load the plug-in from the build tree we need
  # to copy the SASL library DLL and SASL SCRAM library DLL to the same
  # directory as the client executables.
  ADD_CUSTOM_COMMAND(TARGET authentication_ldap_sasl_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SASL_LIBRARY_DLL}"
      "${CMAKE_BINARY_DIR}/runtime_output_directory/${CMAKE_CFG_INTDIR}/${SASL_DLL_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SASL_SCRAM_PLUGIN}"
      "${CMAKE_BINARY_DIR}/runtime_output_directory/${CMAKE_CFG_INTDIR}/${SASL_SCRAM_PLUGIN_NAME}"
  )
  ADD_DEPENDENCIES(authentication_ldap_sasl_client mysqltest)

ENDIF()
