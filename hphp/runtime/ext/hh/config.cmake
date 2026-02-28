HHVM_DEFINE_EXTENSION("hh" REQUIRED
  SOURCES
    ext_hh.cpp
  HEADERS
    ext_hh.h
  SYSTEMLIB
    ext_hh.php
)

HHVM_DEFINE_EXTENSION("implicit_context" REQUIRED
  SOURCES
    ext_implicit_context.cpp
  HEADERS
    ext_implicit_context.h
  SYSTEMLIB
    ext_implicit_context.php
)
