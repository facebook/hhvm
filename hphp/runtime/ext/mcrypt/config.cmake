HHVM_DEFINE_EXTENSION("mcrypt"
  SOURCES
    ext_mcrypt.cpp
  SYSTEMLIB
    ext_mcrypt.php
  DEPENDS
    libMCrypt
)
