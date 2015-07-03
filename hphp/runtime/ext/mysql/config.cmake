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
  EXTENSION_LIBRARY
    ext_mysql.php
    ext_mysql-async.php
  DEPENDS_UPON
    ext_pcre
    ext_std
    libFolly
    libMySQL
    systemlib
)

HHVM_DEFINE_EXTENSION("mysqli"
  PRETTY_NAME "MySQLi"
  SOURCES
    ext_mysqli.cpp
  EXTENSION_LIBRARY
    ext_mysqli.php
  DEPENDS_UPON
    ext_mysql
)
