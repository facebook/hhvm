HHVM_DEFINE_EXTENSION("closure" REQUIRED
  SOURCES
    ext_closure.cpp
  HEADERS
    ext_closure.h
  IDL
    ../../../system/idl/closure.idl.json
)
