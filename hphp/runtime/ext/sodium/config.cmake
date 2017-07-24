HHVM_DEFINE_EXTENSION("sodium" IMPLICIT
  SOURCES
    ext_sodium.cpp
  SYSTEMLIB
    ext_sodium.php
  DEPENDS
    "libsodium 1.0.7"
)
