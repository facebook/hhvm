if (APPLE)
  option(ENABLE_EXTENSION_IMAP "Build the imap extension" OFF)
endif()

HHVM_DEFINE_EXTENSION("imap" IMPLICIT
  IS_ENABLED EXT_IMAP
  SOURCES
    ext_imap.cpp
  SYSTEMLIB
    ext_imap.php
  DEPENDS
    libCClient
)
