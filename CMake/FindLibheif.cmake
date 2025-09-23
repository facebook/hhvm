if(LIBHEIF_FOUND)
  set(LIBHEIF_FIND_QUIETLY TRUE)
endif()

find_path(LIBHEIF_INCLUDE_DIR
  NAMES heif.h
  PATH_SUFFIXES libheif
  PATHS /usr/include /usr/local/include /usr/pkg/include
)

find_library(LIBHEIF_LIBRARY
  NAMES heif
  PATHS /lib /usr/lib /usr/local/lib /usr/pkg/lib
)

if(LIBHEIF_INCLUDE_DIR AND LIBHEIF_LIBRARY)
  set(LIBHEIF_FOUND TRUE)
endif()

mark_as_advanced(
  LIBHEIF_INCLUDE_DIR
  LIBHEIF_LIBRARY
)
