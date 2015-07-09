HHVM_DEFINE_EXTENSION("stream" REQUIRED
  SOURCES
    ext_stream.cpp
    ext_stream-user-filters.cpp
  HEADERS
    ext_stream.h
    ext_stream-user-filters.h
  EXTENSION_LIBRARY
    ext_stream.php
    ext_stream-user-filters.php
  DEPENDS_UPON
    ext_array
    ext_sockets
    ext_std
    systemlib
)
