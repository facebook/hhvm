# - Try to find libmagickwand
# Once done this will define
#
#  LIBMAGICKWAND_FOUND - system has libmagickwand
#  LIBMAGICKWAND_INCLUDE_DIRS - the libmagickwand include directory
#  LIBMAGICKWAND_LIBRARIES - Link these to use libmagickwand
#  LIBMAGICKWAND_DEFINITIONS - Compiler switches required for using libmagickwand
#
#  Copyright (c) 2008 Bernhard Walle <bernhard.walle@gmx.de>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LIBMAGICKWAND_LIBRARIES AND LIBMAGICKWAND_INCLUDE_DIRS)
  set (LibMagicWand_FIND_QUIETLY TRUE)
endif (LIBMAGICKWAND_LIBRARIES AND LIBMAGICKWAND_INCLUDE_DIRS)

set (ImageMagick_FIND_QUIETLY TRUE)
find_package(ImageMagick COMPONENTS MagickWand MagickCore)
if (ImageMagick_MagickWand_FOUND)
  set (LIBMAGICKWAND_INCLUDE_DIRS ${ImageMagick_MagickWand_INCLUDE_DIR} CACHE STRING "")
  set (LIBMAGICKWAND_LIBRARIES ${ImageMagick_MagickWand_LIBRARY} CACHE STRING "")
endif ()
if (ImageMagick_MagickCore_FOUND)
  set (LIBMAGICKCORE_INCLUDE_DIRS ${ImageMagick_MagickCore_INCLUDE_DIR} CACHE STRING "")
  set (LIBMAGICKCORE_LIBRARIES ${ImageMagick_MagickCore_LIBRARY} CACHE STRING "")
endif ()

find_path (LIBMAGICKWAND_INCLUDE_DIRS
    NAMES
      wand/MagickWand.h
    PATH_SUFFIXES
      ImageMagick-6
    PATHS
      /usr/include/ImageMagick
      /usr/local/include/ImageMagick
      ENV CPATH)

find_library (LIBMAGICKWAND_LIBRARIES
    NAMES
      MagickWand
      MagickWand-6.Q16
    PATHS
      /usr/lib
      /usr/local/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)

find_library (LIBMAGICKCORE_LIBRARIES
    NAMES
      MagickCore
      MagickCore-6.Q16
    PATHS
      /usr/lib
      /usr/local/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)

include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBMAGICKWAND_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibMagicWand DEFAULT_MSG
    LIBMAGICKWAND_LIBRARIES
    LIBMAGICKCORE_LIBRARIES
    LIBMAGICKWAND_INCLUDE_DIRS)

mark_as_advanced(LIBMAGICKWAND_INCLUDE_DIRS LIBMAGICKWAND_LIBRARIES LIBMAGICKCORE_LIBRARIES)
