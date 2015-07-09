HHVM_DEFINE_EXTENSION("fribidi"
  SOURCES
    ext_fribidi.cpp
  EXTENSION_LIBRARY
    ext_fribidi.php
  DEPENDS_UPON
    "libFribidi 0.19.6"
    libGlib
)
