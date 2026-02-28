if(LMDB_FOUND)
  set(LMDB_FIND_QUIETLY TRUE)
endif()

find_path(LMDB_INCLUDE_DIR
  NAMES lmdb.h
  PATHS /usr/include /usr/local/include /usr/pkg/include
)

find_library(LMDB_LIBRARY
  NAMES lmdb
  PATHS /lib /usr/lib /usr/local/lib /usr/pkg/lib
)

if(LMDB_INCLUDE_DIR AND LMDB_LIBRARY)
  set(LMDB_FOUND TRUE)
endif()
