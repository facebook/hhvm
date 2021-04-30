HHVM_DEFINE_EXTENSION("watchman" IMPLICIT
  SOURCES
    ext_watchman.cpp
  SYSTEMLIB
    ext_watchman.php
  DEPENDS
    libwatchmanclient
)
