CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

# includes
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

# package information
SET(PACKAGE_NAME      "hhvm")
SET(PACKAGE_VERSION   "3.9.0-dev")
SET(PACKAGE_STRING    "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(PACKAGE_TARNAME   "${PACKAGE_NAME}-${PACKAGE_VERSION}")
SET(PACKAGE_BUGREPORT "https://github.com/facebook/hhvm/issues")

PROJECT(${PACKAGE_NAME} C CXX ASM)

MARK_AS_ADVANCED(CLEAR CMAKE_INSTALL_PREFIX)
IF(APPLE)
  # CMake really likes finding libraries inside OS X frameworks. This can
  # create super unexpected results, such as the LDAP framework, where the
  # ldap.h header there just consists of "#include <ldap.h>" -- obviously
  # assuming /usr/include appears on the include path before that framework
  # (which wasn't really supposed to be on the include path at all). This
  # leads to a hilarious recursive include and general fireworks. Instead,
  # tell CMake to search frameworks *last*, if it doesn't find something in
  # /usr (or MacPorts/Homebrew).
  SET(CMAKE_FIND_FRAMEWORK "LAST")
  MARK_AS_ADVANCED(CMAKE_OSX_ARCHITECTURES
    CMAKE_OSX_DEPLOYMENT_TARGET
    CMAKE_OSX_SYSROOT)
ENDIF()

# Check architecture OS
IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  MESSAGE(FATAL_ERROR "HHVM requires a 64bit OS")
ENDIF()

# 3rd party library
IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third-party/CMakeLists.txt")
  MESSAGE(FATAL_ERROR "third-party/CMakeLists.txt missing. "
                      "Try updating your submodule with:
rm -r third-party
git submodule update --init --recursive
")
ENDIF()

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/CMake/HPHPFunctions.cmake")
INCLUDE(CheckFunctionExists)

SET(HPHP_HOME ${CMAKE_CURRENT_SOURCE_DIR})
SET(TP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

ADD_SUBDIRECTORY(third-party)
ADD_SUBDIRECTORY(hphp)

# use GNU install dirs (e.g. lib64 instead of lib)
INCLUDE(GNUInstallDirs)

# modules / depends
FILE(GLOB HHVM_CMAKE_FILES "CMake/*.cmake")
INSTALL(
  FILES ${HHVM_CMAKE_FILES}
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/hhvm/CMake"
  COMPONENT dev)
