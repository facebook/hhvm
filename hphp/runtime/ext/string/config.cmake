HHVM_DEFINE_EXTENSION("string" REQUIRED
  SOURCES
    ext_string.cpp
  HEADERS
    ext_string.h
  SYSTEMLIB
    ext_string.php
  DEPENDS
    libFolly
)
