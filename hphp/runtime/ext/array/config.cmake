HHVM_DEFINE_EXTENSION("array" REQUIRED
  SOURCES
    ext_array.cpp
  HEADERS
    ext_array.h
  EXTENSION_LIBRARY
    ext_array.php
  DEPENDS_UPON
    ext_collections
    ext_generator
    ext_std
)
