HHVM_DEFINE_EXTENSION("xenon" REQUIRED
  SOURCES
    ext_xenon.cpp
  HEADERS
    ext_xenon.h
  EXTENSION_LIBRARY
    ext_xenon.php
  DEPENDS_UPON
    ext_std
)
