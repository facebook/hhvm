HHVM_DEFINE_EXTENSION("async_mysql"
  PRETTY_NAME "Async MySQL"
  SOURCES
    ext_async_mysql.cpp
  HEADERS
    ext_async_mysql.h
  SYSTEMLIB
    ext_async_mysql.php
    ext_async_mysql_exceptions.php
    ext_mysqlrow.php
  DEPENDS
    libFolly
    libSquangle
    libWebscaleSQL
    varENABLE_ASYNC_MYSQL
)
