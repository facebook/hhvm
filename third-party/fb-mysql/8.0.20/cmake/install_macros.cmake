# Copyright (c) 2009, 2020, Oracle and/or its affiliates. All rights reserved.
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

# For windows: install .pdf file for each target.
MACRO (INSTALL_DEBUG_SYMBOLS targets)
  IF(MSVC)
  FOREACH(target ${targets})
    GET_TARGET_PROPERTY(type ${target} TYPE)
    IF(NOT INSTALL_LOCATION)
      IF(type MATCHES "STATIC_LIBRARY"
          OR type MATCHES "MODULE_LIBRARY"
          OR type MATCHES "SHARED_LIBRARY")
        SET(INSTALL_LOCATION "lib")
      ELSEIF(type MATCHES "EXECUTABLE")
        SET(INSTALL_LOCATION "bin")
      ELSE()
        MESSAGE(FATAL_ERROR
          "cannot determine type of ${target}. Don't now where to install")
     ENDIF()
    ENDIF()

    IF(target STREQUAL "mysqld" OR target STREQUAL "mysqlbackup")
      SET(comp Server)
    ELSE()
      SET(comp Debuginfo)
    ENDIF()

    # No .pdb file for static libraries.
    IF(NOT type MATCHES "STATIC_LIBRARY")
      INSTALL(FILES $<TARGET_PDB_FILE:${target}>
        DESTINATION ${INSTALL_LOCATION} COMPONENT ${comp})
    ENDIF()
  ENDFOREACH()
  ENDIF()
ENDMACRO()


FUNCTION(INSTALL_SCRIPT)
 MYSQL_PARSE_ARGUMENTS(ARG
  "DESTINATION;COMPONENT"
  ""
  ${ARGN}
  )
  
  SET(script ${ARG_DEFAULT_ARGS})
  IF(NOT ARG_DESTINATION)
    SET(ARG_DESTINATION ${INSTALL_BINDIR})
  ENDIF()
  IF(ARG_COMPONENT)
    SET(COMP COMPONENT ${ARG_COMPONENT})
  ELSE()
    SET(COMP)
  ENDIF()

  SET(DO_INSTALL_SCRIPT 1)
  IF(ARG_COMPONENT AND ${ARG_COMPONENT} MATCHES "Server")
    IF(WITHOUT_SERVER)
      UNSET(DO_INSTALL_SCRIPT)
    ENDIF()
  ENDIF()
  IF(DO_INSTALL_SCRIPT)
    INSTALL(FILES
      ${script}
      DESTINATION ${ARG_DESTINATION}
      PERMISSIONS OWNER_READ OWNER_WRITE
      OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE  ${COMP}
      )
  ELSE()
#   MESSAGE(STATUS "skip install of ${script}")
  ENDIF()
ENDFUNCTION()


# Installs targets, also installs pdbs on Windows.
#
#

FUNCTION(MYSQL_INSTALL_TARGETS)
  MYSQL_PARSE_ARGUMENTS(ARG
    "DESTINATION;COMPONENT"
  ""
  ${ARGN}
  )
  SET(TARGETS ${ARG_DEFAULT_ARGS})
  IF(NOT TARGETS)
    MESSAGE(FATAL_ERROR "Need target list for MYSQL_INSTALL_TARGETS")
  ENDIF()
  IF(NOT ARG_DESTINATION)
     MESSAGE(FATAL_ERROR "Need DESTINATION parameter for MYSQL_INSTALL_TARGETS")
  ENDIF()

  IF(ARG_COMPONENT)
    SET(COMP COMPONENT ${ARG_COMPONENT})
  ENDIF()
  INSTALL(TARGETS ${TARGETS} DESTINATION ${ARG_DESTINATION} ${COMP})
  SET(INSTALL_LOCATION ${ARG_DESTINATION} )
  INSTALL_DEBUG_SYMBOLS("${TARGETS}")
  SET(INSTALL_LOCATION)
ENDFUNCTION()

# Optionally install mysqld/client from debug build run.
# outside of the current build dir
# (unless multi-config generator is used like Visual Studio or Xcode). 
# For single-config generators like Makefile generators we default Debug
# build directory to ${buildroot}/../debug.
GET_FILENAME_COMPONENT(BINARY_PARENTDIR ${CMAKE_BINARY_DIR} PATH)
SET(DEBUGBUILDDIR "${BINARY_PARENTDIR}/debug" CACHE INTERNAL
  "Directory of debug build")


FUNCTION(INSTALL_DEBUG_TARGET target)
  MYSQL_PARSE_ARGUMENTS(ARG
    "DESTINATION;RENAME;COMPONENT"
    ""
    ${ARGN}
    )

  # Relevant only for RelWithDebInfo builds
  IF(BUILD_IS_SINGLE_CONFIG AND CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
    RETURN()
  ENDIF()

  GET_TARGET_PROPERTY(target_type ${target} TYPE)
  GET_TARGET_PROPERTY(target_name ${target} DEBUG_OUTPUT_NAME)
  IF(NOT target_name)
    GET_TARGET_PROPERTY(target_name ${target} OUTPUT_NAME)
    IF(NOT target_name)
      SET(target_name "${target}")
    ENDIF()
  ENDIF()

  # On windows we install client libraries
  IF(target_type STREQUAL "STATIC_LIBRARY")
    SET(debug_target_location
      "${CMAKE_BINARY_DIR}/archive_output_directory/Debug/${target_name}.lib")
  # mysqld or mysqld-debug
  ELSEIF(target_type STREQUAL "EXECUTABLE")
    SET(EXE_SUFFIX "${CMAKE_EXECUTABLE_SUFFIX}")
    IF(BUILD_IS_SINGLE_CONFIG)
      SET(debug_target_location
        "${DEBUGBUILDDIR}/runtime_output_directory/${target_name}${EXE_SUFFIX}")
    ELSE()
      SET(debug_target_location
        "${CMAKE_BINARY_DIR}/runtime_output_directory/Debug/${target_name}${EXE_SUFFIX}")
    ENDIF()
  # Plugins and components
  ELSEIF(target_type STREQUAL "MODULE_LIBRARY")
    SET(DLL_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    IF(APPLE)
      SET(DLL_SUFFIX ".so") # we do not use .dylib
    ENDIF()

    SET(MODULE_DIRECTORY "plugin_output_directory")
    IF(BUILD_IS_SINGLE_CONFIG)
      SET(debug_target_location
        "${DEBUGBUILDDIR}/${MODULE_DIRECTORY}/${target_name}${DLL_SUFFIX}")
    ELSE()
      SET(debug_target_location
        "${CMAKE_BINARY_DIR}/${MODULE_DIRECTORY}/Debug/${target_name}${DLL_SUFFIX}")
    ENDIF()
  # libprotobuf-debug libprotobuf-lite-debug
  ELSEIF(target_type STREQUAL "SHARED_LIBRARY")
    GET_TARGET_PROPERTY(debug_postfix ${target} DEBUG_POSTFIX)
    GET_TARGET_PROPERTY(library_version ${target} VERSION)

    IF(BUILD_IS_SINGLE_CONFIG)
      SET(debug_target_location "${DEBUGBUILDDIR}")
    ELSE()
      SET(debug_target_location "${CMAKE_BINARY_DIR}")
    ENDIF()
    STRING_APPEND(debug_target_location "/library_output_directory")
    IF(NOT BUILD_IS_SINGLE_CONFIG)
      STRING_APPEND(debug_target_location "/Debug")
    ENDIF()
    STRING_APPEND(debug_target_location "/${CMAKE_SHARED_LIBRARY_PREFIX}")
    STRING_APPEND(debug_target_location "${target_name}${debug_postfix}")
    STRING_APPEND(debug_target_location "${CMAKE_SHARED_LIBRARY_SUFFIX}")
    IF(NOT WIN32)
      STRING_APPEND(debug_target_location ".${library_version}")
    ENDIF()

    MESSAGE(STATUS "INSTALL_DEBUG_TARGET ${debug_target_location}")
  ENDIF()

  # This is only used for mysqld / mysqld-debug
  IF(ARG_RENAME)
    SET(RENAME_PARAM RENAME ${ARG_RENAME})
  ELSE()
    SET(RENAME_PARAM)
  ENDIF()

  IF(NOT ARG_DESTINATION)
    MESSAGE(FATAL_ERROR "Need DESTINATION parameter for INSTALL_DEBUG_TARGET")
  ENDIF()

  IF(NOT ARG_COMPONENT)
    SET(ARG_COMPONENT DebugBinaries)
  ENDIF()
  
  # Define permissions
  # For executable files
  SET(PERMISSIONS_EXECUTABLE
      PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE)

  # Permissions for shared library (honors CMAKE_INSTALL_NO_EXE which is 
  # typically set on Debian)
  IF(CMAKE_INSTALL_SO_NO_EXE)
    SET(PERMISSIONS_SHARED_LIBRARY
      PERMISSIONS
      OWNER_READ OWNER_WRITE 
      GROUP_READ
      WORLD_READ)
  ELSE()
    SET(PERMISSIONS_SHARED_LIBRARY ${PERMISSIONS_EXECUTABLE})
  ENDIF()

  # Shared modules get the same permissions as shared libraries
  SET(PERMISSIONS_MODULE_LIBRARY ${PERMISSIONS_SHARED_LIBRARY})

  #  Define permissions for static library
  SET(PERMISSIONS_STATIC_LIBRARY
      PERMISSIONS
      OWNER_READ OWNER_WRITE 
      GROUP_READ
      WORLD_READ)

  INSTALL(FILES ${debug_target_location}
    DESTINATION ${ARG_DESTINATION}
    ${RENAME_PARAM}
    ${PERMISSIONS_${target_type}}
    CONFIGURATIONS Release RelWithDebInfo
    COMPONENT ${ARG_COMPONENT}
    OPTIONAL)

  # For windows, install .pdb files for .exe and .dll files.
  IF(MSVC AND NOT target_type STREQUAL "STATIC_LIBRARY")
    GET_FILENAME_COMPONENT(ext ${debug_target_location} EXT)
    STRING(REPLACE "${ext}" ".pdb"
      debug_pdb_target_location "${debug_target_location}" )
    IF (RENAME_PARAM)
      STRING(REPLACE "${ext}" ".pdb"  pdb_rename "${ARG_RENAME}")
      SET(PDB_RENAME_PARAM RENAME "${pdb_rename}")
    ENDIF()

    INSTALL(FILES ${debug_pdb_target_location}
      DESTINATION ${ARG_DESTINATION}
      ${PDB_RENAME_PARAM}
      CONFIGURATIONS Release RelWithDebInfo
      COMPONENT ${ARG_COMPONENT}
      OPTIONAL)
  ENDIF()
ENDFUNCTION()


FUNCTION(INSTALL_PRIVATE_LIBRARY TARGET)
  IF(APPLE)
    INSTALL(TARGETS ${TARGET}
      DESTINATION "${INSTALL_LIBDIR}" COMPONENT SharedLibraries
      )
  ELSEIF(WIN32)
    INSTALL(TARGETS ${TARGET}
      DESTINATION "${INSTALL_BINDIR}" COMPONENT SharedLibraries
      )
  ELSEIF(UNIX)
    INSTALL(TARGETS ${TARGET}
      LIBRARY
      DESTINATION "${INSTALL_PRIV_LIBDIR}"
      COMPONENT SharedLibraries
      NAMELINK_SKIP
      )
  ENDIF()
ENDFUNCTION()


# On Unix: add to RPATH of an executable when it is installed.
# Use 'chrpath' to inspect results.
# For Solaris, use 'elfdump -d'
MACRO(ADD_INSTALL_RPATH TARGET VALUE)
  GET_TARGET_PROPERTY(CURRENT_RPATH_${TARGET} ${TARGET} INSTALL_RPATH)
  IF(NOT CURRENT_RPATH_${TARGET})
    SET(CURRENT_RPATH_${TARGET})
  ENDIF()
  LIST(APPEND CURRENT_RPATH_${TARGET} ${VALUE})
  SET_TARGET_PROPERTIES(${TARGET}
    PROPERTIES INSTALL_RPATH "${CURRENT_RPATH_${TARGET}}")
ENDMACRO()


# For standalone Linux build or community RPM build, we support
#   -DWITH_SSL=</path/to/custom/openssl>
# SSL libraries are installed in lib/private
# We need to extend INSTALL_RPATH with location of SSL libraries:
# executable  in bin        rpath $ORIGIN/../lib/private
# plugins     in lib/plugin rpath $ORIGIN/../private
# shared libs in lib        rpath $ORIGIN/private
MACRO(ADD_INSTALL_RPATH_FOR_OPENSSL TARGET)
  IF(LINUX_INSTALL_RPATH_ORIGIN)
    GET_TARGET_PROPERTY(TARGET_TYPE_${TARGET} ${TARGET} TYPE)
    IF(TARGET_TYPE_${TARGET} STREQUAL "EXECUTABLE")
      ADD_INSTALL_RPATH(${TARGET} "\$ORIGIN/../${INSTALL_PRIV_LIBDIR}")
    ELSEIF(TARGET_TYPE_${TARGET} STREQUAL "MODULE_LIBRARY")
      ADD_INSTALL_RPATH(${TARGET} "\$ORIGIN/../private")
    ELSEIF(TARGET_TYPE_${TARGET} STREQUAL "SHARED_LIBRARY")
      ADD_INSTALL_RPATH(${TARGET} "\$ORIGIN/private")
    ELSE()
      MESSAGE(FATAL_ERROR "unknown type ${TARGET_TYPE_${TARGET}} for ${TARGET}")
    ENDIF()
  ENDIF()
ENDMACRO()

# For APPLE: set INSTALL_RPATH, and adjust path dependecy for libprotobuf.
# Use 'otool -L' to inspect results.
# For UNIX: extend INSTALL_RPATH with libprotobuf location.
MACRO(ADD_INSTALL_RPATH_FOR_PROTOBUF TARGET)
  IF(APPLE)
    SET_PROPERTY(TARGET ${TARGET} PROPERTY INSTALL_RPATH "@loader_path")
    # install_name_tool [-change old new] input
    # Changing it to @loader_path/../lib/ works in build sandbox
    # because we have a symlink to ./library_output_directory.
    ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
      COMMAND install_name_tool -change
      "@rpath/$<TARGET_FILE_NAME:libprotobuf-lite>"
      "@loader_path/../lib/$<TARGET_FILE_NAME:libprotobuf-lite>"
      "$<TARGET_FILE:${TARGET}>"
      COMMAND install_name_tool -change
      "@rpath/$<TARGET_FILE_NAME:libprotobuf>"
      "@loader_path/../lib/$<TARGET_FILE_NAME:libprotobuf>"
      "$<TARGET_FILE:${TARGET}>"
      )
  ELSEIF(UNIX)
    ADD_INSTALL_RPATH(${TARGET} "\$ORIGIN/../${INSTALL_PRIV_LIBDIR}")
  ENDIF()
ENDMACRO()

# For APPLE: adjust path dependecy for SSL shared libraries.
FUNCTION(SET_PATH_TO_SSL target target_out_dir)
  IF(APPLE AND HAVE_CRYPTO_DYLIB AND HAVE_OPENSSL_DYLIB)
    IF(BUILD_IS_SINGLE_CONFIG)
      ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD
        COMMAND install_name_tool -change
              "${CRYPTO_VERSION}" "@loader_path/../lib/${CRYPTO_VERSION}"
              $<TARGET_FILE_NAME:${target}>
        COMMAND install_name_tool -change
              "${OPENSSL_VERSION}" "@loader_path/../lib/${OPENSSL_VERSION}"
              $<TARGET_FILE_NAME:${target}>
        WORKING_DIRECTORY ${target_out_dir}
      )
    ELSE()
      ADD_CUSTOM_COMMAND(TARGET ${target} POST_BUILD
        COMMAND install_name_tool -change
            "${CRYPTO_VERSION}"
            "@loader_path/../../lib/${CMAKE_CFG_INTDIR}/${CRYPTO_VERSION}"
        $<TARGET_FILE_NAME:${target}>
        COMMAND install_name_tool -change
            "${OPENSSL_VERSION}"
            "@loader_path/../../lib/${CMAKE_CFG_INTDIR}/${OPENSSL_VERSION}"
        $<TARGET_FILE_NAME:${target}>
        WORKING_DIRECTORY ${target_out_dir}/${CMAKE_CFG_INTDIR}
      )
    ENDIF()
  ENDIF()
ENDFUNCTION()


# For standalone Linux build and -DWITH_LDAP -DWITH_SASL -DWITH_SSL and
# -DWITH_KERBEROS set to custom path.
#
# Move the custom shared library and symlinks to library_output_directory.
# The subdir argument is typically empty, but set to "sasl2" for SASL plugins,
# in which case we move library_full_filename and its symlinks to
# library_output_directory/sasl2.
#
# We ensure that the copied custom libraries have the execute bit set.
# We also update the RUNPATH of libraries to be '$ORIGIN' to ensure that
# libraries get correct load-time dependencies. This is done using the
# linux tool patchelf(1)
#
# Set ${OUTPUT_LIBRARY_NAME} to the new location.
# Set ${OUTPUT_TARGET_NAME} to the name of a target which will do the copying.
# Add an INSTALL(FILES ....) rule to install library and symlinks into
#   ${INSTALL_PRIV_LIBDIR} or ${INSTALL_PRIV_LIBDIR}/sasl2
FUNCTION(COPY_CUSTOM_SHARED_LIBRARY library_full_filename subdir
    OUTPUT_LIBRARY_NAME
    OUTPUT_TARGET_NAME
    )
  IF(NOT LINUX_WITH_CUSTOM_LIBRARIES)
    RETURN()
  ENDIF()
  GET_FILENAME_COMPONENT(LIBRARY_EXT "${library_full_filename}" EXT)
  IF(NOT LIBRARY_EXT STREQUAL ".so")
    RETURN()
  ENDIF()
  EXECUTE_PROCESS(
    COMMAND readlink "${library_full_filename}" OUTPUT_VARIABLE library_version
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  GET_FILENAME_COMPONENT(library_directory "${library_full_filename}" DIRECTORY)
  GET_FILENAME_COMPONENT(library_name "${library_full_filename}" NAME)
  GET_FILENAME_COMPONENT(library_name_we "${library_full_filename}" NAME_WE)

  FIND_SONAME(${library_full_filename} library_soname)
  FIND_OBJECT_DEPENDENCIES(${library_full_filename} library_dependencies)

  MESSAGE(STATUS "CUSTOM library ${library_full_filename}")
# MESSAGE(STATUS "CUSTOM version ${library_version}")
# MESSAGE(STATUS "CUSTOM directory ${library_directory}")
# MESSAGE(STATUS "CUSTOM name ${library_name}")
# MESSAGE(STATUS "CUSTOM name_we ${library_name_we}")
# MESSAGE(STATUS "CUSTOM soname ${library_soname}")

  SET(COPIED_LIBRARY_NAME
    "${CMAKE_BINARY_DIR}/library_output_directory/${subdir}/${library_name}")
  SET(COPY_TARGET_NAME "copy_${library_name_we}_dll")

  # Keep track of libraries and dependencies.
  SET(SONAME_${library_name_we} "${library_soname}"
    CACHE INTERNAL "SONAME for ${library_name_we}" FORCE)
  SET(NEEDED_${library_name_we} "${library_dependencies}"
    CACHE INTERNAL "" FORCE)
  SET(KNOWN_CUSTOM_LIBRARIES
    ${KNOWN_CUSTOM_LIBRARIES} ${library_name_we} CACHE INTERNAL "" FORCE)

  # Do copying and patching in a sub-process, so that we can skip it if
  # already done. The BYPRODUCTS arguments is needed by Ninja, and is
  # ignored on non-Ninja generators except to mark byproducts GENERATED.
  ADD_CUSTOM_TARGET(${COPY_TARGET_NAME} ALL
    COMMAND ${CMAKE_COMMAND}
    -Dlibrary_directory="${library_directory}"
    -Dlibrary_name="${library_name}"
    -Dlibrary_soname="${library_soname}"
    -Dlibrary_version="${library_version}"
    -Dsubdir="${subdir}"
    -DPATCHELF_EXECUTABLE="${PATCHELF_EXECUTABLE}"
    -P ${CMAKE_SOURCE_DIR}/cmake/copy_custom_library.cmake

    BYPRODUCTS
    "${CMAKE_BINARY_DIR}/library_output_directory/${subdir}/${library_name}"

    WORKING_DIRECTORY
    "${CMAKE_BINARY_DIR}/library_output_directory/${subdir}"
    )

  # Link with the copied library, rather than the original one.
  SET(${OUTPUT_LIBRARY_NAME} "${COPIED_LIBRARY_NAME}" PARENT_SCOPE)
  SET(${OUTPUT_TARGET_NAME} "${COPY_TARGET_NAME}" PARENT_SCOPE)

  ADD_DEPENDENCIES(copy_linux_custom_dlls ${COPY_TARGET_NAME})

  MESSAGE(STATUS "INSTALL ${library_name} to ${INSTALL_PRIV_LIBDIR}/${subdir}")

  # Cannot use INSTALL_PRIVATE_LIBRARY because these are not targets.
  INSTALL(FILES
    ${CMAKE_BINARY_DIR}/library_output_directory/${subdir}/${library_name}
    ${CMAKE_BINARY_DIR}/library_output_directory/${subdir}/${library_soname}
    ${CMAKE_BINARY_DIR}/library_output_directory/${subdir}/${library_version}
    DESTINATION "${INSTALL_PRIV_LIBDIR}/${subdir}" COMPONENT SharedLibraries
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
    )

ENDFUNCTION()
