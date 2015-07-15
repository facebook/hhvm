HHVM_DEFINE_EXTENSION("process" REQUIRED
  SOURCES
    ext_process.cpp
  HEADERS
    ext_process.h
  SYSTEMLIB
    ext_process.php
  DEPENDS
    libFolly
    osPosix
)
