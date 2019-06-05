HHVM_DEFINE_EXTENSION("mysql"
  PRETTY_NAME "MySQL"
  SOURCES
    ext_mysql.cpp
    mysql_common.cpp
    mysql_stats.cpp
  HEADERS
    ext_mysql.h
    mysql_common.h
    mysql_stats.h
  SYSTEMLIB
    ext_mysql.php
    ext_mysql-async.php
  DEPENDS
    libFolly
    libMySQL
)
