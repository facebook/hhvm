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

MESSAGE(STATUS "Running cmake version ${CMAKE_VERSION}")

IF(WIN32)
  # CMake 3.8.0 is needed for Visual Studio 2017 and x64 toolset.
  CMAKE_MINIMUM_REQUIRED(VERSION 3.8.0)
  IF(CMAKE_GENERATOR MATCHES "Visual Studio" AND CMAKE_GENERATOR MATCHES "2019")
    IF(CMAKE_VERSION MATCHES "MSVC")
      # It is the bundled version, ignore version check,
      # (although this seems to be buggy too).
    ELSE()
      # Bug in msbuild, install the latest in the 3.15 series as a workaround.
      # https://gitlab.kitware.com/cmake/cmake/issues/19303
      # custom commands are re-built every time
      CMAKE_MINIMUM_REQUIRED(VERSION 3.15.3)
    ENDIF()
  ENDIF()
ELSEIF(APPLE)
  # Version 3.12.4 is needed because the new build system of Xcode is not
  # supported by cmake. 3.12.4 will force using the legacy build system.
  # Version 3.9.2 is needed because INCLUDE_DIRECTORIES(SYSTEM ...) wasn't
  # handled properly by the cmake Xcode generator.
  IF(CMAKE_GENERATOR STREQUAL "Xcode")
    CMAKE_MINIMUM_REQUIRED(VERSION 3.12.4)
  ELSE()
    CMAKE_MINIMUM_REQUIRED(VERSION 3.9.2)
  ENDIF()
ELSEIF(UNIX)
  # This is currently minimum version on all supported platforms.
  IF(CMAKE_VERSION VERSION_LESS 3.5.1)
    # Default cmake is 2.8.12.2 on RedHat
    IF(EXISTS "/etc/redhat-release")
      MESSAGE(WARNING "Please use cmake3 rather than cmake on this platform")
      FIND_PROGRAM(MY_CMAKE3 cmake3 /bin /usr/bin /usr/local/bin)
      IF(MY_CMAKE3)
        MESSAGE(STATUS "Found ${MY_CMAKE3}")
      ELSE()
        MESSAGE(STATUS "Please install cmake3 (yum install cmake3)")
      ENDIF()
    ELSE()
      # On SunOS /opt/csw/bin/cmake is (most likely) too old.
      FIND_PROGRAM(MY_UNAME uname /bin /usr/bin /usr/local/bin /sbin)
      IF(MY_UNAME)
        EXEC_PROGRAM(uname ARGS -s OUTPUT_VARIABLE MY_HOST_SYSTEM_NAME)
        IF(MY_HOST_SYSTEM_NAME MATCHES "SunOS")
          FIND_PROGRAM(MY_CMAKE cmake /usr/bin
            NO_CMAKE_ENVIRONMENT_PATH
            NO_SYSTEM_ENVIRONMENT_PATH
            )
          IF(MY_CMAKE)
            MESSAGE(STATUS "Found ${MY_CMAKE}")
            EXEC_PROGRAM(${MY_CMAKE} ARGS --version)
          ELSE()
            MESSAGE(STATUS "Please install /usr/bin/cmake ")
          ENDIF()
        ENDIF()
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

# CMake 3.5 is needed for TARGET_SOURCES(... $<TARGET_OBJECTS:${LIB}_objlib>)
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.1)

# Explicitly set CMP0018 and CMP0022 = NEW since newer versions of
# CMake has OLD as default even if OLD is deprecated.
# See cmake --help-policy CMP0018 / CMP0022
CMAKE_POLICY(SET CMP0018 NEW)
CMAKE_POLICY(SET CMP0022 NEW)

# Include TARGET_OBJECTS expressions.
CMAKE_POLICY(SET CMP0051 NEW)

# Disallow use of the LOCATION property for build targets.
IF(POLICY CMP0026)
  CMAKE_POLICY(SET CMP0026 NEW)
ENDIF()

# INTERPROCEDURAL_OPTIMIZATION is enforced when enabled (CMake 3.9+)
IF(POLICY CMP0069)
  CMAKE_POLICY(SET CMP0069 NEW)
ENDIF()

# In CMake 3.12 and above, the
#
# * ``check_include_file`` macro in the ``CheckIncludeFile`` module, the
# * ``check_include_file_cxx`` macro in the
#   ``CheckIncludeFileCXX`` module, and the
# * ``check_include_files`` macro in the ``CheckIncludeFiles`` module
#
# now prefer to link the check executable to the libraries listed in the
# ``CMAKE_REQUIRED_LIBRARIES`` variable.
IF(POLICY CMP0075)
  CMAKE_POLICY(SET CMP0075 OLD)
ENDIF()

# Will set GIT_EXECUTABLE and GIT_FOUND
FIND_PACKAGE(Git)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# First, decide about build type (debug or release)
# If cmake is invoked with -DCMAKE_BUILD_TYPE, 
# respect user wishes and do not (re)define CMAKE_BUILD_TYPE. If WITH_DEBUG
# is given, set CMAKE_BUILD_TYPE = Debug. Otherwise, use Relwithdebinfo.

IF(DEFINED CMAKE_BUILD_TYPE)
  SET(HAVE_CMAKE_BUILD_TYPE TRUE)
ENDIF()

OPTION(WITH_DEBUG "Use dbug/safemutex" OFF)
OPTION(CHECK_ERRMSG_FORMAT "Check printf format for English error messages" OFF)

# Use a default manufacturer if no manufacturer was identified.
SET(MANUFACTURER_DOCSTRING
  "Set the entity that appears as the manufacturer of packages that support a manufacturer field.")
IF(NOT DEFINED MANUFACTURER) 
  SET(MANUFACTURER "Built from Source" CACHE  STRING ${MANUFACTURER_DOCSTRING})
  MARK_AS_ADVANCED(MANUFACTURER)
ENDIF()


# MAX_INDEXES - Set the maximum number of indexes per table, default 64U
IF (NOT MAX_INDEXES)
  SET(MAX_INDEXES 64U)
ELSEIF(MAX_INDEXES MATCHES "^[0-9]+[Uu]?$")
  # MAX_INDEXES should be unsigned, so add the U suffix if it's missing.
  STRING(REGEX REPLACE "^([0-9]+).*$" "\\1U" MAX_INDEXES "${MAX_INDEXES}")
  MESSAGE(STATUS "Configuring with MAX_INDEXES = ${MAX_INDEXES}")
ELSE()
  MESSAGE(FATAL_ERROR "MAX_INDEXES should be an unsigned integer.")
ENDIF(NOT MAX_INDEXES)

IF(MAX_INDEXES GREATER 255)
  MESSAGE(FATAL_ERROR "MAX_INDEXES values greater than 255 is not supported!")
ELSEIF(MAX_INDEXES LESS 64)
  # Per documentation, ignore values less than 64 and use the default instead.
  MESSAGE(WARNING "MAX_INDEXES option ignored because it is less than 64.")
  SET(MAX_INDEXES 64U)
ENDIF()

# We choose to provide WITH_DEBUG as alias to standard CMAKE_BUILD_TYPE=Debug
# which turns out to be not trivial, as this involves synchronization 
# between CMAKE_BUILD_TYPE and WITH_DEBUG. Besides, we have to deal with cases
# where WITH_DEBUG is  reset from ON to OFF  and here we need to reset 
# CMAKE_BUILD_TYPE to either none or default RelWithDebInfo

SET(BUILDTYPE_DOCSTRING
 "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or
 CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel")

IF(WITH_DEBUG)
  SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING ${BUILDTYPE_DOCSTRING} FORCE)
  SET(OLD_WITH_DEBUG 1 CACHE INTERNAL "" FORCE)
ELSEIF(NOT HAVE_CMAKE_BUILD_TYPE OR OLD_WITH_DEBUG)
  IF(CMAKE_BUILD_TYPE MATCHES "Debug" OR NOT HAVE_CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING 
       ${BUILDTYPE_DOCSTRING} FORCE)
  ENDIF()
  SET(OLD_WITH_DEBUG 0 CACHE INTERNAL "" FORCE)
ENDIF()

STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)

IF(CMAKE_GENERATOR MATCHES "Visual Studio [1-9][0-9].*" AND
   CMAKE_GENERATOR_TOOLSET STREQUAL "")
  # Switch to 64 bit toolset on Windows (32 bit is default).
  # This is recommended as the 32 bit linker will run into address space issues
  # and not exit for long time.
  SET(CMAKE_GENERATOR_TOOLSET "host=x64")
ENDIF()

# On Linux el6/el7 the default gcc is too old, see if devtoolset is installed.
# Same with SLES 12, look for gcc 7 there.
# We need to look for gcc before calling PROJECT below.
OPTION(FORCE_UNSUPPORTED_COMPILER "Disable compiler version checks" OFF)
MARK_AS_ADVANCED(WITHOUT_SERVER FORCE_UNSUPPORTED_COMPILER)

# Use 'uname -r' and 'rpm -qf /' to figure out host system.
# For Docker images we cannot trust uname, so use rpm instead.
IF(UNIX)
  FIND_PROGRAM(MY_UNAME uname /bin /usr/bin /usr/local/bin /sbin)
  IF(MY_UNAME)
    EXECUTE_PROCESS(COMMAND ${MY_UNAME} -s
      OUTPUT_VARIABLE MY_HOST_SYSTEM_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE MY_UNAME_RESULT
      )
  ENDIF()
  FIND_PROGRAM(MY_RPM rpm /bin /usr/bin)
  IF(MY_RPM)
    EXECUTE_PROCESS(COMMAND ${MY_RPM} -qf /
      OUTPUT_VARIABLE MY_HOST_FILESYSTEM_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE MY_RPM_RESULT
      )
  ENDIF()
ENDIF()

IF(CMAKE_HOST_UNIX AND NOT FORCE_UNSUPPORTED_COMPILER
    AND NOT CMAKE_C_COMPILER AND NOT CMAKE_CXX_COMPILER)
  # Cannot INCLUDE(CMakeDetermineSystem) prior to PROJECT initialization below.
  SET (ENV_CC "$ENV{CC}")
  SET (ENV_CXX "$ENV{CXX}")
  IF (ENV_CC STREQUAL "" AND ENV_CXX STREQUAL "")
    IF(MY_UNAME)
      EXEC_PROGRAM(${MY_UNAME} ARGS -r OUTPUT_VARIABLE MY_HOST_SYSTEM_VERSION)
      IF(MY_HOST_SYSTEM_NAME MATCHES "Linux" AND
          (MY_HOST_SYSTEM_VERSION MATCHES "\\.el[678](uek)?\\.")
          OR
          (MY_HOST_FILESYSTEM_NAME MATCHES "\\.el[678]\\."))
        MESSAGE(STATUS
          "This is el6, el7 or el8 as found from 'uname -r' or 'rpm -qf /'")
        MESSAGE(STATUS "We probably need some devtoolset compiler")
        # el6 /opt/rh/devtoolset-8/root/usr/bin/gcc
        # el7 /opt/rh/devtoolset-9/root/usr/bin/gcc
        # el8 /opt/rh/gcc-toolset-9/root/usr/bin/gcc
        SET(ALTERNATIVE_PATHS
          "/opt/rh/devtoolset-9"
          "/opt/rh/gcc-toolset-9"
          "/opt/rh/devtoolset-8"
          )
        FOREACH(OPT_PATH ${ALTERNATIVE_PATHS})
          FIND_PROGRAM(ALTERNATIVE_GCC gcc
            NO_DEFAULT_PATH
            PATHS "${OPT_PATH}/root/usr/bin")
          FIND_PROGRAM(ALTERNATIVE_GPP g++
            NO_DEFAULT_PATH
            PATHS "${OPT_PATH}/root/usr/bin")
          FIND_PROGRAM(ALTERNATIVE_ENABLE enable
            NO_DEFAULT_PATH
            PATHS "${OPT_PATH}")
        ENDFOREACH()

        IF(ALTERNATIVE_GCC AND ALTERNATIVE_GPP)
          SET(CMAKE_C_COMPILER ${ALTERNATIVE_GCC})
          SET(CMAKE_CXX_COMPILER ${ALTERNATIVE_GPP})
          MESSAGE(STATUS "Using ${ALTERNATIVE_GCC}")
          MESSAGE(STATUS "Using ${ALTERNATIVE_GPP}")
        ELSE()
          MESSAGE(WARNING "Could not find devtoolset gcc")
        ENDIF()
      ELSEIF(MY_HOST_SYSTEM_NAME MATCHES "Linux" AND
          EXISTS "/etc/os-release")
        FILE(READ "/etc/os-release" MY_OS_RELEASE)
        IF (MY_OS_RELEASE MATCHES "SUSE Linux Enterprise Server 12")
          MESSAGE(STATUS "We need to look for a newer GCC on SLES 12")

          FIND_PROGRAM(ALTERNATIVE_GCC gcc-7
            NO_DEFAULT_PATH
            PATHS "/usr/bin")
          FIND_PROGRAM(ALTERNATIVE_GPP g++-7
            NO_DEFAULT_PATH
            PATHS "/usr/bin")
          IF (ALTERNATIVE_GCC AND ALTERNATIVE_GPP)
            SET(CMAKE_C_COMPILER ${ALTERNATIVE_GCC})
            SET(CMAKE_CXX_COMPILER ${ALTERNATIVE_GPP})
            MESSAGE(STATUS "Using ${ALTERNATIVE_GCC}")
            MESSAGE(STATUS "Using ${ALTERNATIVE_GPP}")
            # Use the new ABI so that std::string can be used with allocators
            # that are not default-constructible (e.g. Memroot_allocator)
            ADD_DEFINITIONS(-D_GLIBCXX_USE_CXX11_ABI=1)
          ELSE()
            MESSAGE(WARNING "Could not find newer gcc")
	  ENDIF()
	ENDIF()
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

# Optionally set project name, e.g.
# foo.xcodeproj (mac) or foo.sln (windows)
SET(MYSQL_PROJECT_NAME_DOCSTRING "MySQL project name")
IF(DEFINED MYSQL_PROJECT_NAME)
  SET(MYSQL_PROJECT_NAME ${MYSQL_PROJECT_NAME} CACHE STRING
      ${MYSQL_PROJECT_NAME_DOCSTRING} FORCE)
ELSE()
  SET(MYSQL_PROJECT_NAME "MySQL" CACHE STRING
      ${MYSQL_PROJECT_NAME_DOCSTRING} FORCE)
  MARK_AS_ADVANCED(MYSQL_PROJECT_NAME)
ENDIF()

# Remember if WITH_NDBCLUSTER was specified by user
IF(DEFINED WITH_NDBCLUSTER)
  SET(HAVE_WITH_NDBCLUSTER 1)
ENDIF()
OPTION(WITH_NDBCLUSTER "Build MySQL Cluster" OFF)
OPTION(WITH_NDBCLUSTER_STORAGE_ENGINE
  "Legacy and deprecated alias for WITH_NDBCLUSTER" OFF)
OPTION(WITH_PLUGIN_NDBCLUSTER
  "Legacy and deprecated alias for WITH_NDBCLUSTER" OFF)
MARK_AS_ADVANCED(WITH_NDBCLUSTER_STORAGE_ENGINE WITH_PLUGIN_NDBCLUSTER)
IF(NOT HAVE_WITH_NDBCLUSTER)
  # Unless user has used WITH_NDBCLUSTER also look at
  # some of the legacy variables
  IF(WITH_PLUGIN_NDBCLUSTER)
    MESSAGE(WARNING "WITH_PLUGIN_NDBCLUSTER is deprecated and will be "
      "removed in a future release. Please use WITH_NDBCLUSTER instead.")
    SET(WITH_NDBCLUSTER ON)
  ENDIF()
  IF(WITH_NDBCLUSTER_STORAGE_ENGINE)
    MESSAGE(WARNING "WITH_NDBCLUSTER_STORAGE_ENGINE is deprecated and "
      "will be removed in a future release. Please use WITH_NDBCLUSTER "
      "instead.")
    SET(WITH_NDBCLUSTER ON)
  ENDIF()
ENDIF()
IF(WITH_NDBCLUSTER)
  MESSAGE(STATUS "Building MySQL Cluster")
ENDIF()

INCLUDE(mysql_version)
IF(POLICY CMP0048)
  CMAKE_POLICY(SET CMP0048 NEW)
  PROJECT(${MYSQL_PROJECT_NAME}
    VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
ELSE()
  PROJECT(${MYSQL_PROJECT_NAME})
ENDIF()

GET_FILENAME_COMPONENT(REALPATH_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR} REALPATH)
GET_FILENAME_COMPONENT(REALPATH_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR} REALPATH)

MESSAGE(STATUS "Source directory ${REALPATH_CMAKE_SOURCE_DIR}")
MESSAGE(STATUS "Binary directory ${REALPATH_CMAKE_BINARY_DIR}")

# IN PB2 we have a few configurations which are built in-source (e.g. doxygen).
IF(DEFINED ENV{PB2WORKDIR})
  OPTION(FORCE_INSOURCE_BUILD "Allow in-source build" ON)
ELSE()
  OPTION(FORCE_INSOURCE_BUILD "Allow in-source build" OFF)
ENDIF()

# https://gitlab.kitware.com/cmake/community/wikis/FAQ
# cmake-does-not-generate-a-make-distclean-target-why
# Why disallow in-source build?
# Basically because 'make clean' or 'make distclean' do not work.
# So if you do a 'git pull' you may end up with a developer sandbox
# that no longer works as expected (CMakeCache.txt and other
# generated/configured files may contain data which is no longer valid)
IF(${REALPATH_CMAKE_SOURCE_DIR} STREQUAL ${REALPATH_CMAKE_BINARY_DIR})
  IF(FORCE_INSOURCE_BUILD)
    MESSAGE(WARNING "This is an in-source build")
  ELSE()
    MESSAGE(FATAL_ERROR
      "Please do not build in-source. "
      "Out-of source builds are highly recommended: "
      "you can have multiple builds for the same source, "
      "and there is an easy way to do cleanup, "
      "simply remove the build directory "
      "(note that 'make clean' or 'make distclean' does *not* work) "
      "\nYou *can* force in-source build by invoking cmake with -DFORCE_INSOURCE_BUILD=1")
  ENDIF()
ENDIF()

MACRO(REPORT_CXX_FLAGS)
  MESSAGE(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
  FOREACH(BUILD_TYPE "" _DEBUG _RELWITHDEBINFO _RELEASE _MINSIZEREL)
    SET(flag "CMAKE_CXX_FLAGS${BUILD_TYPE}")
    MESSAGE(STATUS "${flag}: ${${flag}}")
  ENDFOREACH()
ENDMACRO()

# STRING(APPEND ...) from cmake VERSION 3.4
MACRO(STRING_APPEND STRING_VAR INPUT)
  SET(${STRING_VAR} "${${STRING_VAR}}${INPUT}")
ENDMACRO()

MACRO(STRING_PREPEND STRING_VAR INPUT)
  SET(${STRING_VAR} "${INPUT}${${STRING_VAR}}")
ENDMACRO()

# Write content to file, using CONFIGURE_FILE
# The advantage compared to FILE(WRITE) is that timestamp
# does not change if file already has the same content
SET(MYSQL_CMAKE_SCRIPT_DIR "${CMAKE_SOURCE_DIR}/cmake")
MACRO(CONFIGURE_FILE_CONTENT content file)
  SET(CMAKE_CONFIGURABLE_FILE_CONTENT
    "${content}\n")
  CONFIGURE_FILE(
    ${MYSQL_CMAKE_SCRIPT_DIR}/configurable_file_content.in
    ${file}
    @ONLY)
ENDMACRO()

SET(BUILD_IS_SINGLE_CONFIG TRUE)
MESSAGE(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
IF(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR STREQUAL "Xcode")
  SET(BUILD_IS_SINGLE_CONFIG FALSE)
ENDIF()

IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(MY_COMPILER_IS_CLANG 1)
ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  SET(MY_COMPILER_IS_GNU 1)
ELSEIF(CMAKE_CXX_COMPILER_ID MATCHES "SunPro")
  SET(MY_COMPILER_IS_SUNPRO 1)
ENDIF()

IF(MY_COMPILER_IS_CLANG OR MY_COMPILER_IS_GNU)
  SET(MY_COMPILER_IS_GNU_OR_CLANG 1)
ENDIF()

# Maintainer mode is default on only for debug builds using GCC/G++
IF(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG" OR WITH_DEBUG)
  IF(MY_COMPILER_IS_GNU)
    SET(MYSQL_MAINTAINER_MODE ON CACHE BOOL
        "MySQL maintainer-specific development environment")
  ENDIF()
ENDIF()

OPTION(WITH_DEFAULT_COMPILER_OPTIONS
  "Use flags from cmake/build_configurations/compiler_options.cmake"
  ON)
OPTION(WITH_DEFAULT_FEATURE_SET
  "Use feature set in cmake/build_configurations/feature_set.cmake"
  ON)
IF(BUILD_CONFIG)
  INCLUDE(
  ${CMAKE_SOURCE_DIR}/cmake/build_configurations/${BUILD_CONFIG}.cmake)
ENDIF()

OPTION(INSTALL_STATIC_LIBRARIES "Install static libraries" ON)

#cmake on 64bit windows/mac/solaris doesn't set CMAKE_SYSTEM_PROCESSOR correctly
SET(MYSQL_MACHINE_TYPE ${CMAKE_SYSTEM_PROCESSOR})

SET(KNOWN_64BIT_ARCHITECTURES
  aarch64
  ppc64
  ppc64le
  s390x
  x86_64
)

# Include the platform-specific file. To allow exceptions, this code
# looks for files in order of how specific they are. If there is, for
# example, a generic Linux.cmake and a version-specific
# Linux-2.6.28-11-generic, it will pick Linux-2.6.28-11-generic and
# include it. It is then up to the file writer to include the generic
# version if necessary.
FOREACH(_base
    ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}
    ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}
    ${CMAKE_SYSTEM_NAME})
  SET(_file ${CMAKE_SOURCE_DIR}/cmake/os/${_base}.cmake)
  IF(EXISTS ${_file})
    INCLUDE(${_file})
    BREAK()
  ENDIF()
ENDFOREACH()


IF(UNIX)
  OPTION(WITH_INNODB_MEMCACHED "" OFF)
  OPTION(ENABLE_MEMCACHED_SASL "Enable SASL on InnoDB Memcached" OFF)
  OPTION(ENABLE_MEMCACHED_SASL_PWDB "Enable SASL on InnoDB Memcached" OFF)
ELSE()
  OPTION(WITH_INNODB_MEMCACHED "" OFF)
ENDIF()
 
# Following autotools tradition, add preprocessor definitions
# specified in environment variable CPPFLAGS
IF(DEFINED ENV{CPPFLAGS})
  ADD_DEFINITIONS($ENV{CPPFLAGS})
ENDIF()

INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE("void *" SIZEOF_VOIDP)
MESSAGE(STATUS "SIZEOF_VOIDP ${SIZEOF_VOIDP}")
# On some platforms, cmake may think that CMAKE_SIZEOF_VOID_P == 4
# even if we have configured for 64bit build....
SET(CMAKE_SIZEOF_VOID_P ${SIZEOF_VOIDP})

IF(WITH_DEFAULT_COMPILER_OPTIONS)
  INCLUDE(${CMAKE_SOURCE_DIR}/cmake/build_configurations/compiler_options.cmake)
ENDIF()
IF(WITH_DEFAULT_FEATURE_SET)
  INCLUDE(${CMAKE_SOURCE_DIR}/cmake/build_configurations/feature_set.cmake)
ENDIF()

INCLUDE(CMakePushCheckState)

# Add macros
INCLUDE(pkg-config)
INCLUDE(character_sets)
INCLUDE(compile_flags)
INCLUDE(cpu_info)
INCLUDE(fileutils)
INCLUDE(zlib)
INCLUDE(zstd)
INCLUDE(lz4)
INCLUDE(icu)
INCLUDE(libevent)
INCLUDE(ssl)
INCLUDE(sasl)
INCLUDE(ldap)
INCLUDE(kerberos)
INCLUDE(rpc)
INCLUDE(readline)
INCLUDE(protobuf)
INCLUDE(package_name)
INCLUDE(libutils)
INCLUDE(plugin)
INCLUDE(component)
INCLUDE(install_macros)
INCLUDE(install_layout)
INCLUDE(mysql_add_executable)
INCLUDE(curl)
INCLUDE(rapidjson)
INCLUDE(fprofile)
INCLUDE(prepend_append_cflags_if_supported)

OPTION(DISABLE_SHARED
 "Don't build shared libraries, compile code as position-dependent" OFF)
IF(DISABLE_SHARED)
  MESSAGE("Dynamic plugins are disabled.")
ENDIF()

IF(NOT DISABLE_SHARED)
  SET(PIC_EXT "_pic")
ELSE()
  SET(PIC_EXT "")
ENDIF()

OPTION(WITH_JEMALLOC "Use jemalloc rather than builtin malloc/free etc." OFF)
IF(WITH_JEMALLOC)
  IF(EXISTS "${WITH_JEMALLOC}/lib")
    # If WITH_JEMALLOC points to a path
    MESSAGE("${WITH_JEMALLOC}/lib/libjemalloc${PIC_EXT}.a")
    FIND_LIBRARY(JEMALLOC_SYSTEM_LIBRARY NAMES libjemalloc${PIC_EXT}.a HINTS ${WITH_JEMALLOC}/lib)
  ELSE()
    FIND_LIBRARY(JEMALLOC_SYSTEM_LIBRARY NAMES jemalloc)
    STRING_APPEND(CMAKE_EXE_LINKER_FLAGS " -ljemalloc")
    STRING_APPEND(CMAKE_MODULE_LINKER_FLAGS " -ljemalloc")
    STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS " -ljemalloc")
  ENDIF()
  IF(NOT JEMALLOC_SYSTEM_LIBRARY)
    MESSAGE(FATAL_ERROR "Could not find jemalloc library")
  ENDIF()
  SET(HAVE_JEMALLOC ON)
  SET(JEMALLOC_LIBRARY ${JEMALLOC_SYSTEM_LIBRARY})
  STRING_APPEND(CMAKE_C_FLAGS " -fno-builtin-malloc -fno-builtin-calloc")
  STRING_APPEND(CMAKE_C_FLAGS " -fno-builtin-realloc -fno-builtin-free")
  STRING_APPEND(CMAKE_CXX_FLAGS " -fno-builtin-malloc -fno-builtin-calloc")
  STRING_APPEND(CMAKE_CXX_FLAGS " -fno-builtin-realloc -fno-builtin-free")
ENDIF()

OPTION(ENABLED_PROFILING "Enable profiling" ON)
OPTION(WITHOUT_SERVER OFF)
IF(UNIX)
  OPTION(WITH_VALGRIND "Valgrind instrumentation" OFF)
ENDIF()
IF(WIN32)
  OPTION(WITH_MSCRT_DEBUG "MS Visual Studio Debug CRT instrumentation" OFF)
ENDIF()
IF(NOT WITHOUT_SERVER)
  OPTION(WITH_UNIT_TESTS "Compile MySQL with unit tests" ON)
  OPTION(WITH_ROUTER "Build MySQL Router" ON)
ENDIF()

IF(EXISTS ${CMAKE_SOURCE_DIR}/internal/CMakeLists.txt)
  SET(WITH_INTERNAL_DEFAULT 1)
ELSE()
  SET(WITH_INTERNAL_DEFAULT 0)
ENDIF()

IF(NOT DEFINED WITH_INTERNAL)
  SET(WITH_INTERNAL ${WITH_INTERNAL_DEFAULT})
ENDIF()

# On windows we need a non-standard package for SASL.
IF(WITH_INTERNAL AND (NOT WIN32 OR WITH_SASL))
  SET(WITH_AUTHENTICATION_LDAP_DEFAULT ON)
ELSE()
  SET(WITH_AUTHENTICATION_LDAP_DEFAULT OFF)
ENDIF()

OPTION(WITH_AUTHENTICATION_LDAP
  "Report error if the LDAP authentication plugin cannot be built."
  ${WITH_AUTHENTICATION_LDAP_DEFAULT})

OPTION(WITH_LOCK_ORDER
  "Build the lock order mutex instrumentation code." OFF)

IF(WITH_LOCK_ORDER)
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E make_directory lock_order
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
ENDIF()

IF(DEFINED WITH_LTO_DEFAULT)
  SET(WITH_LTO_DEFAULT ${WITH_LTO_DEFAULT})
ELSE()
  SET(WITH_LTO_DEFAULT OFF)
ENDIF()

OPTION(WITH_LTO
  "Enable the link-time optimizer. Currently works for gcc7/gcc8/gcc9/macos only."
  ${WITH_LTO_DEFAULT}
  )

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
# We need some extra FAIL_REGEX patterns
# Note that CHECK_C_SOURCE_COMPILES is a misnomer, it will also link.
MACRO (MY_CHECK_C_COMPILER_FLAG FLAG RESULT)
  CMAKE_PUSH_CHECK_STATE()
  STRING_APPEND(CMAKE_REQUIRED_FLAGS " ${FLAG}")
  # As mentioned above, CMAKE_C_SOURCE_COMPILES not only compiles the source
  # but also performs linking to create the binary. The problem is that cmake
  # automatically appends CFLAGS to the linker command, and there's no way to
  # stop that.
  # When '-nostdinc' option is included in CFLAGS, clang spits
  # 'unused-command-line-argument' warning as the option is only valid for
  # a compiler but not for a linker. Here we temporarily silience the warning
  # for linking to pass the check. The warning is still enabled for the
  # compilation command.
  IF(CMAKE_C_COMPILER_ID MATCHES "Clang")
    STRING_APPEND(CMAKE_REQUIRED_FLAGS " -Wno-unused-command-line-argument")
  ENDIF()
  CHECK_C_SOURCE_COMPILES("int main(void) { return 0; }" ${RESULT}
    FAIL_REGEX "unknown argument ignored"
    FAIL_REGEX "argument unused during compilation"
    FAIL_REGEX "unsupported .*option"
    FAIL_REGEX "unknown .*option"
    FAIL_REGEX "unrecognized .*option"
    FAIL_REGEX "ignoring unknown option"
    FAIL_REGEX "[Ww]arning: [Oo]ption"
    FAIL_REGEX "error: visibility"
    FAIL_REGEX "warning: visibility"
    )
  CMAKE_POP_CHECK_STATE()
ENDMACRO()

MACRO (MY_CHECK_CXX_COMPILER_FLAG FLAG RESULT)
  CMAKE_PUSH_CHECK_STATE()
  STRING_APPEND(CMAKE_REQUIRED_FLAGS " ${FLAG}")
  IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    STRING_APPEND(CMAKE_REQUIRED_FLAGS " -Wno-unused-command-line-argument")
  ENDIF()
  CHECK_CXX_SOURCE_COMPILES("int main(void) { return 0; }" ${RESULT}
    FAIL_REGEX "unknown argument ignored"
    FAIL_REGEX "argument unused during compilation"
    FAIL_REGEX "unsupported .*option"
    FAIL_REGEX "unknown .*option"
    FAIL_REGEX "unrecognized .*option"
    FAIL_REGEX "ignoring unknown option"
    FAIL_REGEX "[Ww]arning: [Oo]ption"
    FAIL_REGEX "error: visibility"
    FAIL_REGEX "warning: visibility"
    )
  CMAKE_POP_CHECK_STATE()
ENDMACRO()

# Check whether a specific warning option is supported.
# Intended use is primarily to silence warnings from third-party code.
# If *our* code has warnings with gcc[56789] or clang[345678] the
# code should be fixed, rather than silencing the compiler warnings.
#
# Returns the corresponding "-Wno-<option>" if supported, 0 otherwise.
# WARNING_OPTION should be as found in clang/gcc documentation.
# Note: Sun Studio accepts/ignores anything starting with "-W".
#       Visual Studio has a few /W options, but are totally different.
# Hence we check for gcc/clang only.
# No return value for functions in cmake, so we use an output argument.
FUNCTION(MY_CHECK_CXX_COMPILER_WARNING WARNING_OPTION RETURN_VALUE)
  # Strip off possible leading -Wno- or -W
  STRING(REGEX REPLACE "^-Wno-" "" WARNING_OPTION ${WARNING_OPTION})
  STRING(REGEX REPLACE "^-W" "" WARNING_OPTION ${WARNING_OPTION})
  # Use HAVE_CXX_W_warning_option as cache variable.
  STRING(REPLACE "-" "_" VAR_NAME ${WARNING_OPTION})
  STRING(REPLACE "=" "_" VAR_NAME ${VAR_NAME})
  STRING(REPLACE "c++" "cpp" VAR_NAME ${VAR_NAME})
  SET(VAR_NAME "HAVE_CXX_W_${VAR_NAME}")
  IF(MY_COMPILER_IS_GNU_OR_CLANG)
    MY_CHECK_CXX_COMPILER_FLAG("-W${WARNING_OPTION}" ${VAR_NAME})
  ELSE()
    SET(${VAR_NAME} 0 CACHE INTERNAL "-W${WARNING_OPTION} not supported")
  ENDIF()
  IF(${VAR_NAME})
    SET(${RETURN_VALUE} "-Wno-${WARNING_OPTION}" PARENT_SCOPE)
  ELSE()
    SET(${RETURN_VALUE} 0 PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()

MACRO(MY_SANITIZER_CHECK SAN_OPT ADD_OPTIMIZATION RESULT)
  MY_CHECK_C_COMPILER_FLAG("${SAN_OPT}" C_RESULT)
  MY_CHECK_CXX_COMPILER_FLAG("${SAN_OPT}" CXX_RESULT)
  IF(C_RESULT AND CXX_RESULT)
    STRING_APPEND(CMAKE_C_FLAGS   " ${SAN_OPT}")
    STRING_APPEND(CMAKE_CXX_FLAGS " ${SAN_OPT}")
    # We switch on basic optimization also for debug builds.
    # We disable inlining for stack trace readability.
    # With optimization we may get some warnings, so we switch off -Werror
    IF(${ADD_OPTIMIZATION})
      IF(MSVC)
        STRING_APPEND(CMAKE_C_FLAGS   " -O1 /Ob0")
        STRING_APPEND(CMAKE_CXX_FLAGS " -O1 /Ob0")
      ELSE()
        STRING_APPEND(CMAKE_C_FLAGS   " -O1 -fno-inline")
        STRING_APPEND(CMAKE_CXX_FLAGS " -O1 -fno-inline")
      ENDIF()
      IF(NOT SANITIZE_MAINTAINER_MODE)
        SET(MYSQL_MAINTAINER_MODE OFF CACHE BOOL
          "MySQL maintainer-specific development environment" FORCE)
      ENDIF()
    ENDIF()

    IF (CMAKE_C_COMPILER_ID MATCHES "Clang")
      if (DEFINED ENV{SAN_SYMBOLIZER_FLAGS})
        MESSAGE(STATUS "SAN_SYMBOLIZER_FLAGS defined")
        SET(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} $ENV{SAN_SYMBOLIZER_FLAGS}")
      ENDIF()
    ENDIF()
    SET(${RESULT} 1)
  ELSE()
    SET(${RESULT} 0)
  ENDIF()
ENDMACRO()

OPTION(WITH_ASAN "Enable address sanitizer" OFF)
IF(WITH_ASAN)
  IF(WIN32_CLANG)
    # NOTE: We do _not_ modify CMAKE_C_FLAGS / CMAKE_CXX_FLAGS to add
    # -fsanitize=address here yet; Clang (at least of 5.0.0) requires /MT,
    # but CMake adds /MDd to all feature checks almost no matter what we do.
    # For the same reason, we have to enable it unconditionalyl without
    # doing a feature test; we know it's working. Modifying the flags happens
    # below, near the end.
    SET(HAVE_ASAN 1)

    # Get the path to the ASAN support libraries.
    GET_FILENAME_COMPONENT(ASAN_LIB_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
    SET(ASAN_LIB_DIR "${ASAN_LIB_DIR}/../lib/clang/${CMAKE_CXX_COMPILER_VERSION}/lib/windows")
    GET_FILENAME_COMPONENT(ASAN_LIB_DIR ${ASAN_LIB_DIR} REALPATH)
  ELSE()
    MY_SANITIZER_CHECK("-fsanitize=address" TRUE WITH_ASAN_OK)
    IF(WITH_ASAN_OK)
      OPTION(WITH_ASAN_SCOPE "Enable -fsanitize-address-use-after-scope" ON)
      SET(HAVE_ASAN 1)
      # This works with clang, but not gcc it seems.
      MY_CHECK_CXX_COMPILER_FLAG(
        "-fsanitize=address -fsanitize-address-use-after-scope"
        HAVE_SANITIZE_SCOPE)
      IF(WITH_ASAN_SCOPE AND HAVE_SANITIZE_SCOPE)
        STRING_APPEND(CMAKE_C_FLAGS   " -fsanitize-address-use-after-scope")
        STRING_APPEND(CMAKE_CXX_FLAGS " -fsanitize-address-use-after-scope")
      ENDIF()
    ELSE()
      MESSAGE(FATAL_ERROR "Do not know how to enable address sanitizer")
    ENDIF()
  ENDIF()
ENDIF()

OPTION(WITH_LSAN "Enable leak sanitizer" OFF)
IF(WITH_LSAN)
  MY_SANITIZER_CHECK("-fsanitize=leak"
    TRUE WITH_LSAN_OK)
  IF(NOT WITH_LSAN_OK)
    MESSAGE(FATAL_ERROR "Do not know how to enable leak sanitizer")
  ELSE()
    SET(HAVE_LSAN 1)
  ENDIF()
ENDIF()

OPTION(WITH_MSAN "Enable memory sanitizer" OFF)
IF(WITH_MSAN)
  MY_SANITIZER_CHECK("-fsanitize=memory -fsanitize-memory-track-origins"
    TRUE WITH_MSAN_OK)
  IF(NOT WITH_MSAN_OK)
    MESSAGE(FATAL_ERROR "Do not know how to enable memory sanitizer")
  ELSE()
    MESSAGE(WARNING "Memory sanitizer support is currently experimental.")
  ENDIF()
ENDIF()

OPTION(WITH_UBSAN "Enable undefined behavior sanitizer" OFF)
IF(WITH_UBSAN)
  MY_SANITIZER_CHECK("-fsanitize=undefined" TRUE WITH_UBSAN_OK)
  STRING_APPEND(CMAKE_CXX_FLAGS " -fno-sanitize=vptr,alignment")
  IF(NOT WITH_UBSAN_OK)
    MESSAGE(FATAL_ERROR
            "Do not know how to enable undefined behavior sanitizer")
  ELSE()
    SET(HAVE_UBSAN 1)
  ENDIF()
ENDIF()

OPTION(WITH_TSAN "Enable thread sanitizer" OFF)
IF(WITH_TSAN)
  MY_SANITIZER_CHECK("-fsanitize=thread" TRUE WITH_TSAN_OK)
  IF(NOT WITH_TSAN_OK)
    MESSAGE(FATAL_ERROR "Do not know how to enable thread sanitizer")
  ELSE()
    MESSAGE(WARNING "Thread sanitizer support is currently experimental.")
    SET(HAVE_TSAN 1)
  ENDIF()
ENDIF()

IF(WITH_ASAN AND WITH_MSAN)
  MESSAGE(FATAL_ERROR
          "Cannot use AddressSanitizer and MemorySanitizer together")
ENDIF()


# Older versions of ccache must be disabled: export CCACHE_DISABLE=1
# See http://www.cmake.org/Wiki/CTest/Coverage
OPTION(ENABLE_GCOV "Enable gcov (debug, Linux builds only)" OFF)
IF (ENABLE_GCOV AND NOT WIN32 AND NOT APPLE)
  STRING_APPEND(CMAKE_C_FLAGS   " -fprofile-arcs -ftest-coverage -DHAVE_GCOV")
  STRING_APPEND(CMAKE_CXX_FLAGS " -fprofile-arcs -ftest-coverage -DHAVE_GCOV")
  STRING_APPEND(CMAKE_EXE_LINKER_FLAGS " -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()

OPTION(ENABLE_GPROF "Enable gprof (optimized, Linux builds only)" OFF)
IF (ENABLE_GPROF AND NOT WIN32 AND NOT APPLE)
  STRING_APPEND(CMAKE_C_FLAGS " -pg")
  STRING_APPEND(CMAKE_CXX_FLAGS " -pg")
  STRING_APPEND(CMAKE_EXE_LINKER_FLAGS " -pg")
ENDIF()

# Use lld for Clang if available and not explicitly disabled.
# Also works for gcc on Debian/Ubuntu. Do 'apt install lld'.
# LTO build fails with lld, so turn it off by default.
IF(LINUX AND NOT WITH_LTO)
  OPTION(USE_LD_LLD "Use llvm lld linker" ON)
ELSE()
  OPTION(USE_LD_LLD "Use llvm lld linker" OFF)
ENDIF()

IF(USE_LD_LLD)
  CMAKE_PUSH_CHECK_STATE(RESET)

  # RPM builds on Fedora use -specs files.
  # This may cause the test for -fuse-ld=lld to succeed, when it should fail.
  SET(SAVE_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
  SET(SAVE_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  STRING(REGEX REPLACE "-specs=/usr/lib/rpm/redhat/.*"  ""
    CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
  STRING(REGEX REPLACE "-specs=/usr/lib/rpm/redhat/.*"  ""
    CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

  SET(CMAKE_REQUIRED_LIBRARIES "-fuse-ld=lld")
  CHECK_C_SOURCE_COMPILES("int main() {}" C_LD_LLD_RESULT)
  CHECK_CXX_SOURCE_COMPILES("int main() {}" CXX_LD_LLD_RESULT)
  IF(C_LD_LLD_RESULT AND CXX_LD_LLD_RESULT)
    STRING_APPEND(CMAKE_C_LINK_FLAGS " -fuse-ld=lld")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -fuse-ld=lld")
  ENDIF()
  SET(CMAKE_C_FLAGS ${SAVE_CMAKE_C_FLAGS})
  SET(CMAKE_CXX_FLAGS ${SAVE_CMAKE_CXX_FLAGS})
  CMAKE_POP_CHECK_STATE()
ENDIF()

# Use gold on x86 if available and not explicitly disabled.
IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT WIN32)
  OPTION(USE_LD_GOLD "Use GNU gold linker" ON)
ELSE()
  OPTION(USE_LD_GOLD "Use GNU gold linker" OFF)
ENDIF()

IF(LINUX)
  OPTION(LINK_RANDOMIZE "Randomize the order of all symbols in the binary" OFF)
  SET(LINK_RANDOMIZE_SEED "mysql"
    CACHE STRING "Seed to use for link randomization")
ELSE()
  SET(LINK_RANDOMIZE OFF)
ENDIF()

IF (USE_LD_GOLD AND NOT (C_LD_LLD_RESULT AND CXX_LD_LLD_RESULT))
  CMAKE_PUSH_CHECK_STATE(RESET)
  SET(CMAKE_REQUIRED_LIBRARIES "-fuse-ld=gold")
  CHECK_C_SOURCE_COMPILES("int main() {}" C_LD_GOLD_RESULT)
  CHECK_CXX_SOURCE_COMPILES("int main() {}" CXX_LD_GOLD_RESULT)
  IF (C_LD_GOLD_RESULT AND CXX_LD_GOLD_RESULT AND NOT SOLARIS)
    STRING_APPEND(CMAKE_C_LINK_FLAGS " -fuse-ld=gold")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -fuse-ld=gold")

    # Seemingly --gc-sections causes problems with randomization, so only
    # turn it on if we are not making a randomized build.
    IF (NOT LINK_RANDOMIZE)
      STRING_APPEND(CMAKE_C_LINK_FLAGS " -Wl,--gc-sections")
      STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -Wl,--gc-sections")
    ENDIF()
  ENDIF()
  CMAKE_POP_CHECK_STATE()
ENDIF()

# Linker/binutils bug in Fedora 28, undefined reference to symbol '__bss_start'
# Similar bug on 32bit Ubuntu.
IF(LINUX_FEDORA_28 OR LINUX_UBUNTU_16_04)
  IF(NOT CMAKE_CXX_LINK_FLAGS MATCHES "-fuse-ld=lld" AND
      NOT CMAKE_CXX_LINK_FLAGS MATCHES "-fuse-ld=gold")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -Wl,--copy-dt-needed-entries")
  ENDIF()
ENDIF()

IF(LINK_RANDOMIZE)
  STRING_APPEND(CMAKE_C_FLAGS   " -ffunction-sections -fdata-sections")
  STRING_APPEND(CMAKE_CXX_FLAGS " -ffunction-sections -fdata-sections")

  SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--sort-section=name")
  SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--sort-section=name")

  SET(CMAKE_CXX_ARCHIVE_CREATE "sh -c '${CMAKE_SOURCE_DIR}/scripts/randomize-order.pl ${LINK_RANDOMIZE_SEED} <OBJECTS> && <CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>'")
  SET(CMAKE_CXX_ARCHIVE_CREATE_APPEND "sh -c '${CMAKE_SOURCE_DIR}/scripts/randomize-order.pl ${LINK_RANDOMIZE_SEED} <OBJECTS> && <CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>'")
  SET(CMAKE_C_ARCHIVE_CREATE ${CMAKE_CXX_ARCHIVE_CREATE})
  SET(CMAKE_C_ARCHIVE_CREATE_APPEND ${CMAKE_CXX_ARCHIVE_CREATE_APPEND})
ENDIF()

# Gold and LLD support the --gdb-index option, which adds a .gdb_index
# section to the binaries. It makes loading the binaries in gdb much
# faster. The overhead of generating the index is small in LLD.
OPTION(ADD_GDB_INDEX "Generate a .gdb_index section in the binaries." OFF)

IF(ADD_GDB_INDEX AND
    (CMAKE_CXX_LINK_FLAGS MATCHES "-fuse-ld=lld" OR
      CMAKE_CXX_LINK_FLAGS MATCHES "-fuse-ld=gold"))
  # -fuse-ld=lld is not enough to ensure that ld.lld will actually be used.
  # On some platforms we need an extra check for -Wl,--gdb-index
  CMAKE_PUSH_CHECK_STATE()
  IF(CMAKE_CXX_LINK_FLAGS MATCHES "-fuse-ld=lld")
    SET(CMAKE_REQUIRED_LIBRARIES "-fuse-ld=lld -Wl,--gdb-index")
  ELSE()
    SET(CMAKE_REQUIRED_LIBRARIES "-fuse-ld=gold -Wl,--gdb-index")
  ENDIF()
  CHECK_CXX_SOURCE_COMPILES("int main() {}" CXX_LD_GDB_INDEX_RESULT)
  CMAKE_POP_CHECK_STATE()

  IF(CXX_LD_GDB_INDEX_RESULT)
    STRING_APPEND(CMAKE_C_LINK_FLAGS " -Wl,--gdb-index")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -Wl,--gdb-index")
    # gdb sometimes reports "incomplete type" after linking with
    # --gdb-index. This flag makes the index contain enough
    # information to avoid the problem.
    STRING_APPEND(CMAKE_C_FLAGS " -ggnu-pubnames")
    STRING_APPEND(CMAKE_CXX_FLAGS " -ggnu-pubnames")
  ELSE()
    MESSAGE(STATUS "Seems like linker does not support --gdb-index")
  ENDIF()
ENDIF()

IF(LINUX)
  # May be set to ON in cmake/build_configurations/mysql_release.cmake
  IF(DEFINED REPRODUCIBLE_BUILD)
    SET(REPRODUCIBLE_BUILD_DEFAULT ${REPRODUCIBLE_BUILD})
  ELSE()
    SET(REPRODUCIBLE_BUILD_DEFAULT OFF)
  ENDIF()
  OPTION(REPRODUCIBLE_BUILD
    "Take extra pains to make build result independent of build location and time"
    ${REPRODUCIBLE_BUILD_DEFAULT})
ENDIF()
IF(REPRODUCIBLE_BUILD)
  SET(DEBUG_PREFIX_FLAGS
    "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}/=./ -fdebug-prefix-map=${CMAKE_CURRENT_BINARY_DIR}=./obj")

  # See if -fdebug-prefix= commands are included in the debug output,
  # making the build unreproducible with switches recorded.
  # See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69821.
  EXECUTE_PROCESS(COMMAND
    ${CMAKE_C_COMPILER} -g3 -x c -S -fdebug-prefix-map=foo=bar -o - -
    INPUT_FILE /dev/null OUTPUT_VARIABLE DEBUG_PREFIX_MAP_RESULT)
  IF(DEBUG_PREFIX_MAP_RESULT MATCHES "foo=bar")
    SET(DEBUG_PREFIX_FLAGS "${DEBUG_PREFIX_FLAGS} -gno-record-gcc-switches")
  ENDIF()

  STRING_APPEND(CMAKE_C_FLAGS   " ${DEBUG_PREFIX_FLAGS}")
  STRING_APPEND(CMAKE_CXX_FLAGS " ${DEBUG_PREFIX_FLAGS}")

  STRING_APPEND(CMAKE_C_LINK_FLAGS " -Wl,--build-id=none")
  STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -Wl,--build-id=none")
  SET(BISON_NO_LINE_OPT "--no-lines")

  SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE
    "${CMAKE_SOURCE_DIR}/scripts/invoke-with-relative-paths.pl")
ENDIF()

OPTION(ENABLED_LOCAL_INFILE
 "If we should enable LOAD DATA LOCAL by default" OFF)
MARK_AS_ADVANCED(ENABLED_LOCAL_INFILE)

#
# Options related to client-side protocol tracing
#

OPTION(WITH_CLIENT_PROTOCOL_TRACING
  "Support for client-side protocol tracing plugins" ON)
OPTION(WITH_TEST_TRACE_PLUGIN
  "Have a built-in test protocol trace plugin in libmysql (requires WITH_CLIENT_PROTOCOL_TRACING option)" OFF)

# Sanity checks for protocol tracing options

IF(WITH_TEST_TRACE_PLUGIN AND NOT WITH_CLIENT_PROTOCOL_TRACING)
  MESSAGE("WARNING: Test trace plugin was selected but client protocol tracing
           infrastructure is not enabled - ignoring")
  SET(WITH_TEST_TRACE_PLUGIN 0)
ENDIF()

IF(WITH_TEST_TRACE_PLUGIN AND NOT CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
  MESSAGE(SEND_ERROR 
    "Test trace plugin was selected but it can be included only in
     debug binaries. Set WITH_TEST_TRACE_PLUGIN to OFF or WITH_DEBUG to ON.")
ENDIF()

# Set DBUG_OFF for non-debug project types.
FOREACH(BUILD_TYPE RELEASE RELWITHDEBINFO MINSIZEREL)
  FOREACH(LANG C CXX)
    STRING_PREPEND(CMAKE_${LANG}_FLAGS_${BUILD_TYPE} "-DDBUG_OFF ")
  ENDFOREACH()
ENDFOREACH()

IF(NOT CMAKE_BUILD_TYPE
    AND NOT CMAKE_GENERATOR MATCHES "Visual Studio" 
    AND NOT CMAKE_GENERATOR MATCHES "Xcode") 
    # This is the case of no CMAKE_BUILD_TYPE choosen, typical for VS and Xcode
    # or if custom C flags are set. In VS and Xcode for non-Debug configurations 
    # DBUG_OFF is already correctly set.
    ADD_DEFINITIONS(-DDBUG_OFF)
ENDIF()

# Add safemutex for debug configurations
# Enable debug sync for debug builds.
FOREACH(LANG C CXX)
  STRING_PREPEND(CMAKE_${LANG}_FLAGS_DEBUG "-DENABLED_DEBUG_SYNC ")
  IF (NOT DISABLE_SAFE_MUTEX)
     STRING_PREPEND(CMAKE_${LANG}_FLAGS_DEBUG "-DSAFE_MUTEX ")
  ENDIF()
ENDFOREACH()


# Set commonly used variables
IF(WIN32)
  SET(DEFAULT_MYSQL_HOME "C:/Program Files/MySQL/MySQL Server ${MYSQL_BASE_VERSION}" )
  SET(SHAREDIR share)
ELSE()
  IF (NOT DEFAULT_MYSQL_HOME)
    SET(DEFAULT_MYSQL_HOME ${CMAKE_INSTALL_PREFIX})
  ENDIF()
  SET(SHAREDIR ${DEFAULT_MYSQL_HOME}/${INSTALL_MYSQLSHAREDIR})
ENDIF()

SET(DEFAULT_BASEDIR "${DEFAULT_MYSQL_HOME}")

IF(IS_ABSOLUTE ${INSTALL_MYSQLDATADIR})
  SET(MYSQL_DATADIR ${INSTALL_MYSQLDATADIR} CACHE PATH
    "default MySQL data directory")
ELSE()
  SET(MYSQL_DATADIR "${DEFAULT_MYSQL_HOME}/${INSTALL_MYSQLDATADIR}" CACHE PATH
    "default MySQL data directory")
ENDIF()

IF(IS_ABSOLUTE ${INSTALL_MYSQLKEYRINGDIR})
  SET(MYSQL_KEYRINGDIR ${INSTALL_MYSQLKEYRINGDIR} CACHE PATH
    "default MySQL keyring directory")
ELSE()
  SET(MYSQL_KEYRINGDIR "${DEFAULT_MYSQL_HOME}/${INSTALL_MYSQLKEYRINGDIR}"
    CACHE PATH "default MySQL keyring directory")
ENDIF()

SET(DEFAULT_CHARSET_HOME "${DEFAULT_MYSQL_HOME}")
SET(PLUGINDIR "${DEFAULT_MYSQL_HOME}/${INSTALL_PLUGINDIR}")
IF(SYSCONFDIR)
  SET(DEFAULT_SYSCONFDIR "${SYSCONFDIR}")
ENDIF()

IF(WIN32) # P_tmpdir is not defined on Windows as of VS2015.
  SET(TMPDIR "" # So we use empty path as default. In practice TMP/TEMP is used
    CACHE PATH
    "PATH to MySQL TMP dir")
ELSE()
  SET(TMPDIR "P_tmpdir"
    CACHE PATH
    "PATH to MySQL TMP dir. Defaults to the P_tmpdir macro in <stdio.h>")
ENDIF()
IF(TMPDIR STREQUAL "P_tmpdir")
  # Do not quote it, to refer to the P_tmpdir macro.
  SET(DEFAULT_TMPDIR "P_tmpdir")
ELSE()
  # Quote it, to make it a const char string.
  SET(DEFAULT_TMPDIR "\"${TMPDIR}\"")
ENDIF()

IF(NOT WITHOUT_SERVER)
  INCLUDE(cmake/boost.cmake)
  INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
ENDIF()

IF (LINUX)
  OPTION(WITH_SYSTEMD "Enable installation of systemd support files" OFF)
  IF (WITH_SYSTEMD)
    INCLUDE(cmake/systemd.cmake)
  ENDIF()
ELSE()
  IF (WITH_SYSTEMD)
    MESSAGE(FATAL_ERROR "Installation of systemd support files not supported")
  ENDIF()
ENDIF()

# Run platform tests
INCLUDE(configure.cmake)

# Common defines and includes
ADD_DEFINITIONS(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DHAVE_OPENSSL)            # Removed by Oracle in 8.0.20
ADD_DEFINITIONS(-D__STDC_LIMIT_MACROS)     # Enable C99 limit macros
ADD_DEFINITIONS(-D__STDC_FORMAT_MACROS)    # Enable C99 printf format macros
ADD_DEFINITIONS(-D_USE_MATH_DEFINES)       # Get access to M_PI, M_E, etc. in math.h
ADD_DEFINITIONS(-DLZ4_DISABLE_DEPRECATE_WARNINGS) # C++14 deprecation warnings in LZ4.

OPTION(ENABLE_EXPERIMENT_SYSVARS "Expose ussually hidden system variables to allow experiments" OFF)
IF(ENABLE_EXPERIMENT_SYSVARS)
  ADD_DEFINITIONS(-DENABLE_EXPERIMENT_SYSVARS)
ENDIF()


INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  )

IF(WITH_LTO)
  IF(APPLE)
    # Requires CMake 3.9+
    INCLUDE(CheckIPOSupported)
    # Will terminate cmake if the check fails.
    CHECK_IPO_SUPPORTED()
    # macOS has problems with enabling LTO for shared libs, so we only set it
    # as target property for mysqld later.
  ELSE()
    MY_CHECK_CXX_COMPILER_FLAG("-flto" CXX_LTO_RESULT)
    IF(NOT CXX_LTO_RESULT)
      MESSAGE(FATAL_ERROR "Compiler does not support -flto")
    ENDIF()
    STRING_APPEND(CMAKE_C_FLAGS   " -flto")
    STRING_APPEND(CMAKE_CXX_FLAGS " -flto")

    # Test for parallel linking.
    SET(NUM_PROCESSING_UNITS 4)
    MY_CHECK_CXX_COMPILER_FLAG(
      "-flto=${NUM_PROCESSING_UNITS}" CXX_LTO_PARALLEL_RESULT)
    IF(CXX_LTO_PARALLEL_RESULT)
      FIND_PROGRAM(NPROC_EXECUTABLE nproc)
      IF(NPROC_EXECUTABLE)
        EXECUTE_PROCESS(COMMAND ${NPROC_EXECUTABLE}
          OUTPUT_VARIABLE NPROC_NUM
          RESULT_VARIABLE NPROC_RESULT
          OUTPUT_STRIP_TRAILING_WHITESPACE)
        IF(NOT NPROC_RESULT)
          SET(NUM_PROCESSING_UNITS ${NPROC_NUM})
        ENDIF()
      ENDIF()
      STRING_APPEND(CMAKE_CXX_LINK_FLAGS      " -flto=${NUM_PROCESSING_UNITS}")
      STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS " -flto=${NUM_PROCESSING_UNITS}")
    ELSE()
      STRING_APPEND(CMAKE_CXX_LINK_FLAGS      " -flto")
      STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS " -flto")
    ENDIF()
  ENDIF()

  # On some platforms (Fedora) we *must* use gcc-ar / gcc-ranlib
  # to get the right plugins for supporting -flto output from gcc.
  IF(MY_COMPILER_IS_GNU)
    IF(ALTERNATIVE_ENABLE)
      GET_FILENAME_COMPONENT(ALTERNATIVE_ENABLE_DIR ${ALTERNATIVE_ENABLE} PATH)
      SET(DEVTOOLSET_ROOT "${ALTERNATIVE_ENABLE_DIR}/root")
      FIND_PROGRAM(GCC_AR_EXECUTABLE gcc-ar
        NO_DEFAULT_PATH
        PATHS "${DEVTOOLSET_ROOT}/usr/bin")
      FIND_PROGRAM(GCC_RANLIB_EXECUTABLE gcc-ranlib
        NO_DEFAULT_PATH
        PATHS "${DEVTOOLSET_ROOT}/usr/bin")
    ELSEIF(ALTERNATIVE_GCC)
      STRING(REGEX MATCH  "^([0-9])"
        ALTERNATIVE_GCC_MAJ_VER ${CMAKE_C_COMPILER_VERSION})
      GET_FILENAME_COMPONENT(ALTERNATIVE_GCC_DIR ${ALTERNATIVE_GCC} PATH)
      FIND_PROGRAM(GCC_AR_EXECUTABLE gcc-ar-${ALTERNATIVE_GCC_MAJ_VER}
        NO_DEFAULT_PATH
        PATHS "${ALTERNATIVE_GCC_DIR}")
      FIND_PROGRAM(GCC_RANLIB_EXECUTABLE gcc-ranlib-${ALTERNATIVE_GCC_MAJ_VER}
        NO_DEFAULT_PATH
        PATHS "${ALTERNATIVE_GCC_DIR}")
    ELSE()
      FIND_PROGRAM(GCC_AR_EXECUTABLE gcc-ar)
      FIND_PROGRAM(GCC_RANLIB_EXECUTABLE gcc-ranlib)
    ENDIF()
    IF(GCC_AR_EXECUTABLE AND GCC_RANLIB_EXECUTABLE)
      SET(CMAKE_AR ${GCC_AR_EXECUTABLE})
      SET(CMAKE_RANLIB ${GCC_RANLIB_EXECUTABLE})
    ENDIF()
  ENDIF()
ENDIF(WITH_LTO)

# Explicit list of all -DWITH_XXX=system possibilities
# *except* boost, which requires one specific version to be downloaded.
# RapidJSON is also excluded, since the latest released version does not
# fully support the use of std::regex as the regular expression engine. It
# is required by JSON_SCHEMA_VALID, so don't add it automatically until an
# official version with full support for std::regex is released. The bundled
# version has the necessary patches to make it work.
SET(SYSTEM_LIBRARIES
  CURL
  EDITLINE
  ICU
  LIBEVENT
  LZ4
  PROTOBUF
  SSL
  ZLIB
  ZSTD
  )

SET(WITH_SYSTEM_LIBS_DEFAULT OFF)
OPTION(WITH_SYSTEM_LIBS
  "Use -DWITH_XXX=system for ${SYSTEM_LIBRARIES}" ${WITH_SYSTEM_LIBS_DEFAULT})

GET_CMAKE_PROPERTY(CACHE_VARS CACHE_VARIABLES)
UNSET(COMMAND_LINE_VARS)
FOREACH(CACHE_VAR ${CACHE_VARS})
  GET_PROPERTY(CACHE_VAR_HELPSTRING CACHE ${CACHE_VAR} PROPERTY HELPSTRING)
  IF(CACHE_VAR_HELPSTRING STREQUAL
      "No help, variable specified on the command line."
      )
    LIST(APPEND COMMAND_LINE_VARS ${CACHE_VAR})
  ENDIF()
ENDFOREACH()

# Default to "system" for all SYSTEM_LIBRARIES
IF(WITH_SYSTEM_LIBS)
  FOREACH(SYSTEM_LIB ${SYSTEM_LIBRARIES})
    LIST(FIND COMMAND_LINE_VARS WITH_${SYSTEM_LIB} FOUNDIT)
    IF(FOUNDIT LESS 0)
      SET(WITH_${SYSTEM_LIB} "system")
      SET(WITH_${SYSTEM_LIB} "system" CACHE STRING
        "Use system rather than bundled" FORCE)
      MESSAGE(STATUS "Adding -DWITH_${SYSTEM_LIB}=${WITH_${SYSTEM_LIB}}")
    ELSE()
      MESSAGE(STATUS
        "Found WITH_${SYSTEM_LIB}=${WITH_${SYSTEM_LIB}} on command line")
    ENDIF()
  ENDFOREACH()
  # Don't redo the system analysis on subsequent runs of cmake
  UNSET(WITH_SYSTEM_LIBS)
  UNSET(WITH_SYSTEM_LIBS CACHE)
ENDIF()

# Add bundled or system zlib.
MYSQL_CHECK_ZLIB_WITH_COMPRESS()
IF(BUILD_BUNDLED_ZLIB)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/extra/zlib
    ${CMAKE_CURRENT_BINARY_DIR}/extra/zlib)
ENDIF()
# Add bundled or system zstd.
MYSQL_CHECK_ZSTD()

IF(LINUX)
  # Look up patchelf(1) here, before doing SSL/KERBEROS/SASL/LDAP config.
  FIND_PROGRAM(PATCHELF_EXECUTABLE patchelf)
  ADD_CUSTOM_TARGET(copy_linux_custom_dlls)
  SET(KNOWN_CUSTOM_LIBRARIES "" CACHE INTERNAL "" FORCE)
ENDIF()

# If all custom packages are in the same directory:
IF(WITH_CUSTOM_LIBRARIES)
  FILE(GLOB WITH_SSL      "${WITH_CUSTOM_LIBRARIES}/openssl*")
  FILE(GLOB WITH_KERBEROS "${WITH_CUSTOM_LIBRARIES}/krb5*")
  FILE(GLOB WITH_SASL     "${WITH_CUSTOM_LIBRARIES}/cyrus-sasl*")
  FILE(GLOB WITH_LDAP     "${WITH_CUSTOM_LIBRARIES}/openldap*")
  FOREACH(WITH_OPT WITH_SSL WITH_KERBEROS WITH_SASL WITH_LDAP)
    IF(IS_DIRECTORY ${${WITH_OPT}})
      SET(${WITH_OPT} ${${WITH_OPT}} CACHE INTERNAL "" FORCE)
      MESSAGE(STATUS "${WITH_OPT} ${${WITH_SSL}}")
    ELSE()
      MESSAGE(WARNING "${WITH_OPT} not found") # This should be FATAL_ERROR
    ENDIF()
  ENDFOREACH()
ENDIF()

# Add system/custom openssl.
MYSQL_CHECK_SSL()
MYSQL_CHECK_SSL_DLLS()

# Add Kerberos library
MYSQL_CHECK_KERBEROS()
MYSQL_CHECK_KERBEROS_DLLS()

# Add SASL library
MYSQL_CHECK_SASL()
MYSQL_CHECK_SASL_DLLS()

# Add LDAP library
MYSQL_CHECK_LDAP()
MYSQL_CHECK_LDAP_DLLS()

# Custom versions of LDAP / SASL / SSL.
IF(LINUX AND KNOWN_CUSTOM_LIBRARIES)
  # Custom libraries are not targets, we install them as FILES.
  # INSTALL_RPATH must must be set depending on binary type
  # EXECUTABLE MODULE_LIBRARY SHARED_LIBRARY, see ADD_INSTALL_RPATH_FOR_OPENSSL
  SET(LINUX_INSTALL_RPATH_ORIGIN 1)

  # Debug versions of plugins may be installed to <root>/lib/plugin/debug
  FOREACH(LINK_FLAG
      CMAKE_MODULE_LINKER_FLAGS_DEBUG
      CMAKE_SHARED_LINKER_FLAGS_DEBUG
      )
    STRING_APPEND(${LINK_FLAG} " -Wl,-rpath,'\$ORIGIN/../../private'")
    MESSAGE(STATUS "${LINK_FLAG} ${${LINK_FLAG}}")
  ENDFOREACH()
  IF(NOT PATCHELF_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Please install the patchelf(1) utility.")
  ENDIF()
  VERIFY_CUSTOM_LIBRARY_DEPENDENCIES()
ENDIF()

IF(WITH_AUTHENTICATION_LDAP)
  IF(LINUX AND KNOWN_CUSTOM_LIBRARIES)
    # LDAP / SASL / KERBEROS / SSL must all be "system" or "custom", not a mix.
    IF(WITH_LDAP STREQUAL "system" OR
        WITH_SASL STREQUAL "system" OR
#       WITH_KERBEROS STREQUAL "system" OR # enable this later
        WITH_SSL STREQUAL "system")
      MESSAGE(WARNING "-DWITH_AUTHENTICATION_LDAP=ON")
      MESSAGE(FATAL_ERROR "Inconsistent options for LDAP/SASL/KERBEROS/SSL")
    ENDIF()
  ELSEIF(WIN32)
    # system LDAP, but SASL needs to be explicitly added
    IF(NOT SASL_LIBRARY_DLL OR NOT SASL_SCRAM_PLUGIN)
      MESSAGE(WARNING "-DWITH_AUTHENTICATION_LDAP=ON")
      MESSAGE(FATAL_ERROR "Missing SASL and/or SCRAM libraries.")
    ENDIF()
  ELSEIF(APPLE)
    # system LDAP and SASL, but there is no scram plugin.
  ELSEIF(SOLARIS)
    # 11.3 has system LDAP and SASL, but there is no scram plugin.
  ELSE()
    # We must have "system" LDAP, KERBEROS and SASL (SSL is always required)
    SET(LDAP_WARN_GIVEN)
    SET(SASL_WARN_GIVEN)
    SET(KERBEROS_WARN_GIVEN)
    WARN_MISSING_SYSTEM_LDAP(LDAP_WARN_GIVEN)
    WARN_MISSING_SYSTEM_SASL(SASL_WARN_GIVEN)
    WARN_MISSING_SYSTEM_KERBEROS(KERBEROS_WARN_GIVEN)
    IF(LDAP_WARN_GIVEN OR SASL_WARN_GIVEN OR KERBEROS_WARN_GIVEN)
      # SUSE linux: may or may not have SCRAM, do not break the build.
      IF(LINUX_SUSE)
        MESSAGE(WARNING
          "-DWITH_AUTHENTICATION_LDAP=ON, but missing system libraries")
      ELSE()
        MESSAGE(FATAL_ERROR
          "-DWITH_AUTHENTICATION_LDAP=ON, but missing system libraries")
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

# Add system/bundled editline.
MYSQL_CHECK_EDITLINE()
# Add libevent
MYSQL_CHECK_LIBEVENT()
# Add lz4 library
MYSQL_CHECK_LZ4()
# Add icu library
MYSQL_CHECK_ICU()
# Add protoc and libprotobuf
IF(NOT WITHOUT_SERVER)
  MYSQL_CHECK_PROTOBUF()
ENDIF()
# Try and set CURL_LIBRARY
MYSQL_CHECK_CURL()
MYSQL_CHECK_CURL_DLLS()
# Add RapidJSON library.
MYSQL_CHECK_RAPIDJSON()

MACRO(MY_INCLUDE_SYSTEM_DIRECTORIES LIBRARY)
  IF(${WITH_${LIBRARY}} STREQUAL "bundled")
    SET(BEFORE_OR_AFTER "BEFORE")
  ELSE()
    SET(BEFORE_OR_AFTER "AFTER")
  ENDIF()

  IF(DEFINED ${LIBRARY}_INCLUDE_DIRS)
    INCLUDE_DIRECTORIES(${BEFORE_OR_AFTER} SYSTEM ${${LIBRARY}_INCLUDE_DIRS})
  ELSEIF(DEFINED ${LIBRARY}_INCLUDE_DIR)
    INCLUDE_DIRECTORIES(${BEFORE_OR_AFTER} SYSTEM ${${LIBRARY}_INCLUDE_DIR})
  ENDIF()
  IF("${${LIBRARY}_INCLUDE_DIR}" STREQUAL "")
    IF("${${LIBRARY}_INCLUDE_DIRS}" STREQUAL "")
      MESSAGE(WARNING "${LIBRARY} is unknown")
    ENDIF()
  ENDIF()
  IF(DEFINED ${LIBRARY}_INCLUDE_DIRS AND DEFINED ${LIBRARY}_INCLUDE_DIR)
    IF(NOT "${${LIBRARY}_INCLUDE_DIR}" STREQUAL "${${LIBRARY}_INCLUDE_DIRS}")
      MESSAGE(WARNING
        "NOT ${${LIBRARY}_INCLUDE_DIR} STREQUAL ${${LIBRARY}_INCLUDE_DIRS}")
    ENDIF()
  ENDIF()
ENDMACRO()

MACRO(MY_TARGET_INCLUDE_SYSTEM_DIRECTORIES TARGET LIBRARY)
  IF(${WITH_${LIBRARY}} STREQUAL "bundled")
    TARGET_INCLUDE_DIRECTORIES(${TARGET} SYSTEM BEFORE PRIVATE
      ${${LIBRARY}_INCLUDE_DIR})
  ELSE()
    TARGET_INCLUDE_DIRECTORIES(${TARGET} SYSTEM PRIVATE
      ${${LIBRARY}_INCLUDE_DIR})
  ENDIF()
ENDMACRO()

IF(LINUX)
  # Check for pthread_setname_np
  CHECK_C_SOURCE_COMPILES("
  #include <pthread.h>

  int main(int ac, char **av)
  {
    const char *thread_name= 0;
    int ret = pthread_setname_np(pthread_self(), thread_name);
    return ret;
  }"
  HAVE_PTHREAD_SETNAME_NP)
ENDIF()

IF(WITH_PROTOBUF STREQUAL "bundled")
  # Protobuf library is a target, installed to <root>/${INSTALL_PRIV_LIBDIR}
  # INSTALL_RPATH must be set for all binaries linking with libprotobuf.
  SET(UNIX_INSTALL_RPATH_ORIGIN_PRIV_LIBDIR 1)
  ADD_SUBDIRECTORY(extra/protobuf)
ENDIF()

#
# Setup maintainer mode options by the end. Platform checks are
# not run with the warning options as to not perturb fragile checks
# (i.e. do not make warnings into errors).
#
# Why don't these flags affect the entire build?
# Because things may already have been included with ADD_SUBDIRECTORY
#
OPTION(MYSQL_MAINTAINER_MODE
       "MySQL maintainer-specific development environment" OFF)

INCLUDE(maintainer)

IF(WITH_UNIT_TESTS)
  ENABLE_TESTING()
  INCLUDE(googletest)
ENDIF()

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(strings)
ADD_SUBDIRECTORY(vio)
ADD_SUBDIRECTORY(mysys)
ADD_SUBDIRECTORY(libmysql)
ADD_SUBDIRECTORY(libbinlogevents)
ADD_SUBDIRECTORY(libbinlogstandalone)

IF(NOT WITHOUT_SERVER)
  # Build failure on Solaris 11.3 for debug builds with NDBCLUSTER.
  # Adding basic local optimizations unbreaks it.
  # We *could* test 'uname -v' VERSION_LESS 11.4 here.
  # A clean build takes roughly the same time with or without -xO1
  # Link time of mysqld is roughly 10% faster with -xO1.
  IF(SOLARIS_SPARC AND MY_COMPILER_IS_SUNPRO AND
      CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
    MESSAGE(STATUS
      "Adding -xO1 for debug builds on Solaris ${MY_KERNEL_VERSION}")
    STRING_APPEND(CMAKE_CXX_FLAGS_DEBUG " -xO1")
  ENDIF()

  # Add storage engines and plugins.
  SET (MYSQLD_STATIC_PLUGIN_LIBS "" CACHE INTERNAL "")
  CONFIGURE_PLUGINS()
ELSE()
  IF(EXISTS "${CMAKE_SOURCE_DIR}/storage/ndb")
    # We may still want Cluster client libraries, use -DWITH_NDBCLUSTER=1
    ADD_SUBDIRECTORY(storage/ndb)
  ENDIF()
ENDIF()

IF(NOT WITHOUT_SERVER)
  CONFIGURE_COMPONENTS()
ENDIF()

IF(WITH_UNIT_TESTS)
  ADD_SUBDIRECTORY(unittest)
  ADD_SUBDIRECTORY(unittest/examples)
  ADD_SUBDIRECTORY(unittest/mytap)
  ADD_SUBDIRECTORY(unittest/mytap/t)
ENDIF()

ADD_SUBDIRECTORY(utilities)
ADD_SUBDIRECTORY(share)
ADD_SUBDIRECTORY(libservices)

IF(NOT WITHOUT_SERVER)
  ADD_SUBDIRECTORY(client)
  ADD_SUBDIRECTORY(testclients)
  ADD_SUBDIRECTORY(sql)
ENDIF()

# Some unit tests need to link with the entire server, use this library:
IF(NOT WITHOUT_SERVER AND WITH_UNIT_TESTS)
  SET(DUMMY_SOURCE_FILE ${CMAKE_BINARY_DIR}/server_unittest_library.c)
  ADD_CUSTOM_COMMAND(
    OUTPUT  ${DUMMY_SOURCE_FILE}
    COMMAND ${CMAKE_COMMAND} -E touch ${DUMMY_SOURCE_FILE}
    DEPENDS sql_main
    )
  ADD_LIBRARY(server_unittest_library STATIC ${DUMMY_SOURCE_FILE})
  TARGET_LINK_LIBRARIES(server_unittest_library perfschema)
  TARGET_LINK_LIBRARIES(server_unittest_library sql_main)
  TARGET_LINK_LIBRARIES(server_unittest_library minchassis)
  TARGET_LINK_LIBRARIES(server_unittest_library ${ICU_LIBRARIES})
ENDIF()

# scripts/mysql_config depends on client and server targets loaded above.
# It is referenced by some of the directories below, so we insert it here.
ADD_SUBDIRECTORY(scripts)

IF(NOT WITHOUT_SERVER)
  ADD_SUBDIRECTORY(mysql-test)
  ADD_SUBDIRECTORY(mysql-test/lib/My/SafeProcess)
  ADD_SUBDIRECTORY(support-files)
  IF(WITH_INTERNAL)
    ADD_SUBDIRECTORY(internal)
  ENDIF()
ENDIF()

INCLUDE(cmake/abi_check.cmake)
INCLUDE(cmake/tags.cmake)

CONFIGURE_FILE(config.h.cmake   ${CMAKE_BINARY_DIR}/include/my_config.h)
CONFIGURE_FILE(config.h.cmake   ${CMAKE_BINARY_DIR}/include/config.h)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/include/mysql_version.h.in
               ${CMAKE_BINARY_DIR}/include/mysql_version.h )
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/sql/sql_builtin.cc.in
    ${CMAKE_BINARY_DIR}/sql/sql_builtin.cc)

# depends on mysql_version.h to exist
IF(NOT WITHOUT_SERVER)
  IF(WITH_ROUTER)
    ADD_SUBDIRECTORY(router)
  ENDIF()
ENDIF()

IF(ENABLE_GCOV)
  INCLUDE(fastcov)
ENDIF()

IF(UNIX)
  ADD_SUBDIRECTORY(man)
ENDIF()

IF(NOT WITHOUT_SERVER)
  ADD_SUBDIRECTORY(packaging/rpm-common)
  ADD_SUBDIRECTORY(packaging/rpm-oel)
  ADD_SUBDIRECTORY(packaging/rpm-fedora)
  ADD_SUBDIRECTORY(packaging/rpm-sles)
  ADD_SUBDIRECTORY(packaging/rpm-docker)
  ADD_SUBDIRECTORY(packaging/deb-in)
ENDIF()

GET_PROPERTY(CWD_DEFINITIONS DIRECTORY PROPERTY COMPILE_DEFINITIONS)
CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/cmake/info_macros.cmake.in
    ${CMAKE_BINARY_DIR}/info_macros.cmake @ONLY)

SET(KNOWN_CONVENIENCE_LIBRARIES "" CACHE INTERNAL "")
# Add to the 'make clean' target.
# With this you need to do 'cmake .' before doing 'make' again.
SET_PROPERTY(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
  "${CMAKE_BINARY_DIR}/archive_output_directory"
  "${CMAKE_BINARY_DIR}/library_output_directory"
  )

# Handle the "INFO_*" files.
INCLUDE(${CMAKE_BINARY_DIR}/info_macros.cmake)
# Source: This can be done during the cmake phase, all information is
# available, but should be repeated on each "make" just in case someone
# does "cmake ; make ; git pull ; make".
CREATE_INFO_SRC(${CMAKE_BINARY_DIR}/Docs)
ADD_CUSTOM_TARGET(INFO_SRC ALL
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/info_src.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
# Build flags: This must be postponed to the make phase.
ADD_CUSTOM_TARGET(INFO_BIN ALL
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/info_bin.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Packaging
IF(WIN32)
  OPTION(BUNDLE_RUNTIME_LIBRARIES "INSTALL runtime libraries" OFF)
  IF(BUNDLE_RUNTIME_LIBRARIES)
    SET(RUNTIME_LIBRARIES
      msvcp140
      ucrtbase
      vcruntime140
      )
    FOREACH(LIB ${RUNTIME_LIBRARIES})
      SET(LIB_PATH "C:/Windows/System32/${LIB}.dll")
      SET(LIB_D_PATH "C:/Windows/System32/${LIB}d.dll")
      FOREACH(LIBRARY_PATH ${LIB_PATH} ${LIB_D_PATH})
        IF(EXISTS "${LIBRARY_PATH}")
          MESSAGE(STATUS "INSTALL ${LIBRARY_PATH} to ${INSTALL_BINDIR}")
          INSTALL(FILES
            "${LIBRARY_PATH}" DESTINATION "${INSTALL_BINDIR}"
            COMPONENT SharedLibraries)
        ELSE()
          MESSAGE(STATUS "Could not find ${LIBRARY_PATH}")
        ENDIF()
      ENDFOREACH()
    ENDFOREACH()
  ENDIF()
ENDIF()

IF(WIN32)
  SET(CPACK_GENERATOR "ZIP")
ELSE()
  SET(CPACK_GENERATOR "TGZ")
ENDIF() 
ADD_SUBDIRECTORY(packaging/WiX)
ADD_SUBDIRECTORY(packaging/solaris)

# Produce separate tarballs for each INSTALL COMPONENT
IF(PACK_SEPARATE_COMPONENTS)
  SET(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
  SET(CPACK_COMPONENTS_GROUPING IGNORE)
ENDIF()

IF(UNIX)
  INSTALL(FILES
    Docs/mysql.info
    DESTINATION ${INSTALL_INFODIR} OPTIONAL COMPONENT Info)
ENDIF()
#
# RPM installs documentation directly from the source tree
#
IF(NOT INSTALL_LAYOUT MATCHES "RPM")
  INSTALL(FILES
    README
    LICENSE
    DESTINATION ${INSTALL_DOCREADMEDIR} COMPONENT Readme)
  INSTALL(FILES
    ${CMAKE_BINARY_DIR}/Docs/INFO_SRC
    ${CMAKE_BINARY_DIR}/Docs/INFO_BIN
    DESTINATION ${INSTALL_DOCDIR} COMPONENT Info
    )

  # MYSQL_DOCS_LOCATON is used in "make dist",
  # points to the documentation directory
  SET(MYSQL_DOCS_LOCATION "" CACHE PATH
    "Location from where documentation is copied")
  MARK_AS_ADVANCED(MYSQL_DOCS_LOCATION)
  INSTALL(DIRECTORY Docs/ DESTINATION ${INSTALL_DOCDIR}
    COMPONENT Documentation
    PATTERN "Makefile.*" EXCLUDE
    PATTERN "glibc*" EXCLUDE
    PATTERN "linuxthreads.txt" EXCLUDE
    PATTERN "myisam.txt" EXCLUDE
    PATTERN "mysql.info" EXCLUDE
    PATTERN "sp-imp-spec.txt" EXCLUDE
    PATTERN "README.build" EXCLUDE
  )
ENDIF()

# Now that we're done with all ADD_SUBDIRECTORY and thus all feature tests,
# we can safely enable ASan on Clang/Win32 if needed.
IF(HAVE_ASAN AND WIN32_CLANG)
  STRING_APPEND(CMAKE_C_FLAGS
    " -fsanitize=address -fsanitize-address-use-after-scope")
  STRING_APPEND(CMAKE_CXX_FLAGS
    " -fsanitize=address -fsanitize-address-use-after-scope")
ENDIF()

IF(UNIX)
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E make_directory library_output_directory
    COMMAND ${CMAKE_COMMAND} -E make_directory plugin_output_directory
    COMMAND ${CMAKE_COMMAND} -E make_directory runtime_output_directory
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  IF(SASL_CUSTOM_LIBRARY)
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E make_directory library_output_directory/sasl2
    )
  ENDIF()
ENDIF()

# Pre-create directories for library_output_directory and
# plugin_output_directory, so create_symlink does not fail in ssl.cmake
# Do not pre-create runtime_output_directory/Debug or
# runtime_output_directory/RelWithDebInfo, mtr gets confused, and must be
# run with --vs-config=Debug or --vs-config=RelWithDebInfo in order to work.
IF(APPLE AND NOT BUILD_IS_SINGLE_CONFIG)
  FOREACH(dir
      library_output_directory
      plugin_output_directory
      )
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_COMMAND} -E make_directory ${dir}/Debug
      COMMAND ${CMAKE_COMMAND} -E make_directory ${dir}/RelWithDebInfo
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )
  ENDFOREACH()
ENDIF()

IF(UNIX)
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
     ./runtime_output_directory bin
    COMMAND ${CMAKE_COMMAND} -E create_symlink
     ./library_output_directory lib
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
ENDIF()

SET(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
  CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)
  ADD_CUSTOM_TARGET(
    doxygen
    COMMAND ${CMAKE_COMMAND}
    -DDOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE}
    -DDOXYFILE=${CMAKE_BINARY_DIR}/Doxyfile
    -DERROR_FILE=${CMAKE_BINARY_DIR}/doxyerror.log
    -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/doxyoutput.log
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DTOFIX_FILE=${CMAKE_BINARY_DIR}/tofix-all.log
    -DREGRESSION_FILE=${CMAKE_BINARY_DIR}/tofix-regressions.log
    -DIGNORE_FILE=${CMAKE_SOURCE_DIR}/Doxyfile-ignored
    -P ${CMAKE_SOURCE_DIR}/run_doxygen.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating Doxygen documentation" VERBATIM
    )
  SET_PROPERTY(TARGET doxygen PROPERTY EXCLUDE_FROM_ALL TRUE)
  IF(WIN32)
    SET_PROPERTY(TARGET doxygen PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
  ENDIF()
ENDIF(DOXYGEN_FOUND)

INCLUDE(CPack)

# C/CXX compiler flags consist of:
# CPPFLAGS        Taken from environment, see above.
# ADD_DEFINITIONS In each individual CMakeLists.txt
# CMAKE_CXX_FLAGS From command line.
#                 We extend these in maintainer.cmake
#                 We extend these in compiler_options.cmake
# ENV{CXXFLAGS}   From environment, but environment is ignored if
#                 CMAKE_CXX_FLAGS is also given on command line
# CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}, typical initial cmake values:
#      DEBUG          -g
#      RELWITHDEBINFO -O2 -g -DNDEBUG
#      RELEASE        -O3 -DNDEBUG
#      MINSIZEREL     -Os -DNDEBUG
#
# The compiler command line is
# g++ DEFINITIONS CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_<configuration>
#
# So, if you want to use '-O3' for a RELWITHDEBINFO build, you should do:
# cmake -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -g -DNDEBUG"
#
# Note that CMakeCache.txt contains cmake builtins for these variables,
# *not* the values that will actually be used, which we report here:

IF(BUILD_IS_SINGLE_CONFIG)
  MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
ENDIF()
GET_PROPERTY(cwd_definitions DIRECTORY PROPERTY COMPILE_DEFINITIONS)
MESSAGE(STATUS "COMPILE_DEFINITIONS: ${cwd_definitions}")
REPORT_CXX_FLAGS()
MESSAGE(STATUS "CMAKE_C_LINK_FLAGS: ${CMAKE_C_LINK_FLAGS}")
MESSAGE(STATUS "CMAKE_CXX_LINK_FLAGS: ${CMAKE_CXX_LINK_FLAGS}")
MESSAGE(STATUS "CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")
MESSAGE(STATUS "CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS}")
MESSAGE(STATUS "CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS}")
