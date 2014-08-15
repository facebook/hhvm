/*
 * From
 *  http://devzone.zend.com/303/
 */
#ifndef PHP_DSO_TEST_H
#define PHP_DSO_TEST_H 1

#ifdef ZTS
  #include "TSRM.h"
#endif
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "php.h"
#include "php_ini.h"

ZEND_BEGIN_MODULE_GLOBALS(dso_test)
    long counter;
    zend_bool direction;
ZEND_END_MODULE_GLOBALS(dso_test)

#ifdef ZTS
#define DSO_TEST_G(v) TSRMG(dso_test_globals_id, zend_dso_test_globals *, v)
#else
#define DSO_TEST_G(v) (dso_test_globals.v)
#endif

#define PHP_DSO_TEST_WORLD_VERSION "1.0"
#define PHP_DSO_TEST_WORLD_EXTNAME "dso_test"

PHP_MINIT_FUNCTION(dso_test);
PHP_MSHUTDOWN_FUNCTION(dso_test);

PHP_RINIT_FUNCTION(dso_test);
PHP_RSHUTDOWN_FUNCTION(dso_test);

PHP_MINFO_FUNCTION(dso_test);

PHP_GINIT_FUNCTION(dso_test);
PHP_GSHUTDOWN_FUNCTION(dso_test);

PHP_FUNCTION(dso_test_world);
PHP_FUNCTION(dso_test_long);
PHP_FUNCTION(dso_test_double);
PHP_FUNCTION(dso_test_bool);
PHP_FUNCTION(dso_test_null);

extern zend_module_entry dso_test_module_entry;
#define phpext_dso_test_ptr &dso_test_module_entry

#endif
