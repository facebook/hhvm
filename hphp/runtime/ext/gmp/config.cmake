HHVM_DEFINE_EXTENSION("gmp" EXPLICIT
  IS_ENABLED EXT_GMP
  SOURCES
    ext_gmp.cpp
  HEADERS
    ext_gmp.h
  SYSTEMLIB
    ext_gmp.php
  DEPENDS
    libGmp
)
