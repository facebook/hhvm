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

INCLUDE(CheckSymbolExists)
INCLUDE(CheckCSourceRuns)
INCLUDE(CheckCSourceCompiles) 
INCLUDE(CheckCXXSourceCompiles)

SET(SOLARIS 1)
IF(CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
  SET(SOLARIS_SPARC 1)
ELSE()
  SET(SOLARIS_INTEL 1)
ENDIF()

IF (NOT "${CMAKE_C_FLAGS}${CMAKE_CXX_FLAGS}" MATCHES "-m32|-m64")
  IF(NOT FORCE_UNSUPPORTED_COMPILER)
    MESSAGE("Adding -m64")
    STRING_APPEND(CMAKE_C_FLAGS        " -m64")
    STRING_APPEND(CMAKE_CXX_FLAGS      " -m64")
    STRING_APPEND(CMAKE_C_LINK_FLAGS   " -m64")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS " -m64")
  ENDIF()
ENDIF()

INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE("void *" SIZEOF_VOIDP)

# We require at least SunStudio 12.6 (CC 5.15)
IF(NOT FORCE_UNSUPPORTED_COMPILER)
  IF(MY_COMPILER_IS_SUNPRO)
    IF(SIZEOF_VOIDP MATCHES 4)
      MESSAGE(FATAL_ERROR "32 bit Solaris builds are not supported. ")
    ENDIF()
    # CC -V yields
    # CC: Studio 12.6 Sun C++ 5.15 SunOS_sparc Beta 2016/12/19
    # CC: Studio 12.5 Sun C++ 5.14 SunOS_sparc Dodona 2016/04/04
    # CC: Sun C++ 5.13 SunOS_sparc Beta 2014/03/11
    # CC: Sun C++ 5.11 SunOS_sparc 2010/08/13
    EXECUTE_PROCESS(
      COMMAND ${CMAKE_CXX_COMPILER} "-V"
      OUTPUT_VARIABLE stdout
      ERROR_VARIABLE  stderr
      RESULT_VARIABLE result
    )
    STRING(REGEX MATCH "CC: Sun C\\+\\+ 5\\.([0-9]+)" VERSION_STRING ${stderr})
    IF (NOT CMAKE_MATCH_1 OR CMAKE_MATCH_1 STREQUAL "")
      STRING(REGEX MATCH "CC: Studio 12\\.[56] Sun C\\+\\+ 5\\.([0-9]+)"
        VERSION_STRING ${stderr})
    ENDIF()
    SET(CC_MINOR_VERSION ${CMAKE_MATCH_1})
    IF(${CC_MINOR_VERSION} LESS 15)
      MESSAGE(FATAL_ERROR "Oracle Studio 12.6 or newer is required!")
    ENDIF()
  ELSEIF(MY_COMPILER_IS_CLANG)
    MESSAGE(WARNING "Clang is (highly) experimental!!")
  ELSEIF(MY_COMPILER_IS_GNU)
    MESSAGE(WARNING "gcc is experimental")
  ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler!")
  ENDIF()
ENDIF()

# Enable 64 bit file offsets
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)

# Enable general POSIX extensions. See standards(5) man page.
ADD_DEFINITIONS(-D__EXTENSIONS__)

# Solaris threads with POSIX semantics:
# http://docs.oracle.com/cd/E19455-01/806-5257/6je9h033k/index.html
ADD_DEFINITIONS(-D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT -D_PTHREADS)

# On  Solaris, use of intrinsics will screw the lib search logic
# Force using -lm, so rint etc are found.
SET(LIBM m)

# CMake defined -lthread as thread flag. This crashes in dlopen 
# when trying to load plugins workaround with -lpthread
SET(CMAKE_THREAD_LIBS_INIT -lpthread CACHE INTERNAL "" FORCE)

# Solaris specific large page support
CHECK_SYMBOL_EXISTS(MHA_MAPSIZE_VA sys/mman.h  HAVE_SOLARIS_LARGE_PAGES)

# Solaris atomics
CHECK_C_SOURCE_RUNS(
 "
 #include  <atomic.h>
  int main()
  {
    int foo = -10; int bar = 10;
    int64_t foo64 = -10; int64_t bar64 = 10;
    if (atomic_add_int_nv((uint_t *)&foo, bar) || foo)
      return -1;
    bar = atomic_swap_uint((uint_t *)&foo, (uint_t)bar);
    if (bar || foo != 10)
     return -1;
    bar = atomic_cas_uint((uint_t *)&bar, (uint_t)foo, 15);
    if (bar)
      return -1;
    if (atomic_add_64_nv((volatile uint64_t *)&foo64, bar64) || foo64)
      return -1;
    bar64 = atomic_swap_64((volatile uint64_t *)&foo64, (uint64_t)bar64);
    if (bar64 || foo64 != 10)
      return -1;
    bar64 = atomic_cas_64((volatile uint64_t *)&bar64, (uint_t)foo64, 15);
    if (bar64)
      return -1;
    atomic_or_64((volatile uint64_t *)&bar64, 0);
    return 0;
  }
"  HAVE_SOLARIS_ATOMIC)

# This is used for the version_compile_machine variable.
IF(SIZEOF_VOIDP MATCHES 8 AND SOLARIS_INTEL)
  SET(MYSQL_MACHINE_TYPE "x86_64")
ENDIF()


MACRO(DIRNAME IN OUT)
  GET_FILENAME_COMPONENT(${OUT} ${IN} PATH)
ENDMACRO()

# We assume that developer studio runtime libraries are installed.
IF(MY_COMPILER_IS_SUNPRO)
  DIRNAME(${CMAKE_CXX_COMPILER} CXX_PATH)

  SET(LIBRARY_SUFFIX "lib/compilers/CC-gcc/lib")
  IF(SIZEOF_VOIDP EQUAL 8 AND SOLARIS_SPARC)
    SET(LIBRARY_SUFFIX "${LIBRARY_SUFFIX}/sparcv9")
  ENDIF()
  IF(SIZEOF_VOIDP EQUAL 8 AND SOLARIS_INTEL)
    SET(LIBRARY_SUFFIX "${LIBRARY_SUFFIX}/amd64")
  ENDIF()
  FIND_LIBRARY(STL_LIBRARY_NAME
    NAMES "stdc++"
    PATHS ${CXX_PATH}/../${LIBRARY_SUFFIX}
    NO_DEFAULT_PATH
  )
  MESSAGE(STATUS "STL_LIBRARY_NAME ${STL_LIBRARY_NAME}")
  IF(STL_LIBRARY_NAME)
    DIRNAME(${STL_LIBRARY_NAME} STL_LIBRARY_PATH)
    SET(LRFLAGS " -L${STL_LIBRARY_PATH} -R${STL_LIBRARY_PATH}")
    SET(QUOTED_CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}")

    STRING_APPEND(CMAKE_C_LINK_FLAGS          "${LRFLAGS}")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS        "${LRFLAGS}")
    STRING_APPEND(CMAKE_MODULE_LINKER_FLAGS   "${LRFLAGS}")
    STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS   "${LRFLAGS}")
    STRING_APPEND(QUOTED_CMAKE_CXX_LINK_FLAGS "${LRFLAGS}")
  ENDIF()

  STRING_APPEND(CMAKE_C_LINK_FLAGS          " -lc")
  STRING_APPEND(CMAKE_CXX_LINK_FLAGS        " -lstdc++ -lgcc_s -lCrunG3 -lc")
  STRING_APPEND(CMAKE_MODULE_LINKER_FLAGS   " -lstdc++ -lgcc_s -lCrunG3 -lc")
  STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS   " -lstdc++ -lgcc_s -lCrunG3 -lc")
  STRING_APPEND(QUOTED_CMAKE_CXX_LINK_FLAGS " -lstdc++ -lgcc_s -lCrunG3 -lc")

  SET(LIBRARY_SUFFIX "lib/compilers/atomic")
  IF(SIZEOF_VOIDP EQUAL 8 AND SOLARIS_SPARC)
    SET(LIBRARY_SUFFIX "${LIBRARY_SUFFIX}/sparcv9")
  ENDIF()
  IF(SIZEOF_VOIDP EQUAL 8 AND SOLARIS_INTEL)
    SET(LIBRARY_SUFFIX "${LIBRARY_SUFFIX}/amd64")
  ENDIF()
  FIND_LIBRARY(ATOMIC_LIBRARY_NAME
    NAMES "statomic"
    PATHS ${CXX_PATH}/../${LIBRARY_SUFFIX}
    NO_DEFAULT_PATH
  )
  MESSAGE(STATUS "ATOMIC_LIBRARY_NAME ${ATOMIC_LIBRARY_NAME}")
  IF(ATOMIC_LIBRARY_NAME)
    DIRNAME(${ATOMIC_LIBRARY_NAME} ATOMIC_LIB_PATH)
    SET(LRFLAGS " -L${ATOMIC_LIB_PATH} -R${ATOMIC_LIB_PATH}")

    STRING_APPEND(CMAKE_C_LINK_FLAGS          "${LRFLAGS}")
    STRING_APPEND(CMAKE_CXX_LINK_FLAGS        "${LRFLAGS}")
    STRING_APPEND(CMAKE_MODULE_LINKER_FLAGS   "${LRFLAGS}")
    STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS   "${LRFLAGS}")
    STRING_APPEND(QUOTED_CMAKE_CXX_LINK_FLAGS "${LRFLAGS}")
  ENDIF()

  SET(QUOTED_CMAKE_CXX_LINK_FLAGS
    "${QUOTED_CMAKE_CXX_LINK_FLAGS} -lstatomic ")
ENDIF()

# Experimental support for clang on Intel Solaris.
# Try to build and run static_thread_local_test.
IF(MY_COMPILER_IS_CLANG AND SOLARIS_INTEL)
  SET(CLANG_OUTPUT_FILE ${CMAKE_BINARY_DIR}/clang-output)
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_CXX_COMPILER} --print-search-dirs
    OUTPUT_FILE ${CLANG_OUTPUT_FILE}
    )
  FILE(READ ${CLANG_OUTPUT_FILE} OUTPUT_FILE_CONTENTS)
  STRING(REPLACE "\n" ";" OUTPUT_FILE_CONTENTS ${OUTPUT_FILE_CONTENTS})
  FOREACH(LINE ${OUTPUT_FILE_CONTENTS})
    STRING(REGEX MATCH "(libraries: =)(.*)" XXX ${LINE})
    IF(CMAKE_MATCH_1)
      SET(LIBRARIES_STRING "${CMAKE_MATCH_2}")
    ENDIF()
  ENDFOREACH()
  STRING(REPLACE ":" ";" LIBRARIES_LIST "${LIBRARIES_STRING}")

  SET(LRFLAGS "")
  FOREACH(LIB_PATH ${LIBRARIES_LIST})
    GET_FILENAME_COMPONENT(REAL_PATH ${LIB_PATH} REALPATH)
    STRING_APPEND(LRFLAGS " -Wl,-L${REAL_PATH} -Wl,-R${REAL_PATH}")
  ENDFOREACH()

  STRING_APPEND(CMAKE_C_LINK_FLAGS          "${LRFLAGS}")
  STRING_APPEND(CMAKE_CXX_LINK_FLAGS        "${LRFLAGS}")
  STRING_APPEND(CMAKE_MODULE_LINKER_FLAGS   "${LRFLAGS}")
  STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS   "${LRFLAGS}")

  STRING_APPEND(CMAKE_C_LINK_FLAGS          " -lc")
  STRING_APPEND(CMAKE_CXX_LINK_FLAGS        " -lstdc++ -lgcc_s -lc")
  STRING_APPEND(CMAKE_MODULE_LINKER_FLAGS   " -lstdc++ -lgcc_s -lc")
  STRING_APPEND(CMAKE_SHARED_LINKER_FLAGS   " -lstdc++ -lgcc_s -lc")
  STRING_APPEND(QUOTED_CMAKE_CXX_LINK_FLAGS " -lstdc++ -lgcc_s -lc")
ENDIF()
