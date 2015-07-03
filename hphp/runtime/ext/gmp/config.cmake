HHVM_DEFINE_EXTENSION("gmp"
  IS_ENABLED EXT_GMP
  SOURCES
    ext_gmp.cpp
  HEADERS
    ext_gmp.h
  EXTENSION_LIBRARY
    ext_gmp.php
  DEPENDS_UPON
    libGmp
)
