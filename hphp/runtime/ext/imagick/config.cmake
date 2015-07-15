HHVM_DEFINE_EXTENSION("imagick"
  IS_ENABLED EXT_IMAGICK
  SOURCES
    constants.cpp
    ext_imagick.cpp
    imagick.cpp
    imagickdraw.cpp
    imagickpixel.cpp
    imagickpixeliterator.cpp
  HEADERS
    constants.h
    ext_imagick.h
  SYSTEMLIB
    ext_imagick.php
  DEPENDS
    libMagickWand
)
