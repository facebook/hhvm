if (APPLE)
  option(ENABLE_EXTENSION_MCROUTER "Build the imap extension" OFF)
endif()

HHVM_DEFINE_EXTENSION("mcrouter"
  SOURCES
    ext_mcrouter.cpp
  SYSTEMLIB
    ext_mcrouter.php
  DEPENDS
    libMCRouter
    varENABLE_MCROUTER
)
