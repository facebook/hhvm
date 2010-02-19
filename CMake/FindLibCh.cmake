# - Find LibCh (a cross platform RPC lib/tool)
# This module defines
# LibCh_INCLUDE_DIR, where to find LibCh headers
# LibCh_LIBS, LibCh libraries
# LibCh_FOUND, If false, do not try to use LibCh
 
find_path(LibCh_INCLUDE_DIR ch/continuum.h PATHS
    /usr/local/include
    /opt/local/include
  )
 
set(LibCh_LIB_PATHS /usr/local/lib /opt/local/lib)
find_library(LibCh_LIB NAMES ch PATHS ${LibCh_LIB_PATHS})
 
if (LibCh_LIB AND LibCh_INCLUDE_DIR)
  set(LibCh_FOUND TRUE)
  set(LibCh_LIBS ${LibCh_LIB})
else ()
  set(LibCh_FOUND FALSE)
endif ()
 
if (LibCh_FOUND)
  if (NOT LibCh_FIND_QUIETLY)
    message(STATUS "Found libCh: ${LibCh_LIBS}")
  endif ()
else ()
  if (LibCh_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find libch library.")
  endif ()
  message(STATUS "libCh NOT found.")
endif ()
 
mark_as_advanced(
    LibCh_LIB
    LibCh_INCLUDE_DIR
  )
