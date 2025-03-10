HHVM_DEFINE_EXTENSION("core" REQUIRED
  SOURCES
    ext_core.cpp
    ext_core_closure.cpp
  HEADERS
    ext_core.h
  SYSTEMLIB
    ext_core.php
)
