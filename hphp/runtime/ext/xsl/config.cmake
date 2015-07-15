HHVM_DEFINE_EXTENSION("xsl"
  SOURCES
    ext_xsl.cpp
  SYSTEMLIB
    ext_xsl.php
  DEPENDS
    libXSLT
)
