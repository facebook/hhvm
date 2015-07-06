
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h" // @nolint
#include "php_ezc_test.h"
#include "zend_exceptions.h"
#include <stdexcept>

static int le_ezc_test_hash;
#define le_ezc_test_hash_name "EZC test hashtable"

static void ezc_hash_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void ezc_hash_element_dtor(void * data);
static zend_object_value EzcTestCloneable_create_object(zend_class_entry *ce TSRMLS_DC);
static void EzcTestCloneable_free_storage(void *object TSRMLS_DC);
static void EzcTestCloneable_clone_obj(void *object, void **object_clone TSRMLS_DC);

static zend_object_value EzcTestUncloneable1_create_object(zend_class_entry *ce TSRMLS_DC);
static zend_object_value EzcTestUncloneable2_create_object(zend_class_entry *ce TSRMLS_DC);

zend_object_handlers EzcTestCloneable_handlers;
zend_object_handlers EzcTestUncloneable1_handlers;
zend_object_handlers EzcTestUncloneable2_handlers;

zend_class_entry * EzcTestCloneable_ce;
zend_class_entry * EzcTestUncloneable1_ce;
zend_class_entry * EzcTestUncloneable2_ce;

/* {{{ EzcTestCloneable methods
 */
const zend_function_entry ezc_empty_methods[] = {
  ZEND_FE_END
};
/* }}} */

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
  le_ezc_test_hash = zend_register_list_destructors_ex(
      ezc_hash_dtor, NULL, le_ezc_test_hash_name, module_number);

  zend_class_entry ce;
  INIT_CLASS_ENTRY(ce, "EzcTestCloneable", ezc_empty_methods);
  EzcTestCloneable_ce = zend_register_internal_class(&ce TSRMLS_CC);
  EzcTestCloneable_ce->create_object = EzcTestCloneable_create_object;

  memcpy(&EzcTestCloneable_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  EzcTestCloneable_handlers.clone_obj = zend_objects_store_clone_obj;

  INIT_CLASS_ENTRY(ce, "EzcTestUncloneable1", ezc_empty_methods);
  EzcTestUncloneable1_ce = zend_register_internal_class(&ce TSRMLS_CC);
  EzcTestUncloneable1_ce->create_object = EzcTestUncloneable1_create_object;

  memcpy(&EzcTestUncloneable1_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  EzcTestUncloneable1_handlers.clone_obj = NULL;

  INIT_CLASS_ENTRY(ce, "EzcTestUncloneable2", ezc_empty_methods);
  EzcTestUncloneable2_ce = zend_register_internal_class(&ce TSRMLS_CC);
  EzcTestUncloneable2_ce->create_object = EzcTestUncloneable2_create_object;

  memcpy(&EzcTestUncloneable2_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  EzcTestUncloneable2_handlers.clone_obj = zend_objects_store_clone_obj;

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

/* {{{ proto mixed ezc_call(mixed function_name [, mixed parameter] [, mixed ...])
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

/* {{{ proto mixed ezc_try_call(mixed function_name [, mixed parameter] [, mixed ...])
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

/* {{{ proto void ezc_throw_cpp_std()
 * Throw a C++ std::exception */
PHP_FUNCTION(ezc_throw_std)
{
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }

  throw std::runtime_error("ezc_throw_std");
}
/* }}} */

/* {{{ proto void ezc_throw_cpp_nonstd()
 * Throw a non-standard C++ exception */
PHP_FUNCTION(ezc_throw_nonstd)
{
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    RETURN_FALSE;
  }

  throw "ezc_throw_nonstd";
}
/* }}} */

/* {{{ proto string ezc_realpath(string path)
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

/* {{{ proto mixed ezc_min(mixed arg1 [, mixed arg2 [, mixed ...]])
   Varadic argument test, equivalent to min() */
PHP_FUNCTION(ezc_min)
{
  int argc;
  zval ***args = nullptr;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+",
                            &args, &argc) == FAILURE) {
    return;
  }

  zval **min;
  zval * result;
  ALLOC_ZVAL(result);
  int i;

  min = args[0];

  for (i = 1; i < argc; i++) {
    is_smaller_function(result, *args[i], *min TSRMLS_CC);
    if (Z_LVAL_P(result) == 1) {
      min = args[i];
    }
  }
  FREE_ZVAL(result);

  RETVAL_ZVAL(*min, 1, 0);

  if (args) {
    efree(args);
  }
}

/* }}} */

/** {{{ proto resource ezc_hash_create()
 * Create a hash which maps string keys to string values. When a value is
 * deleted from the hash, it is written to the current output buffer.
 */
PHP_FUNCTION(ezc_hash_create)
{
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }
  HashTable * hash;
  ALLOC_HASHTABLE(hash);
  zend_hash_init(hash, 0, NULL, ezc_hash_element_dtor, 0);
  ZEND_REGISTER_RESOURCE(return_value, hash, le_ezc_test_hash);
}
/* }}} */

/** {{{ proto void ezc_hash_set(resource table, string key, string value)
 * Set a hash item
 */
PHP_FUNCTION(ezc_hash_set)
{
  zval * zht;
  HashTable * hash;
  char * key;
  int key_len;
  char * value;
  int value_len;
  char * initial_value;
  char * dest;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
        "rss", &zht, &key, &key_len, &value, &value_len) == FAILURE) {
    return;
  }
  ZEND_FETCH_RESOURCE(hash, HashTable*, &zht, -1, le_ezc_test_hash_name, le_ezc_test_hash);

  initial_value = (char*)ecalloc(1, value_len + 1);
  zend_symtable_update(hash, key, key_len + 1, initial_value, value_len + 1, (void**)&dest);
  memcpy(dest, value, value_len);
  dest[value_len] = '\0';
  efree(initial_value);
}
/* }}} */

/** {{{ proto mixed ezc_hash_get(resource table, string key)
 * Get a hash item
 */
PHP_FUNCTION(ezc_hash_get)
{
  zval * zht;
  HashTable * hash;
  char * key;
  int key_len;
  char * value;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
        "rs", &zht, &key, &key_len) == FAILURE) {
    return;
  }
  ZEND_FETCH_RESOURCE(hash, HashTable*, &zht, -1, le_ezc_test_hash_name, le_ezc_test_hash);

  if (zend_symtable_find(hash, key, key_len + 1, (void**)&value) == SUCCESS) {
    RETURN_STRING(value, 1);
  } else {
    RETURN_FALSE;
  }
}
/* }}} */

/** {{{ proto ezc_hash_append(resource table, string value)
 * Append a value to the hash, with the next-highest available numeric
 * key.
 */
PHP_FUNCTION(ezc_hash_append)
{
  zval * zht;
  HashTable * hash;
  char * value;
  int value_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
        "rs", &zht, &value, &value_len) == FAILURE) {
    return;
  }
  ZEND_FETCH_RESOURCE(hash, HashTable*, &zht, -1, le_ezc_test_hash_name, le_ezc_test_hash);
  zend_hash_next_index_insert(hash, value, value_len + 1, NULL);
}
/* }}} */

static void ezc_hash_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
  HashTable * hash = (HashTable*)rsrc->ptr;
  zend_hash_destroy(hash);
  FREE_HASHTABLE(ht);
}
/* }}} */

static void ezc_hash_element_dtor(void * data) /* {{{ */
{
  TSRMLS_FETCH();
  char * value = (char*)data;
  php_write(value, strlen(value) TSRMLS_CC);
}
/* }}} */

/* {{{ proto void ezc_array_val_set(array a, mixed key, mixed value)
 * Set an element of an array passed by value, by modifying the data pointer
 * returned by zend_hash_find(). This should theoretically have no effect
 * on the caller.
 */
PHP_FUNCTION(ezc_array_val_set)
{
  zval *array, *key, *value;
  zval ** data;
  int found;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
        "azz", &array, &key, &value) == FAILURE)
  {
    return;
  }

  if (Z_TYPE_P(key) == IS_LONG) {
    found = zend_hash_index_find(
        Z_ARRVAL_P(array), Z_LVAL_P(key), (void**)&data);
  } else if (Z_TYPE_P(key) == IS_STRING) {
    found = zend_hash_find(
        Z_ARRVAL_P(array), Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, (void**)&data);
  } else {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key must be either integer or string");
    return;
  }
  if (found == FAILURE) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key must exist");
    return;
  }
  zval_ptr_dtor(data);
  Z_ADDREF_P(value);
  *data = value;
}
/* }}} */

/* {{{ proto void ezc_array_set(array a, mixed key, mixed value)
 * Set an element of an array passed by reference, by modifying the data
 * pointer returned by zend_hash_find().
 */
PHP_FUNCTION(ezc_array_set)
{
  zval *array, *key, *value;
  zval **data;
  int found;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
        "azz", &array, &key, &value) == FAILURE)
  {
    return;
  }

  if (Z_TYPE_P(key) == IS_LONG) {
    found = zend_hash_index_find(
        Z_ARRVAL_P(array), Z_LVAL_P(key), (void**)&data);
  } else if (Z_TYPE_P(key) == IS_STRING) {
    found = zend_hash_find(
        Z_ARRVAL_P(array), Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, (void**)&data);
  } else {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key must be either integer or string");
    return;
  }
  if (found == FAILURE) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "array key must exist");
    return;
  }
  zval_ptr_dtor(data);
  Z_ADDREF_P(value);
  *data = value;
}

/* }}} */

/* {{{ proto mixed ezc_get_error_reporting()
 * Use the zend construct EG(error_reporting) in rval context.
 */
PHP_FUNCTION(ezc_get_error_reporting)
{
  int old_error_reporting = EG(error_reporting);

  RETVAL_LONG(old_error_reporting);
}
/* }}} */

/* {{{ proto mixed ezc_set_error_reporting(integer $val)
 * Use the zend construct EG(error_reporting) in rval and then lval context.
 */
PHP_FUNCTION(ezc_set_error_reporting)
{
  long old_error_reporting = EG(error_reporting);
  long new_error_reporting = 0;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &new_error_reporting) == FAILURE) {
    RETURN_FALSE;
  }
  EG(error_reporting) = new_error_reporting;
  RETVAL_LONG(old_error_reporting);
}
/* }}} */

/* {{{ EzcTestCloneable_create_object */
static zend_object_value EzcTestCloneable_create_object(zend_class_entry *ce TSRMLS_DC)
{
  php_ezctest_obj * obj = (php_ezctest_obj*)emalloc(sizeof(php_ezctest_obj));
  zend_object_value retval;
  memset(obj, 0, sizeof(php_ezctest_obj));
  zend_object_std_init(&obj->std, ce TSRMLS_CC);
  object_properties_init(&obj->std, ce);
  obj->clone_count = 0;
  retval.handle = zend_objects_store_put(
      obj,
      (zend_objects_store_dtor_t) NULL,
      EzcTestCloneable_free_storage,
      EzcTestCloneable_clone_obj
      TSRMLS_CC);
  retval.handlers = &EzcTestCloneable_handlers;
  return retval;
}
/* }}} */

/* {{{ EzcTestCloneable_free_storage */
static void EzcTestCloneable_free_storage(void *object TSRMLS_DC)
{
  php_ezctest_obj * eobj = (php_ezctest_obj*)object;
  php_printf("EzcTestCloneable free_storage (clone %d)\n", eobj->clone_count);
  zend_object_std_dtor(&eobj->std TSRMLS_CC);
  efree(object);
}
/* }}} */

/* {{{ EzcTestCloneable_clone_obj */
static void EzcTestCloneable_clone_obj(void *object, void **object_clone TSRMLS_DC)
{
  php_ezctest_obj * srcobj = (php_ezctest_obj*)object;
  php_ezctest_obj * clone = (php_ezctest_obj*)emalloc(sizeof(php_ezctest_obj));
  memset(clone, 0, sizeof(php_ezctest_obj));
  zend_object_std_init(&clone->std, srcobj->std.ce TSRMLS_CC);
  object_properties_init(&clone->std, srcobj->std.ce);
  clone->clone_count = srcobj->clone_count + 1;
  *object_clone = (void*)clone;
}
/* }}} */

/* {{{ EzcTestUncloneable1_create_object */
static zend_object_value EzcTestUncloneable1_create_object(zend_class_entry *ce TSRMLS_DC)
{
  zend_object * obj;
  zend_object_value retval = zend_objects_new(&obj, ce TSRMLS_CC);
  retval.handlers = &EzcTestUncloneable1_handlers;
  return retval;
}
/* }}} */

/* {{{ EzcTestUncloneable2_create_object */
static zend_object_value EzcTestUncloneable2_create_object(zend_class_entry *ce TSRMLS_DC)
{
  zend_object * obj = (zend_object*)emalloc(sizeof(zend_object));
  zend_object_value retval;
  memset(obj, 0, sizeof(zend_object));
  zend_object_std_init(obj, ce TSRMLS_CC);
  object_properties_init(obj, ce);
  retval.handle = zend_objects_store_put(
      obj,
      (zend_objects_store_dtor_t) zend_objects_destroy_object,
      (zend_objects_free_object_storage_t) zend_objects_free_object_storage,
      NULL // clone
      TSRMLS_CC);
  retval.handlers = &EzcTestUncloneable2_handlers;
  return retval;
}
/* }}} */

/* {{{ proto object ezc_create_cloneable_in_array()
 * Create an array with a TestCloneable object in it. Test of
 * object_init_ex(). */
PHP_FUNCTION(ezc_create_cloneable_in_array)
{
  zval * element;
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }
  array_init_size(return_value, 1);
  ALLOC_INIT_ZVAL(element);
  object_init_ex(element, EzcTestCloneable_ce);
  zend_hash_next_index_insert(Z_ARRVAL_P(return_value),
      (void*)&element, sizeof(zval*), NULL);
}
/* }}} */

/* {{{ proto mixed ezc_atof_toa()
 * Convert a string to a double and back to a string again */
PHP_FUNCTION(ezc_atof_toa)
{
  char *value;
  int valuelen;
  double d;
  const char *end_value;
  const int ndigits = 10;
  int decpt = -1;
  int sign = -1;
  char *result;
  char *resulte;
  char *composed_result;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
        &value, &valuelen) == FAILURE) {
    RETURN_FALSE;
  }

  d = zend_strtod(value, &end_value);
  (void)end_value;

  result = zend_dtoa(d, 0, ndigits, &decpt, &sign, &resulte);
  asprintf(&composed_result, "%d %d %s", decpt, sign, result);
  zend_freedtoa(result);
  RETVAL_STRING(composed_result, 1);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ezc_min, 0, 0, 1)
  ZEND_ARG_INFO(0, arg1)
  ZEND_ARG_INFO(0, arg2)
  ZEND_ARG_INFO(0, ...)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_hash_create, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_hash_set, 0)
  ZEND_ARG_INFO(0, table)
  ZEND_ARG_INFO(0, key)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_hash_get, 0)
  ZEND_ARG_INFO(0, table)
  ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_hash_append, 0)
  ZEND_ARG_INFO(0, table)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_array_val_set, 0)
  ZEND_ARG_INFO(0, a)
  ZEND_ARG_INFO(0, key)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_create_cloneable_in_array, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_array_set, 0)
  ZEND_ARG_INFO(1, a)
  ZEND_ARG_INFO(0, key)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_get_error_reporting, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_set_error_reporting, 0)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ezc_atof_toa, 0)
  ZEND_ARG_INFO(0, value)
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
  PHP_FE(ezc_min, arginfo_ezc_min)
  PHP_FE(ezc_hash_create, arginfo_ezc_hash_create)
  PHP_FE(ezc_hash_set, arginfo_ezc_hash_set)
  PHP_FE(ezc_hash_get, arginfo_ezc_hash_get)
  PHP_FE(ezc_hash_append, arginfo_ezc_hash_append)
  PHP_FE(ezc_array_val_set, arginfo_ezc_array_val_set)
  PHP_FE(ezc_create_cloneable_in_array, arginfo_ezc_create_cloneable_in_array)
  PHP_FE(ezc_array_set, arginfo_ezc_array_set)
  PHP_FE(ezc_get_error_reporting, arginfo_ezc_get_error_reporting)
  PHP_FE(ezc_set_error_reporting, arginfo_ezc_set_error_reporting)
  PHP_FE(ezc_atof_toa, arginfo_ezc_atof_toa)
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
