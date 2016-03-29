HHVM_DEFINE_EXTENSION("apc" REQUIRED
  SOURCES
    ext_apc.cpp
    snapshot.cpp
  HEADERS
    ext_apc.h
    snapshot.h
    snapshot-builder.h
    snapshot-loader.h
  SYSTEMLIB
    ext_apc.php
)
