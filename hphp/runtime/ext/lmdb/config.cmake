HHVM_DEFINE_EXTENSION("lmdb"
  IS_ENABLED EXT_LMDB
  SOURCES
    ext_lmdb.cpp
  DEPENDS
    libLmdb
  SYSTEMLIB
    ext_lmdb.php
)
