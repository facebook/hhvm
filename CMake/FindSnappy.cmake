# - Find Snappy
# This module defines
# SNAPPY_INCLUDE_DIR, where to find Snappy headers
# SNAPPY_LIBS, Snappy libraries
# SNAPPY_FOUND, If false, do not try to use Snappy

find_path(SNAPPY_INCLUDE_DIR snappy.h PATHS
  /usr/local/include
  /opt/local/include
)

#find_library can't seem to find a 64-bit binary if the 32-bit isn't there
set(SNAPPY_LIB_NAMES libsnappy snappy)
set(SNAPPY_LIB_PATHS /usr/local/lib /opt/local/lib /usr/lib64)
find_library(SNAPPY_LIB NAMES ${SNAPPY_LIB_NAMES} PATHS ${SNAPPY_LIB_PATHS})

if (SNAPPY_LIB AND SNAPPY_INCLUDE_DIR)
  set(SNAPPY_FOUND TRUE)
  set(SNAPPY_LIBS ${SNAPPY_LIB})
else ()
  set(SNAPPY_FOUND FALSE)
endif ()

if (SNAPPY_FOUND)
  if (NOT SNAPPY_FIND_QUIETLY)
    message(STATUS "Found Snappy: ${SNAPPY_LIBS}")
  endif ()
else ()
  if (SNAPPY_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find the Snappy library.")
  endif ()
  message(STATUS "Snappy NOT found.")
endif ()

mark_as_advanced(
  SNAPPY_LIB
  SNAPPY_INCLUDE_DIR
)
