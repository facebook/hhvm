HHVM_DEFINE_EXTENSION("fribidi" IMPLICIT
  SOURCES
    ext_fribidi.cpp
  SYSTEMLIB
    ext_fribidi.php
  DEPENDS
    "libFribidi 0.19.6"
    libGlib
)
