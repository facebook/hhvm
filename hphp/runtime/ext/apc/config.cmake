HHVM_DEFINE_EXTENSION("apc" REQUIRED
  SOURCES
    ext_apc.cpp
  HEADERS
    ext_apc.h
  EXTENSION_LIBRARY
    ext_apc.php
  DEPENDS_UPON
    ext_fb
)
