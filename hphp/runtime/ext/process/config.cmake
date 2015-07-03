HHVM_DEFINE_EXTENSION("process" REQUIRED
  SOURCES
    ext_process.cpp
  HEADERS
    ext_process.h
  EXTENSION_LIBRARY
    ext_process.php
  DEPENDS_UPON
    ext_std
    ext_string
    libFolly
    osPosix
)
