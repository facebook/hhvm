include_directories(
  ./
  ../../../runtime/ext_zend_compat/php-src
  ../../../runtime/ext_zend_compat/php-src/Zend/
  ../../../runtime/ext_zend_compat/php-src/TSRM/
  ../../../runtime/ext_zend_compat/php-src/main/
)

HHVM_EXTENSION(dso_test     dso_test.cpp)
HHVM_SYSTEMLIB(dso_test ext_dso_test.php)

