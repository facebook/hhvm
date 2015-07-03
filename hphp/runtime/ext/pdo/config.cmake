HHVM_DEFINE_EXTENSION("pdo"
  SOURCES
    ext_pdo.cpp
    pdo_driver.cpp
  HEADERS
    ext_pdo.h
    pdo_driver.h
  EXTENSION_LIBRARY
    ext_pdo.php
  DEPENDS_UPON
    ext_array
    ext_pdo_mysql OPTIONAL
    ext_pdo_sqlite OPTIONAL
    ext_std
    ext_stream
    ext_string
    systemlib
)

HHVM_DEFINE_EXTENSION("pdo_mysql"
  SOURCES
    pdo_mysql.cpp
  HEADERS
    pdo_mysql.h
  DEPENDS_UPON
    ext_pdo
    libMySQL
)

HHVM_DEFINE_EXTENSION("pdo_sqlite"
  SOURCES
    pdo_sqlite.cpp
  HEADERS
    pdo_sqlite.h
  DEPENDS_UPON
    ext_pdo
    ext_sqlite3
    ext_std
    ext_stream
    libSQLite
)
