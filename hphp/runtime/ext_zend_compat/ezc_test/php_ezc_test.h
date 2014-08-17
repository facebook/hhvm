
#ifndef PHP_EZC_TEST_H
#define PHP_EZC_TEST_H

extern zend_module_entry ezc_test_module_entry;
#define phpext_ezc_test_ptr &ezc_test_module_entry

#define PHP_EZC_TEST_VERSION "0.1.0"

#ifdef PHP_WIN32
#  define PHP_EZC_TEST_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#  define PHP_EZC_TEST_API __attribute__ ((__visibility__("default")))
#else
#  define PHP_EZC_TEST_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(ezc_test)
  long  ini_integer;
  char *ini_string;
  int init;
  zval * user_global;
ZEND_END_MODULE_GLOBALS(ezc_test)

#ifdef ZTS
#define EZC_TEST_G(v) TSRMG(ezc_test_globals_id, zend_ezc_test_globals *, v)
#else
#define EZC_TEST_G(v) (ezc_test_globals.v)
#endif

struct _php_ezctest_obj {
  zend_object std;
  int clone_count;
};
typedef struct _php_ezctest_obj php_ezctest_obj;

#endif  /* PHP_EZC_TEST_H */
