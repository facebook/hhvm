
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ezc_test.h"
#include "zend_exceptions.h"
#include <stdexcept>

ZEND_DECLARE_MODULE_GLOBALS(ezc_test)

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ezc_test.integer", "42", PHP_INI_ALL, OnUpdateLong, ini_integer, zend_ezc_test_globals, ezc_test_globals)
    STD_PHP_INI_ENTRY("ezc_test.string", "foobar", PHP_INI_ALL, OnUpdateString, ini_string, zend_ezc_test_globals, ezc_test_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ezc_test)
{
  REGISTER_INI_ENTRIES();
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ezc_test)
{
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ezc_test)
{
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ezc_test)
{
  if (EZC_TEST_G(user_global)) {
    zval_ptr_dtor(&EZC_TEST_G(user_global));
    EZC_TEST_G(user_global) = NULL;
  }
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ezc_test)
{
  php_info_print_table_start();
  php_info_print_table_header(2, "ezc_test support", "enabled");
  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
/* }}} */
/* {{{ PHP_GINIT_FUNCTION */
PHP_GINIT_FUNCTION(ezc_test)
{
  memset(ezc_test_globals, 0, sizeof(*ezc_test_globals));
  ezc_test_globals->init = 1;
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
PHP_GSHUTDOWN_FUNCTION(ezc_test)
{
  if (ezc_test_globals->init != 1) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "Globals not initialised correctly");
  }
  ezc_test_globals->init = 0;
}
/* }}} */

/* {{{ ezc_test_post_deactivate */
static int ezc_test_post_deactivate()
{
  return SUCCESS;
}
/* }}} */

/* {{{ proto mixed ezc_fetch_global()
 * Get the value of the global */
PHP_FUNCTION(ezc_fetch_global)
{
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }

  if (EZC_TEST_G(user_global) == NULL) {
    RETURN_NULL();
  }
  RETURN_ZVAL(EZC_TEST_G(user_global), 1, 0);
}
/* }}} */

/* {{{ proto void ezc_set_global(mixed value)
 * Set the value of the global */
PHP_FUNCTION(ezc_set_global)
{
  zval * arg;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg) == FAILURE) {
    RETURN_FALSE;
  }

  if (EZC_TEST_G(user_global)) {
    zval_ptr_dtor(&EZC_TEST_G(user_global));
    EZC_TEST_G(user_global) = NULL;
  }
  MAKE_STD_ZVAL(EZC_TEST_G(user_global));
  ZVAL_ZVAL(EZC_TEST_G(user_global), arg, 1, 0);
}
/* }}} */

/* {{{ proto mixed ezc_call(mixed function_name [, mixed parmeter] [, mixed ...])
 * Call a function, like call_user_func() */
PHP_FUNCTION(ezc_call)
{
  zval *retval_ptr = NULL;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f*",
			  &fci, &fci_cache, &fci.params, &fci.param_count) == FAILURE) {
    return;
  }

  fci.retval_ptr_ptr = &retval_ptr;

  if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS
		  && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
    COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
  }

  if (fci.params) {
    efree(fci.params);
  }
}
/* }}} */

/* {{{ proto mixed ezc_try_call(mixed function_name [, mixed parmeter] [, mixed ...])
 * Call a function. If it throws an exception, catch it and return the exception
 * object. Otherwise, return the return value. */
PHP_FUNCTION(ezc_try_call)
{
  zval *retval_ptr = NULL;
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f*",
			  &fci, &fci_cache, &fci.params, &fci.param_count) == FAILURE) {
    return;
  }
  fci.retval_ptr_ptr = &retval_ptr;

  if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS
		  && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
    if (EG(exception)) {
      /* Unlikely for a function to return a value despite throwing an
       * exception, but if it did, I suppose we would have to clean up
       * here */
      zval_ptr_dtor(fci.retval_ptr_ptr);
    } else {
      COPY_PZVAL_TO_ZVAL(*return_value, *fci.retval_ptr_ptr);
    }
  }

  if (fci.params) {
    efree(fci.params);
  }

  if (EG(exception)) {
    RETVAL_ZVAL(EG(exception), 1, 0);
    zend_clear_exception(TSRMLS_C);
  }
}
/* }}} */

/* {{{ proto mixed ezc_throw(string exception_class)
 * Throw an exception with the class of the given name */
PHP_FUNCTION(ezc_throw)
{
  char *class_name = NULL;
  int class_name_length;
  zend_class_entry *ce;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
			  &class_name, &class_name_length) == FAILURE) {
    RETURN_FALSE;
  }

  ce = zend_fetch_class_by_name(
		  class_name, class_name_length, NULL, ZEND_FETCH_CLASS_SILENT TSRMLS_CC);
  if (!ce) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "no such class \"%s\"",
			class_name);
    RETURN_FALSE;
  }
  zend_throw_exception(ce, "ezc_throw", 0 TSRMLS_CC);
}
/* }}} */

/* {{{ proto ezc_throw_cpp_std()
 * Throw a C++ std::exception */
PHP_FUNCTION(ezc_throw_std)
{
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }

  throw std::runtime_error("ezc_throw_std");
}
/* }}} */

/* {{{ proto ezc_throw_cpp_nonstd()
 * Throw a non-standard C++ exception */
PHP_FUNCTION(ezc_throw_nonstd)
{
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }

  throw "ezc_throw_nonstd";
}
/* }}} */

/* {{{ proto ezc_realpath(string path)
 * Return the resolved path
 */
PHP_FUNCTION(ezc_realpath)
{
	char *filename;
	int filename_len;
	char resolved_path_buff[MAXPATHLEN];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p", &filename, &filename_len) == FAILURE) {
		return;
	}

	if (VCWD_REALPATH(filename, resolved_path_buff)) {
		if (php_check_open_basedir(resolved_path_buff TSRMLS_CC)) {
			RETURN_FALSE;
		}

#ifdef ZTS
		if (VCWD_ACCESS(resolved_path_buff, F_OK)) {
			RETURN_FALSE;
		}
#endif
		RETURN_STRING(resolved_path_buff, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */
/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_ezc_fetch_global, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_set_global, 0)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_call, 0)
  ZEND_ARG_INFO(0, function_name)
  ZEND_ARG_INFO(0, parameter)
  ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_try_call, 0)
  ZEND_ARG_INFO(0, function_name)
  ZEND_ARG_INFO(0, parameter)
  ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_throw, 0)
  ZEND_ARG_INFO(0, class_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_throw_std, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_throw_nonstd, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_realpath, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ ezc_test_functions[]
 */
const zend_function_entry ezc_test_functions[] = {
  PHP_FE(ezc_fetch_global, arginfo_ezc_fetch_global)
  PHP_FE(ezc_set_global, arginfo_ezc_set_global)
  PHP_FE(ezc_call, arginfo_ezc_call)
  PHP_FE(ezc_try_call, arginfo_ezc_try_call)
  PHP_FE(ezc_throw, arginfo_ezc_throw)
	PHP_FE(ezc_throw_std, arginfo_ezc_throw_std)
	PHP_FE(ezc_throw_nonstd, arginfo_ezc_throw_nonstd)
	PHP_FE(ezc_realpath, arginfo_ezc_realpath)
  PHP_FE_END
};
/* }}} */

/* {{{ ezc_test_module_entry
 */
zend_module_entry ezc_test_module_entry = {
  STANDARD_MODULE_HEADER,
  "ezc_test",
  ezc_test_functions,
  PHP_MINIT(ezc_test),
  PHP_MSHUTDOWN(ezc_test),
  PHP_RINIT(ezc_test),
  PHP_RSHUTDOWN(ezc_test),
  PHP_MINFO(ezc_test),
  PHP_EZC_TEST_VERSION,
  PHP_MODULE_GLOBALS(ezc_test),
  PHP_GINIT(ezc_test),
  PHP_GSHUTDOWN(ezc_test),
  ezc_test_post_deactivate,
  STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_EZC_TEST
ZEND_GET_MODULE(ezc_test)
#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
