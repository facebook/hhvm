HHVM_DEFINE_EXTENSION("watchman" IMPLICIT
  SOURCES
    ext_watcher.cpp
  SYSTEMLIB
    ext_watcher.php
  DEPENDS
    libwatchmanclient
)
