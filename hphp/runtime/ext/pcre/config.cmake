HHVM_DEFINE_EXTENSION("pcre" REQUIRED
  SOURCES
    ext_pcre.cpp
  HEADERS
    ext_pcre.h
  SYSTEMLIB
    ext_pcre.php
  DEPENDS
    libPCRE
)
