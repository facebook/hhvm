HHVM_DEFINE_EXTENSION("snappy" IMPLICIT
  SOURCES
    ext_snappy.cpp
  SYSTEMLIB
    ext_snappy.php
  DEPENDS
    libSnappy
)
