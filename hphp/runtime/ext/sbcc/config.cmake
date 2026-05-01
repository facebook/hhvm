HHVM_DEFINE_EXTENSION("sbcc"
  SOURCES
    format/sbcc-cache.cpp
    format/sbcc-reader.cpp
    ext_sbcc.cpp
  SYSTEMLIB
    ext_sbcc.php
)
