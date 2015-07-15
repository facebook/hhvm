HHVM_DEFINE_EXTENSION("xmlwriter"
  SOURCES
    ext_xmlwriter.cpp
  SYSTEMLIB
    ext_xmlwriter.php
  DEPENDS
    libXML2
)
