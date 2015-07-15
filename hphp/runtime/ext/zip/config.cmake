HHVM_DEFINE_EXTENSION("zip"
  SOURCES
    ext_zip.cpp
  SYSTEMLIB
    ext_zip.php
  DEPENDS
    libZip
)
