/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2013 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Marcus Boerger <helly@php.net>                              |
   |          Sterling Hughes <sterling@php.net>                          |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "zend.h"
#include "zend_API.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "zend_globals.h"
#include "php.h"
#include "spprintf.h"
#include "hphp/system/systemlib.h"
#include "hphp/runtime/ext_zend_compat/hhvm/zend-class-entry.h"
#include "hphp/runtime/ext_zend_compat/hhvm/ZendExceptionStore.h"

ZEND_API void (*zend_throw_exception_hook)(zval *ex TSRMLS_DC);

void zend_exception_set_previous(zval *exception, zval *add_previous TSRMLS_DC)
{
  zval *previous;

  if (exception == add_previous || !add_previous || !exception) {
    return;
  }
  zend_class_entry *default_exception_ce = zend_exception_get_default(TSRMLS_C);
  if (Z_TYPE_P(add_previous) != IS_OBJECT
      && !instanceof_function(
        Z_OBJCE_P(add_previous), default_exception_ce TSRMLS_CC)) {
    zend_error(E_ERROR, "Cannot set non exception as previous exception");
    return;
  }
  while (exception
      && exception != add_previous
      && Z_OBJ_HANDLE_P(exception) != Z_OBJ_HANDLE_P(add_previous))
  {
    previous = zend_read_property(default_exception_ce, exception,
        "previous", sizeof("previous")-1, 1 TSRMLS_CC);
    if (Z_TYPE_P(previous) == IS_NULL) {
      zend_update_property(default_exception_ce, exception,
          "previous", sizeof("previous")-1, add_previous TSRMLS_CC);
      Z_DELREF_P(add_previous);
      return;
    }
    exception = previous;
  }
}

ZEND_API void zend_clear_exception(TSRMLS_D) /* {{{ */
{
  if (EG(prev_exception)) {
    zval_ptr_dtor(&EG(prev_exception));
    EG(prev_exception) = NULL;
  }
  if (!EG(exception)) {
    return;
  }
  zval_ptr_dtor(&EG(exception));
  EG(exception) = NULL;
  // don't rethrow
  ZendExceptionStore::getInstance().clear();
}
/* }}} */

void zend_register_default_exception(TSRMLS_D) {
}

ZEND_API zend_class_entry *zend_exception_get_default(TSRMLS_D) {
  // TODO request-local cache to avoid hashtable lookup
  return zend_hphp_class_to_class_entry(HPHP::SystemLib::s_ExceptionClass);
}

ZEND_API void zend_throw_exception_object(zval *exception TSRMLS_DC) /* {{{ */
{
  zend_class_entry *exception_ce;
  zend_class_entry *default_exception_ce = zend_exception_get_default(TSRMLS_C);

  if (exception == NULL || Z_TYPE_P(exception) != IS_OBJECT) {
    zend_error(E_ERROR, "Need to supply an object when throwing an exception");
  }

  exception_ce = Z_OBJCE_P(exception);

  if (!exception_ce
      || !instanceof_function(exception_ce, default_exception_ce TSRMLS_CC))
  {
    zend_error(E_ERROR,
        "Exceptions must be valid objects derived from the Exception base class");
  }
  zend_throw_exception_internal(exception TSRMLS_CC);
}
/* }}} */

void zend_throw_exception_internal(zval *exception TSRMLS_DC) /* {{{ */
{
  if (exception != NULL) {
    zend_exception_set_previous(exception, EG(exception) TSRMLS_CC);
    EG(exception) = exception;
  }
  if (EG(exception) != NULL) {
    ZendExceptionStore::getInstance().set(HPHP::Object(Z_OBJVAL_P(EG(exception))));
  }
}
/* }}} */

ZEND_API zval * zend_throw_exception(zend_class_entry *exception_ce,
    char *message, long code TSRMLS_DC) /* {{{ */
{
  zval *ex;
  zend_class_entry *default_exception_ce = zend_exception_get_default(TSRMLS_C);

  MAKE_STD_ZVAL(ex);
  if (exception_ce) {
    if (!instanceof_function(exception_ce, default_exception_ce TSRMLS_CC)) {
      zend_error(E_NOTICE,
          "Exceptions must be derived from the Exception base class");
      exception_ce = default_exception_ce;
    }
  } else {
    exception_ce = default_exception_ce;
  }
  object_init_ex(ex, exception_ce);


  if (message) {
    zend_update_property_string(default_exception_ce, ex,
        "message", sizeof("message")-1, message TSRMLS_CC);
  }
  if (code) {
    zend_update_property_long(default_exception_ce, ex,
        "code", sizeof("code")-1, code TSRMLS_CC);
  }

  zend_throw_exception_internal(ex TSRMLS_CC);
  return ex;
}
/* }}} */

ZEND_API zval * zend_throw_exception_ex(zend_class_entry *exception_ce,
    long code TSRMLS_DC, char *format, ...) /* {{{ */
{
  va_list arg;
  char *message;
  zval *zexception;

  va_start(arg, format);
  vspprintf(&message, 0, format, arg);
  va_end(arg);
  zexception = zend_throw_exception(exception_ce, message, code TSRMLS_CC);
  efree(message);
  return zexception;
}
/* }}} */
