HHVM_DEFINE_EXTENSION("url" REQUIRED
  SOURCES
    ext_url.cpp
  HEADERS
    ext_url.h
  EXTENSION_LIBRARY
    ext_url.php
  DEPENDS_UPON
    ext_curl
    ext_pcre
    ext_std
    ext_string
    systemlib
)
