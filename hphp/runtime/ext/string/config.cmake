HHVM_DEFINE_EXTENSION("string" REQUIRED
  SOURCES
    ext_string.cpp
  HEADERS
    ext_string.h
  EXTENSION_LIBRARY
    ext_string.php
  DEPENDS_UPON
    ext_std
    libFolly
    systemlib
)
