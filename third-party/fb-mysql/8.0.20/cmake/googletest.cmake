# Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.
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

# We want release-1.8.1.zip in order to build these unit tests.
# If you have already downloaded it,
# invoke cmake with -DWITH_GMOCK=/path/to/release-1.8.1.zip
#                or -DWITH_GMOCK=/path/to
#
# Alternatively, set an environment variable
# export WITH_GMOCK=/path/to/release-1.8.1.zip
#
# You can also do cmake -DENABLE_DOWNLOADS=1
# and we will download it from https://github.com/google/googletest/archive/
#
# Either way: we will unpack the zip, compile gmock-all.cc and gtest-all.cc
# and link them into the executables.


# Default location for where to download and build gmock/gtest.
IF(NOT DOWNLOAD_ROOT)
  SET(DOWNLOAD_ROOT ${CMAKE_SOURCE_DIR}/source_downloads)
ENDIF()

# We want googletest version 1.8, which also contains googlemock.
SET(GMOCK_PACKAGE_NAME "release-1.8.1")

IF (DEFINED ENV{WITH_GMOCK} AND NOT DEFINED WITH_GMOCK)
  FILE(TO_CMAKE_PATH "$ENV{WITH_GMOCK}" WITH_GMOCK)
ENDIF()

IF(LOCAL_GMOCK_ZIP
    AND NOT ${LOCAL_GMOCK_ZIP} MATCHES ".*${GMOCK_PACKAGE_NAME}\\.zip")
  SET(LOCAL_GMOCK_ZIP 0)
ENDIF()

IF (WITH_GMOCK)
  FILE(TO_CMAKE_PATH "${WITH_GMOCK}" WITH_GMOCK)
  ## Did we get a full path name, including file name?
  IF (${WITH_GMOCK} MATCHES ".*\\.zip")
    GET_FILENAME_COMPONENT(GMOCK_DIR ${WITH_GMOCK} PATH)
    GET_FILENAME_COMPONENT(GMOCK_ZIP ${WITH_GMOCK} NAME)
    FIND_FILE(LOCAL_GMOCK_ZIP
      NAMES ${GMOCK_ZIP}
      PATHS ${GMOCK_DIR}
      NO_DEFAULT_PATH
      )
  ELSE()
    ## Did we get a path name to the directory of the .zip file?
    ## Check for both release-x.y.z.zip and googletest-release-x.y.z.zip
    FIND_FILE(LOCAL_GMOCK_ZIP
      NAMES "${GMOCK_PACKAGE_NAME}.zip"
            "googletest-${GMOCK_PACKAGE_NAME}.zip"
      PATHS ${WITH_GMOCK}
      NO_DEFAULT_PATH
      )
    ## If WITH_GMOCK is a directory, use it for download.
    SET(DOWNLOAD_ROOT ${WITH_GMOCK})
  ENDIF()
  MESSAGE(STATUS "Local gmock zip ${LOCAL_GMOCK_ZIP}")
ENDIF()

IF(NOT EXISTS DOWNLOAD_ROOT)
  MAKE_DIRECTORY(${DOWNLOAD_ROOT})
ENDIF()

IF(NOT GMOCK_SOURCE_DIR)
  SET(GMOCK_SOURCE_DIR
    ${DOWNLOAD_ROOT}/googletest-${GMOCK_PACKAGE_NAME}/googlemock)
ENDIF()
IF(NOT GTEST_SOURCE_DIR)
  SET(GTEST_SOURCE_DIR
    ${DOWNLOAD_ROOT}/googletest-${GMOCK_PACKAGE_NAME}/googletest)
ENDIF()

# We may have downloaded gmock/gtest already, building in a different directory.
IF(EXISTS ${GMOCK_SOURCE_DIR} OR EXISTS ${LOCAL_GMOCK_ZIP})
  MESSAGE(STATUS "GMOCK_SOURCE_DIR:${GMOCK_SOURCE_DIR}")
  SET(GMOCK_DOWNLOADED 1 CACHE INTERNAL "")
  SET(GMOCK_FOUND 1 CACHE INTERNAL "")
  # If source dir does not exist, reset dependent variables
  # (might be set from before).
ELSE()
  SET(LOCAL_GMOCK_ZIP 0 CACHE INTERNAL "")
  SET(GMOCK_DOWNLOADED 0 CACHE INTERNAL "")
  SET(GMOCK_FOUND 0 CACHE INTERNAL "")
  UNSET(GMOCK_INCLUDE_DIRS)
  UNSET(GMOCK_INCLUDE_DIRS CACHE)
ENDIF()


IF(LOCAL_GMOCK_ZIP AND NOT EXISTS ${GMOCK_SOURCE_DIR})
  # Unpack tarball
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E tar xfz  "${LOCAL_GMOCK_ZIP}"
    WORKING_DIRECTORY "${DOWNLOAD_ROOT}"
    RESULT_VARIABLE tar_result
    )
  IF (tar_result MATCHES 0)
    SET(GMOCK_FOUND 1 CACHE INTERNAL "")
  ENDIF()
ENDIF()


OPTION(ENABLE_DOWNLOADS
  "Download and build 3rd party source code components, e.g. googletest"
  OFF)

# While experimenting, use local URL rather than google.
SET(GMOCK_TARBALL "googletest-${GMOCK_PACKAGE_NAME}.zip")
SET(GMOCK_DOWNLOAD_URL
  "https://github.com/google/googletest/archive/${GMOCK_PACKAGE_NAME}.zip"
  )

MACRO(HTTP_PROXY_HINT)
  MESSAGE(STATUS
    "If you are inside a firewall, you may need to use an https proxy: "
    "export https_proxy=http://example.com:80")
ENDMACRO()


IF(NOT GMOCK_FOUND)
  IF(NOT ENABLE_DOWNLOADS)
    # Give warning
    MESSAGE(STATUS
      "Googletest was not found. gtest-based unit tests will be disabled. "
      "You can run cmake . -DENABLE_DOWNLOADS=1 to automatically download "
      "and build required components from source.")
    HTTP_PROXY_HINT()
    RETURN()
  ENDIF()

  # Download googletest source
  IF(NOT EXISTS ${GMOCK_SOURCE_DIR})
    IF(NOT EXISTS ${DOWNLOAD_ROOT}/${GMOCK_TARBALL})
      # Download the tarball
      # Use CMake builtin download capabilities
      FILE(DOWNLOAD ${GMOCK_DOWNLOAD_URL}
        ${DOWNLOAD_ROOT}/${GMOCK_TARBALL} TIMEOUT 30 STATUS ERR)
      IF(ERR EQUAL 0)
        SET(DOWNLOAD_SUCCEEDED 1)
      ELSE()
        MESSAGE(STATUS "Download failed, error: ${ERR}")
        # A failed DOWNLOAD leaves an empty file, remove it
        FILE(REMOVE ${DOWNLOAD_ROOT}/${GMOCK_TARBALL})
      ENDIF()

      IF (DOWNLOAD_SUCCEEDED)
        MESSAGE(STATUS
          "Successfully downloaded ${GMOCK_DOWNLOAD_URL} to ${DOWNLOAD_ROOT}")
      ELSE()
        MESSAGE(STATUS
          "To enable googletest, please download ${GMOCK_DOWNLOAD_URL} "
          "to the directory ${DOWNLOAD_ROOT}")
        HTTP_PROXY_HINT()
        RETURN()
      ENDIF()
    ENDIF()

    # Unpack tarball
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E tar xfz  "${DOWNLOAD_ROOT}/${GMOCK_TARBALL}"
      WORKING_DIRECTORY "${DOWNLOAD_ROOT}"
      )
  ENDIF()
  IF(EXISTS ${GMOCK_SOURCE_DIR})
    SET(GMOCK_DOWNLOADED 1 CACHE INTERNAL "")
    SET(GMOCK_FOUND 1 CACHE INTERNAL "")
  ENDIF()
ENDIF()


IF(NOT GMOCK_FOUND)
  RETURN()
ENDIF()

SET(GMOCK_INCLUDE_DIRS
  ${GMOCK_SOURCE_DIR}
  ${GMOCK_SOURCE_DIR}/include
  ${GTEST_SOURCE_DIR}
  ${GTEST_SOURCE_DIR}/include
  CACHE INTERNAL "")

ADD_LIBRARY(gmock STATIC ${GMOCK_SOURCE_DIR}/src/gmock-all.cc)
ADD_LIBRARY(gtest STATIC ${GTEST_SOURCE_DIR}/src/gtest-all.cc)
SET(GTEST_LIBRARIES gmock gtest)

ADD_LIBRARY(gmock_main STATIC ${GMOCK_SOURCE_DIR}/src/gmock_main.cc)
ADD_LIBRARY(gtest_main STATIC ${GTEST_SOURCE_DIR}/src/gtest_main.cc)
IF(MY_COMPILER_IS_GNU_OR_CLANG)
  SET_TARGET_PROPERTIES(gtest_main gmock_main
    PROPERTIES
    COMPILE_FLAGS "-Wno-undef -Wno-conversion")
ENDIF()

IF(MY_COMPILER_IS_SUNPRO)
  ## https://community.oracle.com/thread/4106985
  ## Assertion: (../lnk/symdescr.cc, line 96) while processing ....
  IF(NOT EXISTS ${CMAKE_BINARY_DIR}/hack/src/)
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/hack/src
      )
  ENDIF()
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_BINARY_DIR}/hack/src/gtest.cc
    COMMAND sed -e "/using internal::ParseStringFlag/d"
         < ${GTEST_SOURCE_DIR}/src/gtest.cc
         > ${CMAKE_BINARY_DIR}/hack/src/gtest.cc
    DEPENDS ${GTEST_SOURCE_DIR}/src/gtest.cc
    COMMENT "Filtering ${GTEST_SOURCE_DIR}/src/gtest.cc"
    VERBATIM
    )
  ADD_CUSTOM_TARGET(generate_gtest_hack
    DEPENDS ${CMAKE_BINARY_DIR}/hack/src/gtest.cc)

  ## gtest-port.h contains checks for __GLIBCXX__ and gcc older than 4.4.2
  ## We *do* have __GLIBCXX__ defined, but not __GNUC__
  ## so we need to filter away the undef GTEST_HAS_STD_TUPLE
  IF(NOT EXISTS ${CMAKE_BINARY_DIR}/hack/gtest/internal/)
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E
      make_directory ${CMAKE_BINARY_DIR}/hack/gtest/internal
      )
  ENDIF()
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_BINARY_DIR}/hack/gtest/internal/gtest-port.h
    COMMAND sed -e "/undef GTEST_HAS_STD_TUPLE_/d"
         < ${GTEST_SOURCE_DIR}/include/gtest/internal/gtest-port.h
         > ${CMAKE_BINARY_DIR}/hack/gtest/internal/gtest-port.h
    DEPENDS ${GTEST_SOURCE_DIR}/include/gtest/internal/gtest-port.h
    COMMENT "Filtering gtest-port.h"
    VERBATIM
    )
  ADD_CUSTOM_TARGET(generate_gtest_port_hack
    DEPENDS ${CMAKE_BINARY_DIR}/hack/gtest/internal/gtest-port.h)

  FOREACH(target gtest gmock gtest_main gmock_main)
    ADD_DEPENDENCIES(${target} generate_gtest_hack generate_gtest_port_hack)
    TARGET_INCLUDE_DIRECTORIES(${target} SYSTEM PUBLIC ${CMAKE_BINARY_DIR}/hack)
  ENDFOREACH()

ENDIF()

FOREACH(googletest_library
    gmock
    gtest
    gmock_main
    gtest_main
    )
  TARGET_INCLUDE_DIRECTORIES(${googletest_library} SYSTEM PUBLIC
    ${GMOCK_INCLUDE_DIRS}
    )
ENDFOREACH()

IF(MY_COMPILER_IS_SUNPRO)
  ADD_DEFINITIONS(-DGTEST_LANG_CXX11=1)
  LIST(INSERT GMOCK_INCLUDE_DIRS 0 ${CMAKE_BINARY_DIR}/hack)
ENDIF()
