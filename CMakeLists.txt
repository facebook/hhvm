CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
# Make `get_target_property()` on a target that does not exist a fatal error
# https://cmake.org/cmake/help/v3.0/policy/CMP0045.html
cmake_policy(SET CMP0045 NEW)
# ditto for add_dependencies(): https://cmake.org/cmake/help/v3.0/policy/CMP0046.html
cmake_policy(SET CMP0046 NEW)

# This needs to be done before any languages are enabled or
# projects are created.
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/CMake/VisualStudioToolset.cmake")

# includes
SET(
  CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  "${CMAKE_CURRENT_SOURCE_DIR}/build/fbcode_builder/CMake"
  ${CMAKE_MODULE_PATH}
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROJECT(hhvm C CXX ASM)

include(HHVMProject)

if (MSVC)
  enable_language(ASM_MASM)
endif()

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
  SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
ENDIF()

# Check architecture OS
IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  MESSAGE(FATAL_ERROR "HHVM requires a 64bit OS")
ENDIF()

# Enable ccache if present and not already enabled system wide.
option(SKIP_CCACHE "Skip detecting/enabling ccache - no effect if ccache enabled system wide" FALSE)
if(NOT SKIP_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    if (NOT ("${CMAKE_CXX_COMPILER} ${CMAKE_C_COMPILER}" MATCHES ".*ccache.*"))
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
      message(STATUS "Found ccache: ${CCACHE_FOUND} - enabling ccache as compiler wrapper")
    else()
      message(STATUS "Found ccache - ccache already in use as C and/or CXX compiler wrapper")
   endif()
  endif(CCACHE_FOUND)
endif(NOT SKIP_CCACHE)

INCLUDE(HPHPFunctions)
INCLUDE(CheckFunctionExists)

SET(HPHP_HOME ${CMAKE_CURRENT_SOURCE_DIR})
SET(TP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
SET(TP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/third-party")

include(MSVCDefaults)
include(SetOSVars)
include(Options)
include(HPHPCompiler)
include(HPHPFindLibs)

ADD_SUBDIRECTORY(third-party EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(hphp)

# use GNU install dirs (e.g. lib64 instead of lib)
INCLUDE(GNUInstallDirs)

# modules / depends
FILE(GLOB HHVM_CMAKE_FILES "CMake/*.cmake")
INSTALL(
  FILES ${HHVM_CMAKE_FILES}
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/hhvm/CMake"
  COMPONENT dev)
