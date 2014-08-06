include_directories(
  ./
  ../../../runtime/ext_zend_compat/php-src
  ../../../runtime/ext_zend_compat/php-src/Zend/
  ../../../runtime/ext_zend_compat/php-src/TSRM/
  ../../../runtime/ext_zend_compat/php-src/main/
)

HHVM_EXTENSION(hello     hello.cpp)
HHVM_SYSTEMLIB(hello ext_hello.php)

