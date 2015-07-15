HHVM_DEFINE_EXTENSION("odbc" IMPLICIT
  SOURCES
    ext_odbc.cpp
  SYSTEMLIB
    ext_odbc.php
  DEPENDS
    libUODBC
)
