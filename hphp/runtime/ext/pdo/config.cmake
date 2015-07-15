HHVM_DEFINE_EXTENSION("pdo"
  SOURCES
    ext_pdo.cpp
    pdo_driver.cpp
  HEADERS
    ext_pdo.h
    pdo_driver.h
  SYSTEMLIB
    ext_pdo.php
)

HHVM_DEFINE_EXTENSION("pdo_mysql"
  SOURCES
    pdo_mysql.cpp
  HEADERS
    pdo_mysql.h
  DEPENDS
    libMySQL
)

HHVM_DEFINE_EXTENSION("pdo_sqlite"
  SOURCES
    pdo_sqlite.cpp
  HEADERS
    pdo_sqlite.h
  DEPENDS
    libSQLite
)
