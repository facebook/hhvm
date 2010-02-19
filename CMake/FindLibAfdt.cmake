# - Find LibAfdt (a cross platform RPC lib/tool)
# This module defines
# LibAfdt_INCLUDE_DIR, where to find LibAfdt headers
# LibAfdt_LIBS, LibAfdt libraries
# LibAfdt_FOUND, If false, do not try to use LibAfdt
 
find_path(LibAfdt_INCLUDE_DIR afdt.h PATHS
    /usr/local/include
    /opt/local/include
  )
 
set(LibAfdt_LIB_PATHS /usr/local/lib /opt/local/lib)
find_library(LibAfdt_LIB NAMES afdt PATHS ${LibAfdt_LIB_PATHS})
 
if (LibAfdt_LIB AND LibAfdt_INCLUDE_DIR)
  set(LibAfdt_FOUND TRUE)
  set(LibAfdt_LIBS ${LibAfdt_LIB})
else ()
  set(LibAfdt_FOUND FALSE)
endif ()
 
if (LibAfdt_FOUND)
  if (NOT LibAfdt_FIND_QUIETLY)
    message(STATUS "Found libafdt: ${LibAfdt_LIBS}")
  endif ()
else ()
  if (LibAfdt_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find libafdt.")
  endif ()
  message(STATUS "libafdt NOT found.")
endif ()
 
mark_as_advanced(
    LibAfdt_LIB
    LibAfdt_INCLUDE_DIR
  )
