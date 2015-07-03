HHVM_DEFINE_EXTENSION("xmlwriter"
  SOURCES
    ext_xmlwriter.cpp
  EXTENSION_LIBRARY
    ext_xmlwriter.php
  DEPENDS_UPON
    libXML2
)
