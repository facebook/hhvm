HHVM_DEFINE_EXTENSION("lz4"
  SOURCES
    ext_lz4.cpp
  SYSTEMLIB
    ext_lz4.php
  DEPENDS
    libLZ4
)
