HHVM_DEFINE_EXTENSION("pcre" REQUIRED
  SOURCES
    ext_pcre.cpp
  HEADERS
    ext_pcre.h
  EXTENSION_LIBRARY
    ext_pcre.php
  DEPENDS_UPON
    ext_mbstring
    ext_std
    ext_string
    libPCRE
)
