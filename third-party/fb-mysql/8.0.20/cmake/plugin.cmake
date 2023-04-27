# Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.
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


GET_FILENAME_COMPONENT(MYSQL_CMAKE_SCRIPT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
INCLUDE(${MYSQL_CMAKE_SCRIPT_DIR}/cmake_parse_arguments.cmake)

# MYSQL_ADD_PLUGIN(plugin_name source1...sourceN
# [STORAGE_ENGINE]
# [MANDATORY|DEFAULT]
# [STATIC_ONLY|MODULE_ONLY]
# [MODULE_OUTPUT_NAME module_name]
# [STATIC_OUTPUT_NAME static_name]
# [LINK_LIBRARIES lib1...libN]
# [DEPENDENCIES target1...targetN]

# MANDATORY   : not actually a plugin, always builtin
# DEFAULT     : builtin as static by default
# MODULE_ONLY : build only as shared library

MACRO(MYSQL_ADD_PLUGIN)
  MYSQL_PARSE_ARGUMENTS(ARG
    "LINK_LIBRARIES;DEPENDENCIES;MODULE_OUTPUT_NAME;STATIC_OUTPUT_NAME"
    "STORAGE_ENGINE;STATIC_ONLY;MODULE_ONLY;CLIENT_ONLY;MANDATORY;DEFAULT;TEST_ONLY;SKIP_INSTALL"
    ${ARGN}
  )
  
  LIST(GET ARG_DEFAULT_ARGS 0 plugin) 
  SET(SOURCES ${ARG_DEFAULT_ARGS})
  LIST(REMOVE_AT SOURCES 0)
  STRING(TOUPPER ${plugin} plugin)
  STRING(TOLOWER ${plugin} target)
  
  # Figure out whether to build plugin
  IF(WITH_PLUGIN_${plugin})
    SET(WITH_${plugin} 1)
  ENDIF()

  IF(ARG_DEFAULT)
    IF(NOT DEFINED WITH_${plugin} AND
       NOT DEFINED WITHOUT_${plugin} AND
       NOT DEFINED WITH_${plugin}_STORAGE_ENGINE)
      SET(WITH_${plugin} 1)
    ENDIF()
  ENDIF()
  
  IF(WITH_${plugin}_STORAGE_ENGINE 
    OR WITH_{$plugin}
    AND NOT WITHOUT_${plugin}_STORAGE_ENGINE
    AND NOT WITHOUT_${plugin}
    AND NOT ARG_MODULE_ONLY)
     
    SET(WITH_${plugin} 1)
  ELSEIF(WITHOUT_${plugin}_STORAGE_ENGINE)
    SET(WITHOUT_${plugin} 1)
    SET(WITH_${plugin}_STORAGE_ENGINE 0)
    SET(WITH_${plugin} 0)
  ENDIF()
  
  IF(DEFINED WITH_${plugin} AND DEFINED WITHOUT_${plugin})
    IF(WITH_${plugin} EQUAL WITHOUT_${plugin})
      MESSAGE(FATAL_ERROR "WITH_${plugin} == WITHOUT_${plugin}")
    ENDIF()
  ENDIF()

  IF(DEFINED WITH_${plugin})
    IF(WITH_${plugin})
      SET(WITHOUT_${plugin} 0)
    ELSE()
      SET(WITHOUT_${plugin} 1)
    ENDIF()
  ENDIF()

  IF(DEFINED WITHOUT_${plugin})
    IF(WITHOUT_${plugin})
      SET(WITH_${plugin} 0)
    ELSE()
      SET(WITH_${plugin} 1)
    ENDIF()
  ENDIF()

  IF(ARG_MANDATORY)
    SET(WITH_${plugin} 1)
    SET(WITHOUT_${plugin} 0)
  ENDIF()

  IF(ARG_STORAGE_ENGINE)
    SET(with_var "WITH_${plugin}_STORAGE_ENGINE" )
  ELSE()
    SET(with_var "WITH_${plugin}")
  ENDIF()
  
  IF(NOT ARG_DEPENDENCIES)
    SET(ARG_DEPENDENCIES)
  ENDIF()
  SET(BUILD_PLUGIN 1)
  # Build either static library or module
  IF (WITH_${plugin} AND NOT ARG_MODULE_ONLY)
    ADD_LIBRARY(${target} STATIC ${SOURCES})
    SET_TARGET_PROPERTIES(${target}
      PROPERTIES COMPILE_DEFINITIONS "MYSQL_SERVER")

    ADD_DEPENDENCIES(${target} GenError ${ARG_DEPENDENCIES})
    
    IF(ARG_STATIC_OUTPUT_NAME)
      SET_TARGET_PROPERTIES(${target} PROPERTIES 
      OUTPUT_NAME ${ARG_STATIC_OUTPUT_NAME})
    ENDIF()

    # Update mysqld dependencies
    SET (MYSQLD_STATIC_PLUGIN_LIBS ${MYSQLD_STATIC_PLUGIN_LIBS} 
      ${target} ${ARG_LINK_LIBRARIES} CACHE INTERNAL "" FORCE)

    IF(ARG_MANDATORY)
      SET(${with_var} ON CACHE INTERNAL
        "Link ${plugin} statically to the server" FORCE)
    ELSE()	
      SET(${with_var} ON CACHE BOOL
        "Link ${plugin} statically to the server" FORCE)
    ENDIF()

    SET(THIS_PLUGIN_REFERENCE " builtin_${target}_plugin,")
    SET(PLUGINS_IN_THIS_SCOPE
      "${PLUGINS_IN_THIS_SCOPE}${THIS_PLUGIN_REFERENCE}")

    IF(ARG_MANDATORY)
      SET (mysql_mandatory_plugins  
        "${mysql_mandatory_plugins} ${PLUGINS_IN_THIS_SCOPE}" 
        PARENT_SCOPE)
    ELSE()
      SET (mysql_optional_plugins  
        "${mysql_optional_plugins} ${PLUGINS_IN_THIS_SCOPE}"
        PARENT_SCOPE)
    ENDIF()

  ELSEIF(NOT WITHOUT_${plugin} AND NOT ARG_STATIC_ONLY)
    IF(NOT ARG_MODULE_OUTPUT_NAME)
      IF(ARG_STORAGE_ENGINE)
        SET(ARG_MODULE_OUTPUT_NAME "ha_${target}")
      ELSE()
        SET(ARG_MODULE_OUTPUT_NAME "${target}")
      ENDIF()
    ENDIF()

    ADD_VERSION_INFO(${target} MODULE SOURCES)
    ADD_LIBRARY(${target} MODULE ${SOURCES}) 
    SET_TARGET_PROPERTIES (${target} PROPERTIES PREFIX ""
      COMPILE_DEFINITIONS "MYSQL_DYNAMIC_PLUGIN")
    IF(WIN32_CLANG AND WITH_ASAN)
      TARGET_LINK_LIBRARIES(${target}
        "${ASAN_LIB_DIR}/clang_rt.asan_dll_thunk-x86_64.lib")
    ENDIF()
    IF(NOT ARG_CLIENT_ONLY)
      TARGET_LINK_LIBRARIES (${target} mysqlservices)
    ENDIF()

    # Plugin uses symbols defined in mysqld executable.
    # Some operating systems like Windows and OSX and are pretty strict about 
    # unresolved symbols. Others are less strict and allow unresolved symbols
    # in shared libraries. On Linux for example, CMake does not even add 
    # executable to the linker command line (it would result into link error). 
    # Thus we skip TARGET_LINK_LIBRARIES on Linux, as it would only generate
    # an additional dependency.
    # Use MYSQL_PLUGIN_IMPORT for static data symbols to be exported.
    IF(NOT ARG_CLIENT_ONLY)
      IF(WIN32 OR APPLE)
        TARGET_LINK_LIBRARIES (${target} mysqld ${ARG_LINK_LIBRARIES})
      ENDIF()
    ENDIF()
    ADD_DEPENDENCIES(${target} GenError ${ARG_DEPENDENCIES})

    IF(NOT ARG_MODULE_ONLY)
      # set cached variable, e.g with checkbox in GUI
      SET(${with_var} OFF CACHE BOOL "Link ${plugin} statically to the server" 
        FORCE)
    ENDIF()
    SET_TARGET_PROPERTIES(${target} PROPERTIES 
      OUTPUT_NAME "${ARG_MODULE_OUTPUT_NAME}")  

    # Store all plugins in the same directory, for easier testing.
    SET_TARGET_PROPERTIES(${target} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugin_output_directory
      )

    IF(APPLE AND HAVE_CRYPTO_DYLIB AND HAVE_OPENSSL_DYLIB)
      ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD
        COMMAND install_name_tool -change
                "${CRYPTO_VERSION}" "@loader_path/${CRYPTO_VERSION}"
                 $<TARGET_FILE_NAME:${target}>
        COMMAND install_name_tool -change
                "${OPENSSL_VERSION}" "@loader_path/${OPENSSL_VERSION}"
                 $<TARGET_FILE_NAME:${target}>
        WORKING_DIRECTORY
        ${CMAKE_BINARY_DIR}/plugin_output_directory/${CMAKE_CFG_INTDIR}
        )
    ENDIF()

    # Install dynamic library
    IF(NOT ARG_SKIP_INSTALL)
      SET(INSTALL_COMPONENT Server)
      IF(ARG_TEST_ONLY)
        SET(INSTALL_COMPONENT Test)
      ENDIF()
      ADD_INSTALL_RPATH_FOR_OPENSSL(${target})
      MYSQL_INSTALL_TARGETS(${target}
        DESTINATION ${INSTALL_PLUGINDIR}
        COMPONENT ${INSTALL_COMPONENT})
      INSTALL_DEBUG_TARGET(${target}
        DESTINATION ${INSTALL_PLUGINDIR}/debug
        COMPONENT ${INSTALL_COMPONENT})
    ENDIF()
  ELSE()
    IF(WITHOUT_${plugin})
      # Update cache variable
      STRING(REPLACE "WITH_" "WITHOUT_" without_var ${with_var})
      SET(${without_var} ON CACHE BOOL "Don't build ${plugin}" 
       FORCE)
    ENDIF()
    SET(BUILD_PLUGIN 0)
  ENDIF()

  IF(BUILD_PLUGIN AND ARG_LINK_LIBRARIES)
    TARGET_LINK_LIBRARIES (${target} ${ARG_LINK_LIBRARIES})
  ENDIF()

ENDMACRO()


# Add all CMake projects under storage  and plugin 
# subdirectories, configure sql_builtin.cc
MACRO(CONFIGURE_PLUGINS)
  FILE(GLOB dirs_storage ${CMAKE_SOURCE_DIR}/storage/*)
  FILE(GLOB dirs_plugin ${CMAKE_SOURCE_DIR}/plugin/*)
  
  FOREACH(dir ${dirs_storage} ${dirs_plugin})
    IF (EXISTS ${dir}/CMakeLists.txt)
      ADD_SUBDIRECTORY(${dir})
    ENDIF()
  ENDFOREACH()
ENDMACRO()
