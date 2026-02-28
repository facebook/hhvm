HHVM_DEFINE_EXTENSION("watcher" IMPLICIT
  SOURCES
    ext_watcher.cpp
    watcher-clock.cpp
    watcher-options.cpp
  SYSTEMLIB
    ext_watcher.php
  DEPENDS
    libwatchmanclient
)
