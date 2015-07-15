HHVM_DEFINE_EXTENSION("domdocument"
  PRETTY_NAME "DOMDocument"
  SOURCES
    ext_domdocument.cpp
  HEADERS
    ext_domdocument.h
    ext_domdocument_includes.h
  SYSTEMLIB
    ext_domdocument.php
  DEPENDS
    libXML2
)