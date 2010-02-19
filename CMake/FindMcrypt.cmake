# - Find Mcrypt (a cross platform RPC lib/tool)
# This module defines
# Mcrypt_INCLUDE_DIR, where to find Mcrypt headers
# Mcrypt_LIBS, Mcrypt libraries
# Mcrypt_FOUND, If false, do not try to use Mcrypt
 
find_path(Mcrypt_INCLUDE_DIR mcrypt.h PATHS
    /usr/local/include
    /opt/local/include
  )


#find_library can't seem to find a 64-bit binary if the 32-bit isn't there

set(Mcrypt_LIB_PATHS /usr/local/lib /opt/local/lib /usr/lib64)
find_library(Mcrypt_LIB NAMES mcrypt PATHS ${Mcrypt_LIB_PATHS})
 
if (Mcrypt_LIB AND Mcrypt_INCLUDE_DIR)
  set(Mcrypt_FOUND TRUE)
  set(Mcrypt_LIBS ${Mcrypt_LIB})
else ()
  set(Mcrypt_FOUND FALSE)
endif ()
 
if (Mcrypt_FOUND)
  if (NOT Mcrypt_FIND_QUIETLY)
    message(STATUS "Found mcrypt: ${Mcrypt_LIBS}")
  endif ()
else ()
  if (Mcrypt_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find mcrypt library.")
  endif ()
  message(STATUS "mcrypt NOT found.")
endif ()
 
mark_as_advanced(
    Mcrypt_LIB
    Mcrypt_INCLUDE_DIR
  )
