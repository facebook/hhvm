HHVM_DEFINE_EXTENSION("reflection" REQUIRED
  SOURCES
    ext_reflection.cpp
  HEADERS
    ext_reflection.h
  EXTENSION_LIBRARY
    ext_reflection.php
    ext_reflection_hni.php
    ext_reflection-classes.php
    ext_reflection-internals-functions.php
  DEPENDS_UPON
    ext_debugger
    ext_closure
    ext_collections
    ext_std
    ext_string
    systemlib
)
