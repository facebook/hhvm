HHVM_DEFINE_EXTENSION("facts" IMPLICIT
  SOURCES
    ext_facts.cpp
  HEADERS
    ext_facts.h
  SYSTEMLIB
    ext_facts.php
  DEPENDS
    libFolly
    libWatchmanClient
)
