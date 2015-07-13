# Locate Intel Threading Building Blocks include paths and libraries
# TBB can be found at http://www.threadingbuildingblocks.org/
# Written by Hannes Hofmann, hannes.hofmann _at_ informatik.uni-erlangen.de
# Adapted by Gino van den Bergen gino _at_ dtecta.com

# GvdB: This module uses the environment variable TBB_ARCH_PLATFORM which defines architecture and compiler.
#   e.g. "ia32/vc8" or "em64t/cc4.1.0_libc2.4_kernel2.6.16.21"
#   TBB_ARCH_PLATFORM is set by the build script tbbvars[.bat|.sh|.csh], which can be found
#   in the TBB installation directory (TBB_INSTALL_DIR).
#
# For backwards compatibility, you may explicitly set the CMake variables TBB_ARCHITECTURE and TBB_COMPILER.
# TBB_ARCHITECTURE     [ ia32 | em64t | itanium ]
#   which architecture to use
# TBB_COMPILER         e.g. vc9 or cc3.2.3_libc2.3.2_kernel2.4.21 or cc4.0.1_os10.4.9
#   which compiler to use (detected automatically on Windows)

# This module respects
# TBB_INSTALL_DIR or $ENV{TBB21_INSTALL_DIR} or $ENV{TBB_INSTALL_DIR}

# This module defines
# TBB_INCLUDE_DIRS, where to find task_scheduler_init.h, etc.
# TBB_LIBRARY_DIRS, where to find libtbb, libtbbmalloc
# TBB_INSTALL_DIR, the base TBB install directory
# TBB_LIBRARIES, the libraries to link against to use TBB.
# TBB_DEBUG_LIBRARIES, the libraries to link against to use TBB with debug symbols.
# TBB_FOUND, If false, don't try to use TBB.

# lets try in the obvious places first
find_path(TEST_TBB_INCLUDE_DIR
    tbb/task_scheduler_init.h
)
find_library(TEST_TBB_LIBRARY tbb)

if (TEST_TBB_INCLUDE_DIR)
  if(TEST_TBB_LIBRARY)
    set(TBB_OBVIOUS_PLACE "YES")
  endif()
endif()


if (WIN32)
  # has em64t/vc8   em64t/vc9
  # has ia32/vc7.1  ia32/vc8   ia32/vc9
  set(_TBB_DEFAULT_INSTALL_DIR "C:/Program Files/Intel/TBB" "C:/Program Files (x86)/Intel/TBB")
  set(_TBB_LIB_NAME "tbb")
  set(_TBB_LIB_MALLOC_NAME "${_TBB_LIB_NAME}malloc")
  set(_TBB_LIB_DEBUG_NAME "${_TBB_LIB_NAME}_debug")
  set(_TBB_LIB_MALLOC_DEBUG_NAME "${_TBB_LIB_MALLOC_NAME}_debug")
  if (MSVC71)
    set (_TBB_COMPILER "vc7.1")
  endif(MSVC71)
  if (MSVC80)
    set(_TBB_COMPILER "vc8")
  endif(MSVC80)
  if (MSVC90)
    set(_TBB_COMPILER "vc9")
  endif(MSVC90)
  if (MSVC10)
    set(_TBB_COMPILER "vc10")
  endif(MSVC10)
  if (MSVC11)
    set(_TBB_COMPILER "vc11")
  endif(MSVC11)
  if (MSVC12)
    set(_TBB_COMPILER "vc12")
  endif(MSVC12)
  if (NOT _TBB_COMPILER)
    message("ERROR: TBB supports only VC 7.1, 8, 9, 10, 11, and 12 compilers on Windows platforms.")
  endif (NOT _TBB_COMPILER)
  set(_TBB_ARCHITECTURE ${TBB_ARCHITECTURE})
endif (WIN32)

if (UNIX)
  if (APPLE)
    # MAC
    set(_TBB_DEFAULT_INSTALL_DIR "/Library/Frameworks/Intel_TBB.framework/Versions")
    # libs: libtbb.dylib, libtbbmalloc.dylib, *_debug
    set(_TBB_LIB_NAME "tbb")
    set(_TBB_LIB_MALLOC_NAME "${_TBB_LIB_NAME}malloc")
    set(_TBB_LIB_DEBUG_NAME "${_TBB_LIB_NAME}_debug")
    set(_TBB_LIB_MALLOC_DEBUG_NAME "${_TBB_LIB_MALLOC_NAME}_debug")
    # has only one flavor: ia32/cc4.0.1_os10.4.9
    set(_TBB_COMPILER "cc4.0.1_os10.4.9")
    set(_TBB_ARCHITECTURE "ia32")
  else (APPLE)
    # LINUX
    set(_TBB_DEFAULT_INSTALL_DIR "/opt/intel/tbb" "/usr/local/include" "/usr/include")
    set(_TBB_LIB_NAME "tbb")
    set(_TBB_LIB_MALLOC_NAME "${_TBB_LIB_NAME}malloc")
    set(_TBB_LIB_DEBUG_NAME "${_TBB_LIB_NAME}_debug")
    set(_TBB_LIB_MALLOC_DEBUG_NAME "${_TBB_LIB_MALLOC_NAME}_debug")
    # has em64t/cc3.2.3_libc2.3.2_kernel2.4.21  em64t/cc3.3.3_libc2.3.3_kernel2.6.5  em64t/cc3.4.3_libc2.3.4_kernel2.6.9  em64t/cc4.1.0_libc2.4_kernel2.6.16.21
    # has ia32/*
    # has itanium/*
    set(_TBB_COMPILER ${TBB_COMPILER})
    set(_TBB_ARCHITECTURE ${TBB_ARCHITECTURE})
  endif (APPLE)
endif (UNIX)

if (CMAKE_SYSTEM MATCHES "SunOS.*")
# SUN
# not yet supported
# has em64t/cc3.4.3_kernel5.10
# has ia32/*
endif (CMAKE_SYSTEM MATCHES "SunOS.*")


#-- Clear the public variables
set (TBB_FOUND "NO")


#-- Find TBB install dir and set ${_TBB_INSTALL_DIR} and cached ${TBB_INSTALL_DIR}
# first: use CMake variable TBB_INSTALL_DIR
if (TBB_INSTALL_DIR)
    set (_TBB_INSTALL_DIR ${TBB_INSTALL_DIR})
endif (TBB_INSTALL_DIR)
# second: use environment variable
if (NOT _TBB_INSTALL_DIR)
  if (NOT "$ENV{TBB_INSTALL_DIR}" STREQUAL "")
    set (_TBB_INSTALL_DIR $ENV{TBB_INSTALL_DIR})
  endif (NOT "$ENV{TBB_INSTALL_DIR}" STREQUAL "")
  # Intel recommends setting TBB21_INSTALL_DIR
  if (NOT "$ENV{TBB21_INSTALL_DIR}" STREQUAL "")
    set (_TBB_INSTALL_DIR $ENV{TBB21_INSTALL_DIR})
  endif (NOT "$ENV{TBB21_INSTALL_DIR}" STREQUAL "")
endif (NOT _TBB_INSTALL_DIR)
# third: try to find path automatically
if (NOT _TBB_INSTALL_DIR)
  if (_TBB_DEFAULT_INSTALL_DIR)
    set (_TBB_INSTALL_DIR $ENV{_TBB_DEFAULT_INSTALL_DIR})
  endif (_TBB_DEFAULT_INSTALL_DIR)
endif (NOT _TBB_INSTALL_DIR)
# sanity check

if (TBB_OBVIOUS_PLACE)
  set (TBB_FOUND "YES")
  set (TBB_LIBRARIES ${TEST_TBB_LIBRARY} ${TBB_LIBRARIES})
  set (TBB_INCLUDE_DIRS ${TEST_TBB_INCLUDE_DIR} CACHE PATH "TBB include directory" FORCE)
  mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARIES)
  message(STATUS "Found Intel TBB")
elseif (NOT _TBB_INSTALL_DIR)
  message ("ERROR: Unable to find Intel TBB install directory. ${_TBB_INSTALL_DIR}")
else (TBB_OBVIOUS_PLACE)
  # finally: set the cached CMake variable TBB_INSTALL_DIR
  if (NOT TBB_INSTALL_DIR)
    set (TBB_INSTALL_DIR ${_TBB_INSTALL_DIR} CACHE PATH "Intel TBB install directory")
    mark_as_advanced(TBB_INSTALL_DIR)
  endif (NOT TBB_INSTALL_DIR)


  #-- Look for include directory and set ${TBB_INCLUDE_DIR}
  set (TBB_INC_SEARCH_DIR ${_TBB_INSTALL_DIR}/include)
  find_path(TBB_INCLUDE_DIR
    tbb/task_scheduler_init.h
    PATHS ${TBB_INC_SEARCH_DIR}
  )
  mark_as_advanced(TBB_INCLUDE_DIR)


  #-- Look for libraries
  # GvdB: $ENV{TBB_ARCH_PLATFORM} is set by the build script tbbvars[.bat|.sh|.csh]
  if (NOT $ENV{TBB_ARCH_PLATFORM} STREQUAL "")
    set (TBB_LIBRARY_DIR "${_TBB_INSTALL_DIR}/$ENV{TBB_ARCH_PLATFORM}/lib")
  else (NOT $ENV{TBB_ARCH_PLATFORM} STREQUAL "")
    # Undeprecated to allow for Windows use. 
    set (TBB_LIBRARY_DIR "${_TBB_INSTALL_DIR}/lib/${_TBB_ARCHITECTURE}/${_TBB_COMPILER}")
  endif (NOT $ENV{TBB_ARCH_PLATFORM} STREQUAL "")

  find_library(TBB_LIBRARY        ${_TBB_LIB_NAME}        ${TBB_LIBRARY_DIR} NO_DEFAULT_PATH)
  find_library(TBB_MALLOC_LIBRARY ${_TBB_LIB_MALLOC_NAME} ${TBB_LIBRARY_DIR} NO_DEFAULT_PATH)
  mark_as_advanced(TBB_LIBRARY TBB_MALLOC_LIBRARY)

  #-- Look for debug libraries
  find_library(TBB_LIBRARY_DEBUG        ${_TBB_LIB_DEBUG_NAME}        ${TBB_LIBRARY_DIR} NO_DEFAULT_PATH)
  find_library(TBB_MALLOC_LIBRARY_DEBUG ${_TBB_LIB_MALLOC_DEBUG_NAME} ${TBB_LIBRARY_DIR} NO_DEFAULT_PATH)
  mark_as_advanced(TBB_LIBRARY_DEBUG TBB_MALLOC_LIBRARY_DEBUG)


  if (TBB_INCLUDE_DIR)
    if (TBB_LIBRARY)
      set (TBB_FOUND "YES")
      if(CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        set(TBB_LIBRARIES optimized ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY} ${TBB_LIBRARIES} debug ${TBB_LIBRARY_DEBUG} ${TBB_MALLOC_LIBRARY_DEBUG} ${TBB_DEBUG_LIBRARIES})
      else()
        set (TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY} ${TBB_LIBRARIES})
        set (TBB_DEBUG_LIBRARIES ${TBB_LIBRARY_DEBUG} ${TBB_MALLOC_LIBRARY_DEBUG} ${TBB_DEBUG_LIBRARIES})
        mark_as_advanced(TBB_DEBUG_LIBRARIES)
      endif()
      set (TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR} CACHE PATH "TBB include directory" FORCE)
      set (TBB_LIBRARY_DIRS ${TBB_LIBRARY_DIR} CACHE PATH "TBB library directory" FORCE)
      mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARY_DIRS TBB_LIBRARIES)
      message(STATUS "Found Intel TBB")
    endif (TBB_LIBRARY)
  endif (TBB_INCLUDE_DIR)

  if (NOT TBB_FOUND)
    message("ERROR: Intel TBB NOT found!")
    message(STATUS "Looked for Threading Building Blocks in ${_TBB_INSTALL_DIR}")
    # do only throw fatal, if this pkg is REQUIRED
    if (TBB_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find TBB library.")
    endif (TBB_FIND_REQUIRED)
  endif (NOT TBB_FOUND)

endif (TBB_OBVIOUS_PLACE)

if (TBB_FOUND)
  set(TBB_INTERFACE_VERSION 0)

  FILE(READ "${TBB_INCLUDE_DIRS}/tbb/tbb_stddef.h" _TBB_VERSION_CONTENTS)
  STRING(REGEX REPLACE ".*#define TBB_INTERFACE_VERSION ([0-9]+).*" "\\1" TBB_INTERFACE_VERSION "${_TBB_VERSION_CONTENTS}")

  set(TBB_INTERFACE_VERSION "${TBB_INTERFACE_VERSION}")
endif()
