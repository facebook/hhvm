HHVM_DEFINE_EXTENSION("pdo_mysql"
  SOURCES
    pdo_mysql.cpp
  HEADERS
    pdo_mysql.h
  DEPENDS
    libMySQL
)
