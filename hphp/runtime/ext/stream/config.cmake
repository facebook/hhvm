HHVM_DEFINE_EXTENSION("stream" REQUIRED
  SOURCES
    ext_stream.cpp
    ext_stream-user-filters.cpp
  HEADERS
    ext_stream.h
    ext_stream-user-filters.h
  SYSTEMLIB
    ext_stream.php
    ext_stream-user-filters.php
)
