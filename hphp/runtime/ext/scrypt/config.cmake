HHVM_DEFINE_EXTENSION("scrypt" IMPLICIT
  SOURCES
    ext_scrypt.cpp
    crypto/crypto_scrypt.cpp
    crypto/params.cpp
  HEADERS
    crypto/crypto_scrypt.h
    crypto/params.h
  SYSTEMLIB
    ext_scrypt.php
  DEPENDS
    "libsodium 1.0.9"
)
