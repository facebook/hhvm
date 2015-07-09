HHVM_DEFINE_EXTENSION("domdocument"
  PRETTY_NAME "DOMDocument"
  SOURCES
    ext_domdocument.cpp
  HEADERS
    ext_domdocument.h
    ext_domdocument_includes.h
  EXTENSION_LIBRARY
    ext_domdocument.php
  DEPENDS_UPON
    ext_libxml
    ext_simplexml
    ext_std
    ext_string
    libXML2
    systemlib
)