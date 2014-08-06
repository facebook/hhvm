/* From http://devzone.zend.com/303/extension-writing-part-i-introduction-to-php-and-zend/ */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"

#include "hello.h"

int hello_php_post_deactivate(void)
{
  return SUCCESS;
}

ZEND_DECLARE_MODULE_GLOBALS(hello)

static zend_function_entry hello_functions[] = {
  PHP_FE(hello_world, NULL)
  PHP_FE(hello_long, NULL)
  PHP_FE(hello_double, NULL)
  PHP_FE(hello_bool, NULL)
  PHP_FE(hello_null, NULL)
  {NULL, NULL, NULL, 0, 0}
};

zend_module_entry hello_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_HELLO_WORLD_EXTNAME,
  hello_functions,

  PHP_MINIT(hello),
  PHP_MSHUTDOWN(hello),

  PHP_RINIT(hello),
  PHP_RSHUTDOWN(hello),

  PHP_MINFO(hello),
  PHP_HELLO_WORLD_VERSION,

  PHP_MODULE_GLOBALS(hello),
  PHP_GINIT(hello),
  PHP_GSHUTDOWN(hello),

  hello_php_post_deactivate,

  STANDARD_MODULE_PROPERTIES_EX
};

static PHP_INI_MH(hello_greeting_mh)
{
  return SUCCESS;
}

PHP_INI_BEGIN()
  PHP_INI_ENTRY("hello.greeting", "Hello World", PHP_INI_ALL, hello_greeting_mh)
  STD_PHP_INI_ENTRY("hello.direction", "1", PHP_INI_ALL, OnUpdateBool, direction, zend_hello_globals, hello_globals)
PHP_INI_END()

static void php_hello_init_globals(zend_hello_globals *hello_globals)
{
  hello_globals->direction = 1;
}

/* zm_globals_ctor_hello */
PHP_GINIT_FUNCTION(hello)
{
}

/* zm_globals_dtor_hello */
PHP_GSHUTDOWN_FUNCTION(hello)
{
}

/* zm_activate_hello */
PHP_RINIT_FUNCTION(hello)
{
  HELLO_G(counter) = 0;
  return SUCCESS;
}

/* zm_deactivate_hello */
PHP_RSHUTDOWN_FUNCTION(hello)
{
  return SUCCESS;
}

/* zm_startup_hello */
PHP_MINIT_FUNCTION(hello)
{
  ZEND_INIT_MODULE_GLOBALS(hello, php_hello_init_globals, NULL);
  REGISTER_INI_ENTRIES();
  return SUCCESS;
}

/* zm_shutdown_hello */
PHP_MSHUTDOWN_FUNCTION(hello)
{
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}

/* zif_hello_world */
PHP_FUNCTION(hello_world)
{
  RETURN_STRING("Hello World", 1);
}

/* zif_hello_long */
PHP_FUNCTION(hello_long)
{
  if (HELLO_G(direction)) {
    HELLO_G(counter)++;
  } else {
    HELLO_G(counter)--;
  }

  RETURN_LONG(HELLO_G(counter));
}

/* zif_hello_double */
PHP_FUNCTION(hello_double)
{
  RETURN_DOUBLE(3.1415926535);
}

/* zif_hello_bool */
PHP_FUNCTION(hello_bool)
{
  RETURN_BOOL(1);
}

/* zif_hello_null */
PHP_FUNCTION(hello_null)
{
  RETURN_NULL();
}

/* zm_info_hello */
PHP_MINFO_FUNCTION(hello)
{
}

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HHVM)
  #define COMPILE_DL_HELLO 1
#endif

#ifdef COMPILE_DL_HELLO
ZEND_DLEXPORT zend_module_entry *get_module (void);
ZEND_GET_MODULE (hello)
#endif

#ifdef __cplusplus
}
#endif
