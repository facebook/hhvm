if (ENABLE_ASYNC_MYSQL)
  # The fact that the async code uses a replacement for MySQL makes it not exactly clean to support
  # via the new mechanism, but this is better than it not being possible to build it at all.
  HHVM_EXTENSION_INTERNAL_CHECK_DEPENDENCY(ENABLE_ASYNC_MYSQL async_mysql MySQL hphp_runtime_ext)
  HHVM_EXTENSION(async_mysql ext_async_mysql.cpp)
  HHVM_SYSTEMLIB(async_mysql
    ext_async_mysql.php
    ext_async_mysql_exceptions.php
    ext_mysqlrow.php)
  message(STATUS "Building async MySQL extension")
else()
  message("Not building async MySQL extension")
endif()
