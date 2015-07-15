HHVM_DEFINE_EXTENSION("simplexml" REQUIRED
  SOURCES
    ext_simplexml.cpp
  HEADERS
    ext_simplexml.h
    ext_simplexml_include.h
  IDL
    ../../../system/idl/simplexml.idl.json
  DEPENDS
    libXML2
)
