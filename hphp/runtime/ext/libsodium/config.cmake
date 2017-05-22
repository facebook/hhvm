HHVM_DEFINE_EXTENSION("libsodium" IMPLICIT
  SOURCES
    ext_libsodium.cpp
  SYSTEMLIB
    ext_libsodium.php
  DEPENDS
    "libsodium 1.0.7"
)
