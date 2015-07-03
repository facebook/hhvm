HHVM_DEFINE_EXTENSION("datetime" REQUIRED
  SOURCES
    ext_datetime.cpp
  HEADERS
    ext_datetime.h
  EXTENSION_LIBRARY
    ext_datetime.php
  DEPENDS_UPON
    systemlib
)