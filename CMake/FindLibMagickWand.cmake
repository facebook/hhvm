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

find_path (LIBMAGICKWAND_INCLUDE_DIRS
    NAMES
      wand/MagickWand.h
    PATHS
      /usr/include/ImageMagick
      /usr/local/include/ImageMagick
    ENV CPATH)

find_library (LIBMAGICKWAND_LIBRARIES
    NAMES
      MagickWand
    PATHS
      /usr/lib
      /usr/local/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)

include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBMAGICKWAND_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibMagicWand DEFAULT_MSG
    LIBMAGICKWAND_LIBRARIES
    LIBMAGICKWAND_INCLUDE_DIRS)

mark_as_advanced(LIBMAGICKWAND_INCLUDE_DIRS LIBMAGICKWAND_LIBRARIES ELF_GETSHDRSTRNDX)
