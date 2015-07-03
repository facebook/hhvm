HHVM_DEFINE_EXTENSION("simplexml" REQUIRED
  SOURCES
    ext_simplexml.cpp
  HEADERS
    ext_simplexml.h
    ext_simplexml_include.h
  IDL
    simplexml.idl.json
  DEPENDS_UPON
    ext_domdocument
    ext_libxml
    ext_std
    libXML2
    systemlib
)
