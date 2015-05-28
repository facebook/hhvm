if (ENABLE_ASYNC_MYSQL)
  HHVM_EXTENSION(async_mysql ext_async_mysql.cpp)
  HHVM_SYSTEMLIB(async_mysql
    ext_async_mysql.php
    ext_async_mysql_exceptions.php
    ext_mysqlrow.php)
  message(STATUS "Building async MySQL extension")
else()
  message("Not building async MySQL extension")
endif()
