HHVM_DEFINE_EXTENSION("decl" REQUIRED
  SOURCES
    decl-extractor.cpp
    exception.cpp
    ext_decl.cpp
  HEADERS
    decl-extractor.h
    exception.h
  SYSTEMLIB
    ext_decl.php
)
