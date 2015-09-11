HHVM_DEFINE_EXTENSION("pdo_sqlite"
  SOURCES
    pdo_sqlite.cpp
  HEADERS
    pdo_sqlite.h
  DEPENDS
    libSQLite
)
