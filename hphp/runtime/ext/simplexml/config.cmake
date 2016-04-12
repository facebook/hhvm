HHVM_DEFINE_EXTENSION("simplexml" REQUIRED
  SOURCES
    ext_simplexml.cpp
  HEADERS
    ext_simplexml.h
    ext_simplexml_include.h
  SYSTEMLIB
    ext_simplexml.php
  DEPENDS
    libXML2
)
