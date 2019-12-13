HHVM_DEFINE_EXTENSION("protobuf"
  PRETTY_NAME "Protobuf"
  SOURCES
    ext_protobuf.cpp
    reader.cpp
    writer.cpp
  HEADERS
    ext_protobuf.h
    reader.h
    writer.h
    php_protobuf.h
  SYSTEMLIB
    ext_protobuf.php
)


