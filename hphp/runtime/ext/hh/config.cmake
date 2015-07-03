HHVM_DEFINE_EXTENSION("hh" REQUIRED
  SOURCES
    ext_hh.cpp
  HEADERS
    ext_hh.h
  EXTENSION_LIBRARY
    ext_hh.php
  DEPENDS_UPON
    ext_fb
)
