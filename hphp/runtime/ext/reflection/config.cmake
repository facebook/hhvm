HHVM_DEFINE_EXTENSION("reflection" REQUIRED
  SOURCES
    ext_reflection.cpp
  HEADERS
    ext_reflection.h
  SYSTEMLIB
    ext_reflection.php
    ext_reflection_hni.php
    ext_reflection-classes.php
    ext_reflection-internals-functions.php
)
