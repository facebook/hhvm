# - Find LibEv (a cross event library)
# This module defines
# LIBEV_INCLUDE_DIR, where to find LibEv headers
# LIBEV_LIB, LibEv libraries
# LibEv_FOUND, If false, do not try to use libev

set(LibEv_EXTRA_PREFIXES /usr/local /opt/local "$ENV{HOME}")
foreach(prefix ${LibEv_EXTRA_PREFIXES})
	list(APPEND LibEv_INCLUDE_PATHS "${prefix}/include")
	list(APPEND LibEv_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBEV_INCLUDE_DIR ev.h PATHS ${LibEv_INCLUDE_PATHS})
find_library(LIBEV_LIB NAMES libev PATHS ${LibEv_LIB_PATHS})

if (LIBEV_LIB AND LIBEV_INCLUDE_DIR)
  set(LibEv_FOUND TRUE)
  set(LIBEV_LIB ${LIBEV_LIB})
else ()
  set(LibEv_FOUND FALSE)
endif ()

if (LibEv_FOUND)
  if (NOT LibEv_FIND_QUIETLY)
    message(STATUS "Found libev: ${LIBEV_LIB}")
  endif ()
else ()
    if (LibEv_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libev.")
    endif ()
    message(STATUS "libev NOT found.")
endif ()

mark_as_advanced(
    LIBEV_LIB
    LIBEV_INCLUDE_DIR
  )
