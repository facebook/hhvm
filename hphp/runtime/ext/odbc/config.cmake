HHVM_DEFINE_EXTENSION("odbc"
  SOURCES
    ext_odbc.cpp
  EXTENSION_LIBRARY
    ext_odbc.php
  DEPENDS_UPON
    libUODBC
)
