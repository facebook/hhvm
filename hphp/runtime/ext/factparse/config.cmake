HHVM_DEFINE_EXTENSION("factparse"
  SOURCES
    ext_factparse.cpp
  SYSTEMLIB
    ext_factparse.php
  DEPENDS
    libFolly
)
