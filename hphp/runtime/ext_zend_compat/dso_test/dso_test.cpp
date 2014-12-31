/*
 * From
 *  http://devzone.zend.com/303/
 * "hello" was renamed to "dso_test".
 */
#include "dso_test.h"

int dso_test_php_post_deactivate(void)
{
  return SUCCESS;
}

ZEND_DECLARE_MODULE_GLOBALS(dso_test)

static zend_function_entry dso_test_functions[] = {
  PHP_FE(dso_test_world, NULL)
  PHP_FE(dso_test_long, NULL)
  PHP_FE(dso_test_double, NULL)
  PHP_FE(dso_test_bool, NULL)
  PHP_FE(dso_test_null, NULL)
  {NULL, NULL, NULL, 0, 0}
};

zend_module_entry dso_test_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_DSO_TEST_WORLD_EXTNAME,
  dso_test_functions,

  PHP_MINIT(dso_test),
  PHP_MSHUTDOWN(dso_test),

  PHP_RINIT(dso_test),
  PHP_RSHUTDOWN(dso_test),

  PHP_MINFO(dso_test),
  PHP_DSO_TEST_WORLD_VERSION,

  PHP_MODULE_GLOBALS(dso_test),
  PHP_GINIT(dso_test),
  PHP_GSHUTDOWN(dso_test),

  dso_test_php_post_deactivate,

  STANDARD_MODULE_PROPERTIES_EX
};

static PHP_INI_MH(dso_test_greeting_mh)
{
  return SUCCESS;
}

PHP_INI_BEGIN()
  PHP_INI_ENTRY("dso_test.greeting",
    "Hello World", PHP_INI_ALL, dso_test_greeting_mh)
  STD_PHP_INI_ENTRY("dso_test.direction",
    "1", PHP_INI_ALL, OnUpdateBool, direction,
    zend_dso_test_globals, dso_test_globals)
PHP_INI_END()

static void php_dso_test_init_globals(zend_dso_test_globals *dso_test_globals)
{
  dso_test_globals->direction = 1;
}

/* zm_globals_ctor_dso_test */
PHP_GINIT_FUNCTION(dso_test)
{
}

/* zm_globals_dtor_dso_test */
PHP_GSHUTDOWN_FUNCTION(dso_test)
{
}

/* zm_activate_dso_test */
PHP_RINIT_FUNCTION(dso_test)
{
  DSO_TEST_G(counter) = 0;
  return SUCCESS;
}

/* zm_deactivate_dso_test */
PHP_RSHUTDOWN_FUNCTION(dso_test)
{
  return SUCCESS;
}

/* zm_startup_dso_test */
PHP_MINIT_FUNCTION(dso_test)
{
  ZEND_INIT_MODULE_GLOBALS(dso_test, php_dso_test_init_globals, NULL);
  REGISTER_INI_ENTRIES();
  return SUCCESS;
}

/* zm_shutdown_dso_test */
PHP_MSHUTDOWN_FUNCTION(dso_test)
{
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}

/* zif_dso_test_world */
PHP_FUNCTION(dso_test_world)
{
  RETURN_STRING("Hello World", 1);
}

/* zif_dso_test_long */
PHP_FUNCTION(dso_test_long)
{
  if (DSO_TEST_G(direction)) {
    DSO_TEST_G(counter)++;
  } else {
    DSO_TEST_G(counter)--;
  }

  RETURN_LONG(DSO_TEST_G(counter));
}

/* zif_dso_test_double */
PHP_FUNCTION(dso_test_double)
{
  RETURN_DOUBLE(3.1415926535);
}

/* zif_dso_test_bool */
PHP_FUNCTION(dso_test_bool)
{
  RETURN_BOOL(1);
}

/* zif_dso_test_null */
PHP_FUNCTION(dso_test_null)
{
  RETURN_NULL();
}

/* zm_info_dso_test */
PHP_MINFO_FUNCTION(dso_test)
{
}

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HHVM)
  #define COMPILE_DL_DSO_TEST 1
#endif

#ifdef COMPILE_DL_DSO_TEST
ZEND_DLEXPORT zend_module_entry *get_module (void);
ZEND_GET_MODULE (dso_test)
#endif

#ifdef __cplusplus
}
#endif
