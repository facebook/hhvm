HHVM_DEFINE_EXTENSION("sockets" REQUIRED
  SOURCES
    ext_sockets.cpp
  HEADERS
    ext_sockets.h
    unix_socket_constants.h
  EXTENSION_LIBRARY
    ext_sockets.php
  DEPENDS_UPON
    ext_stream
    libFolly
)
